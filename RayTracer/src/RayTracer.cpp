/*
 * RayTracer.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>
#include <math.h>
#include <random>

#include "RayTracer.hpp"
#include "exceptions.hpp"

#include "Utility\mathUtility.hpp"
#include "Utility\materialUtility.hpp"
#include "Textures\ImageTexture.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	using namespace utility;

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/


	void RayTracer::initialize(const std::string sceneDirPath)
	{
		this->renderWidth = this->application.getRenderWidth();
		this->renderHeight = this->application.getRenderHeight();

		if (!this->scene->HasCameras())
		{
			throw Renderer("No camera found!");
		}
		// Using first camera in scene
		aiCamera* camera = (*this->scene->mCameras);

		float aspectRatio = camera->mAspect;
		float fieldOfView = camera->mHorizontalFOV;
		if (aspectRatio != 1.0f)
		{
			SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Aspect ratio of camera is not 1! Proceeding..");
		}

		const aiVector3D cameraUp = camera->mUp.Normalize();
		const aiVector3D lookAt = camera->mLookAt.Normalize();
		const aiVector3D cameraRight = camera->mRight.Normalize();

		float halfViewportWidth = std::tan(fieldOfView / 2.0f);
		float halfViewportHeight = halfViewportWidth * aspectRatio;

		this->pixelShiftX = ((2 * halfViewportWidth) / (renderWidth)) * cameraRight;
		this->pixelShiftY = ((2 * halfViewportHeight) / (renderHeight)) * cameraUp;
		this->topLeftPixel = lookAt - (halfViewportWidth * cameraRight) + (halfViewportHeight * cameraUp);

		this->createJobs();

		std::srand(static_cast<unsigned int>(time(0)));

		this->pixels = new Uint24[renderWidth * renderHeight];

		materialUtility::createMaterialMapping(sceneDirPath, scene, &this->materialMapping);
	}


	void RayTracer::renderMultiThreaded()
	{
		RenderJob job;
		while (this->renderJobs.popFront(job))
		{
			if (this->renderSettings.getUseAA())
			{
				this->renderAntiAliased(job);
			}
			else
			{
				this->render(job);
			}

			std::this_thread::yield();
		}
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

	void RayTracer::render()
	{
		for (uint16_t x = 0; x < this->renderWidth; x++)
		{
			for (uint16_t y = 0; y < this->renderHeight; y++)
			{
				uint32_t currentPixel = y * renderWidth + x;
				aiVector3D rayDirection = (this->topLeftPixel + (this->pixelShiftX * static_cast<float>(x)) + (this->pixelShiftY * static_cast<float>(y))).Normalize();
				aiRay currentRay((*this->scene->mCameras)->mPosition, rayDirection);
#if PATH_TRACE
				this->pixels[currentPixel] = this->tracePath(currentRay);
#else
				this->pixels[currentPixel] = this->traceRay(currentRay);
#endif
			}
		}
	}


	void RayTracer::render(RenderJob& renderJob)
	{
		const uint8_t maxSamples = this->renderSettings.getMaxSamples();
		aiVector3D& cameraPosition = (*this->scene->mCameras)->mPosition;

		for (uint16_t x = renderJob.getTileStartX(); x < renderJob.getTileEndX(); x++)
		{
			for (uint16_t y = renderJob.getTileStartY(); y < renderJob.getTileEndY(); y++)
			{
				aiColor3D pixelAverage{};
				uint32_t currentPixel = y * renderWidth + x;
				aiVector3D nextPixelX = this->pixelShiftX * static_cast<float>(x);
				aiVector3D nextPixelY = this->pixelShiftY * static_cast<float>(y);
				aiVector3D rayDirection = (this->topLeftPixel + nextPixelX - nextPixelY).Normalize();
				aiRay currentRay(cameraPosition, rayDirection);

				for (unsigned int i = 0; i < std::pow(maxSamples, 2U); i++)
				{
					// DOF
					if (this->renderSettings.getUseDOF())
					{
						mathUtility::calculateDepthOfFieldRay(
							&currentRay,
							this->renderSettings.getAperture(),
							this->renderSettings.getFocalDistance());
					}

#if PATH_TRACE
					pixelAverage += this->tracePath(currentRay);
#else
					pixelAverage += this->traceRay(currentRay);
#endif
				}
				pixelAverage = pixelAverage * static_cast<float>(1.f / std::pow(maxSamples, 2U));

				// sRGB 
				mathUtility::gammaCorrectSrgb(&pixelAverage);

				// Adobe RGB
				//mathUtility::gammaCorrectAdobeRgb(&pixelAverage);

				this->pixels[currentPixel] = pixelAverage;
			}
		}
	}

	void RayTracer::renderAntiAliased(RenderJob& renderJob)
	{
		const uint8_t aa = this->renderSettings.getMaxSamples();
		aiVector3D& cameraPosition = (*this->scene->mCameras)->mPosition;

		for (unsigned int x = renderJob.getTileStartX(); x < renderJob.getTileEndX(); x++)
		{
			for (unsigned int y = renderJob.getTileStartY(); y < renderJob.getTileEndY(); y++)
			{
				uint32_t currentPixel = y * renderWidth + x;
				aiColor3D pixelAverage{};

				for (unsigned int p = 0; p < aa; p++)
				{
					for (unsigned int q = 0; q < aa; q++)
					{
						// Anti aliasing
						float r = mathUtility::getRandomFloat(0.f, 1.f);
						float aaShiftX = x + (p + r) / aa;
						float aaShiftY = y + (q + r) / aa;

						aiVector3D nextPixelX = this->pixelShiftX * static_cast<float>(aaShiftX);
						aiVector3D nextPixelY = this->pixelShiftY * static_cast<float>(aaShiftY);
						aiVector3D rayDirection = (this->topLeftPixel + nextPixelX - nextPixelY).Normalize();
						aiRay currentRay(cameraPosition, rayDirection);

						// DOF
						if (this->renderSettings.getUseDOF())
						{
							mathUtility::calculateDepthOfFieldRay(
								&currentRay, 
								this->renderSettings.getAperture(), 
								this->renderSettings.getFocalDistance());
						}

#if PATH_TRACE
						pixelAverage += this->tracePath(currentRay);
#else
						pixelAverage += this->traceRay(currentRay);
#endif
					}
				}
				pixelAverage = pixelAverage * static_cast<float>(1.f/std::pow(aa, 2U));
				
				// sRGB 
				mathUtility::gammaCorrectSrgb(&pixelAverage);

				// Adobe RGB
				//mathUtility::gammaCorrectAdobeRgb(&pixelAverage);

				this->pixels[currentPixel] = pixelAverage;
			}
		}
	}

	aiColor3D RayTracer::sampleLight(IntersectionInformation& intersectionInformation, uint8_t rayDepth)
	{
		unsigned int materialIndex = intersectionInformation.hitMesh->mMaterialIndex;
		aiMaterial* meshMaterial = this->scene->mMaterials[materialIndex];
		Material* material = this->materialMapping[meshMaterial].get();

		aiVector3D smoothNormal = mathUtility::calculateSmoothNormal(intersectionInformation.uv, intersectionInformation.vertexNormals);

		// Return immediately if hit object is emissive
		const aiColor3D& mEmissive = material->getEmissive();
		if (aiColor3D{0.f, 0.f, 0.f} < mEmissive)
		{
			return mEmissive;
		}

		// Russian roulette
		const aiColor3D& mDiffuse = material->getDiffuse(intersectionInformation.uvTextureCoords);
		aiColor3D brdf = mDiffuse / PI;
		const float probability = 
			(mDiffuse.r > mDiffuse.g) && (mDiffuse.r>mDiffuse.b) ?
			 mDiffuse.r : mDiffuse.g > mDiffuse.b ?
			 mDiffuse.g : mDiffuse.b;
		if ((rayDepth + 1) > 5)
		{
			if (mathUtility::russianRoulette(probability, rayDepth))
			{
				// Russian roulette takes effect!
				return mEmissive;
			}
			else
			{
				brdf = brdf * (0.9f / probability);
			}
		}

		// Compute indirect light
		aiVector3D Nt{}, Nb{}, newRayDirection{}, newRayPosition{};
		aiRay sampleRay{};
		const float r1 = mathUtility::getRandomFloat(0.f, 1.f);
		const float r2 = mathUtility::getRandomFloat(0.f, 1.f);
		
		// Calculate transformation matrix to transform sample from world space to shaded point local coordinate system later
		mathUtility::createCoordinateSystem(smoothNormal, Nt, Nb);
		aiMatrix3x3 toLocalMatrix
		{ Nt.x, Nt.y, Nt.z,
			smoothNormal.x, smoothNormal.y, smoothNormal.z,
			Nb.x, Nb.y, Nb.z };

		if (material->getOpacity() < 1.f)
		{
			// Perfect translucent refraction
			const float ior = material->getRefractionIndex();
			float fresnelResult = mathUtility::fresnel(intersectionInformation.ray.dir, smoothNormal, ior);

			if (fresnelResult >= 1.f)
			{
				// Total internal reflection
				return aiColor3D{ 0.f, 0.f, 0.f };
			}

			bool outside = (intersectionInformation.ray.dir * smoothNormal) < 0;
			aiVector3D bias = this->renderSettings.getBias() * smoothNormal;
			const float refractionPropability = mathUtility::getRandomFloat(0.f, 1.f);

			if (refractionPropability > fresnelResult)
			{
				// New ray is a refraction ray
				newRayDirection = mathUtility::calculateRefractionDirection(intersectionInformation.ray.dir, smoothNormal, ior);
				newRayDirection.Normalize();
				newRayPosition = outside ? intersectionInformation.hitPoint - bias : intersectionInformation.hitPoint + bias;
				sampleRay = { newRayPosition, newRayDirection, RayType::REFRACTION };
			}
			else
			{
				// New ray is a reflection ray
				newRayDirection = mathUtility::calculateReflectionDirection(intersectionInformation.ray.dir, smoothNormal);
				newRayDirection.Normalize();
				newRayPosition = outside ? intersectionInformation.ray.pos + bias : intersectionInformation.ray.pos - bias;
				sampleRay = { newRayPosition, newRayDirection, RayType::REFLECTION };
				brdf = material->getReflective() / PI;
			}
		}
		else if (material->getReflectivity() > 0.f)
		{
			// Specular reflection
			const float roughness = 0.f; // 1 = blurry   0 = perfectly specular
			aiVector3D randomScatter = mathUtility::cosineSampleHemisphere(r1, r2);
			randomScatter *= toLocalMatrix;

			// Calculate reflection direction
			newRayDirection = mathUtility::calculateReflectionDirection(intersectionInformation.ray.dir, smoothNormal);
			newRayDirection = newRayDirection + randomScatter * roughness;

			brdf = material->getReflective() / PI;
			newRayPosition = intersectionInformation.hitPoint + (newRayDirection * this->renderSettings.getBias());
			sampleRay = { newRayPosition, newRayDirection, RayType::REFLECTION };
		}
		else
		{
			// Perfect diffuse reflection
			newRayDirection = mathUtility::cosineSampleHemisphere(r1, r2);
			newRayDirection *= toLocalMatrix;

			newRayPosition = intersectionInformation.hitPoint + (newRayDirection * this->renderSettings.getBias());
			sampleRay = { newRayPosition, newRayDirection, RayType::INDIRECT_DIFFUSE };
		}

		// Cast a ray in calculated direction
		aiColor3D incomingLight = tracePath(sampleRay, rayDepth + 1);

		// Simplified rendering equation for cosine weighted sampling
		return brdf * incomingLight * PI;
	}


	aiColor3D RayTracer::shadePixel(IntersectionInformation& intersectionInformation, uint8_t& rayDepth)
	{
		// Get mesh properties
		aiVector3D edge1 = *(intersectionInformation.hitTriangle[1]) - *(intersectionInformation.hitTriangle[0]);
		aiVector3D edge2 = *(intersectionInformation.hitTriangle[2]) - *(intersectionInformation.hitTriangle[0]);
		aiVector3D faceNormal = (edge1 ^ edge2).Normalize();
		unsigned int materialIndex = intersectionInformation.hitMesh->mMaterialIndex;
		aiMaterial* material = this->scene->mMaterials[materialIndex];
		aiColor3D materialColorDiffuse{}, materialColorEmission{}, materialColorAmbient{}, colorReflective{};
		int shadingModel{};
		float reflectivity{}, opacity{ 1 }, refractionIndex{ 1 };
		material->Get(AI_MATKEY_COLOR_DIFFUSE, materialColorDiffuse);
		material->Get(AI_MATKEY_COLOR_EMISSIVE, materialColorEmission);
		material->Get(AI_MATKEY_COLOR_AMBIENT, materialColorAmbient);
		material->Get(AI_MATKEY_SHADING_MODEL, shadingModel);
		material->Get(AI_MATKEY_REFLECTIVITY, reflectivity);
		material->Get(AI_MATKEY_COLOR_REFLECTIVE, colorReflective);
		material->Get(AI_MATKEY_OPACITY, opacity);
		material->Get(AI_MATKEY_REFRACTI, refractionIndex);
		aiColor3D intersectionColor{ 0.f, 0.f, 0.f };

		// Calculate vertex normal for smooth shading
		aiVector3D smoothNormal = (1 - intersectionInformation.uv.x - intersectionInformation.uv.y) *
			*intersectionInformation.vertexNormals[0] + intersectionInformation.uv.x *
			*intersectionInformation.vertexNormals[1] + intersectionInformation.uv.y *
			*intersectionInformation.vertexNormals[2];
		smoothNormal.Normalize();

		if (shadingModel == aiShadingMode::aiShadingMode_NoShading)
		{
			aiColor3D ambientLightStrength{};
			for (unsigned int currentLight = 0; currentLight < scene->mNumLights; currentLight++)
			{
				aiLight* light = this->scene->mLights[currentLight];
				if (light->mType == aiLightSourceType::aiLightSource_AMBIENT)
				{
					ambientLightStrength += light->mColorAmbient;
				}
			}
			// TODO: Verify ambient light contributes to shadeless model
			intersectionColor = materialColorEmission + materialColorAmbient * ambientLightStrength;
		}
		else if (shadingModel == aiShadingMode::aiShadingMode_Phong || shadingModel == aiShadingMode::aiShadingMode_Gouraud)
		{

			if (opacity < 1.f)
			{
				// Surface transmits light. Cast refraction ray and calculate color
				const float EPSILON = 1e-3f;
				aiColor3D refractionColor{}, reflectionColor{};
				float fresnelResult = mathUtility::fresnel(intersectionInformation.ray.dir, smoothNormal, refractionIndex);
				bool outside = (intersectionInformation.ray.dir * smoothNormal) < 0;
				aiVector3D bias = this->renderSettings.getBias() * smoothNormal;

				if (fresnelResult < 1.f)
				{
					// Ignore total internal reflection
					aiVector3D refractionDirection = mathUtility::calculateRefractionDirection(intersectionInformation.ray.dir, smoothNormal, refractionIndex);
					refractionDirection.Normalize();
					aiVector3D refractionPoint = outside ? intersectionInformation.hitPoint - bias : intersectionInformation.hitPoint + bias;
					aiRay refractionRay(refractionPoint, refractionDirection);
					refractionColor = traceRay(refractionRay, rayDepth + 1) * (1 - fresnelResult);
				}

				if ((fresnelResult > EPSILON) && (reflectivity > 0.f))
				{
					// Surface also is reflective. Cast reflection ray and blend color with fresnel
					aiVector3D reflectionDirection = mathUtility::calculateReflectionDirection(intersectionInformation.ray.dir, smoothNormal);
					reflectionDirection.Normalize();
					aiVector3D reflectionPoint = outside ? intersectionInformation.ray.pos + bias : intersectionInformation.ray.pos - bias;
					aiRay reflectionRay(reflectionPoint, reflectionDirection);
					reflectionColor = traceRay(reflectionRay, rayDepth + 1) * fresnelResult * reflectivity;
				}

				intersectionColor += reflectionColor + refractionColor;
				//return intersectionColor; // TODO: Implement color shading for refractive materials
			}
			else if (reflectivity > 0.f)
			{
				// Surface only is reflective. Cast reflection ray and calculate color at reflection
				aiVector3D reflectionDirection = mathUtility::calculateReflectionDirection(intersectionInformation.ray.dir, smoothNormal);
				aiVector3D reflectionPoint = intersectionInformation.hitPoint + (smoothNormal * this->renderSettings.getBias());
				aiRay reflectionRay(reflectionPoint, reflectionDirection);
				intersectionColor += traceRay(reflectionRay, rayDepth + 1) * reflectivity;
			}

			// Calculate surface color
			for (unsigned int currentLight = 0; currentLight < scene->mNumLights; currentLight++)
			{
				aiLight* light = this->scene->mLights[currentLight];
				aiVector3D lightDirection;
				aiColor3D lightIntensity;

				if (light->mType == aiLightSourceType::aiLightSource_POINT)
				{
					lightDirection = (light->mPosition - intersectionInformation.hitPoint);
					float distance = lightDirection.Length();
					float squareDistance = lightDirection.SquareLength();
					lightDirection.Normalize();
					float attConst = light->mAttenuationConstant;
					float attLinear = light->mAttenuationLinear;
					float attQuad = light->mAttenuationQuadratic;
					float attenuation = 1 / (attConst + attLinear * distance + attQuad * squareDistance);
					lightIntensity = light->mColorDiffuse * attenuation;
				}
				else if (light->mType == aiLightSourceType::aiLightSource_DIRECTIONAL)
				{
					lightDirection = -light->mDirection;
					lightDirection.Normalize();
					lightIntensity = light->mColorDiffuse;
				}
				else
				{
					// Skip calculation for undefined or unsupported light types
					continue;
				}

				aiRay shadowRay(intersectionInformation.hitPoint + (smoothNormal * this->renderSettings.getBias()), lightDirection, RayType::SHADOW);
				IntersectionInformation shadowRayIntersectionInfo;
#if USE_ACCELERATION_STRUCTURE
				bool pointInShadow = !this->accelerationStructure->calculateIntersection(shadowRay, shadowRayIntersectionInfo);
#else
				bool pointInShadow = !calculateIntersection(shadowRay, shadowRayIntersectionInfo);
#endif
				intersectionColor += materialColorDiffuse * pointInShadow * lightIntensity * std::max(0.f, smoothNormal * lightDirection) * (1 - reflectivity) * opacity;
			}
		}
		else if (shadingModel == aiShadingMode::aiShadingMode_Phong)
		{
			// TODO: Implement
		}
		else if (shadingModel == aiShadingMode::aiShadingMode_Gouraud) // Lambert in collada
		{
			// TODO: Implement
		}

		return intersectionColor;
	}


	bool RayTracer::calculateIntersection(
		aiRay& ray,
		IntersectionInformation& outIntersection)
	{
		bool intersects{ false };
		std::vector<aiVector3D*> nearestIntersectedTriangle;
		std::vector<aiVector3D*> triangleVertexNormals;
		std::vector<aiVector3D*> textureCoordinates;
		aiVector3D intersectionPoint;
		aiVector2D uvCoordinates;
		// Iterate through meshes and faces and check for  ray-triangle intersections
		for (unsigned int currentMesh = 0; currentMesh < this->scene->mNumMeshes; currentMesh++)
		{
			aiString name = this->scene->mMeshes[currentMesh]->mName;
			aiMesh* mesh = this->scene->mMeshes[currentMesh];

			for (unsigned int currentFace = 0; currentFace < mesh->mNumFaces; currentFace++)
			{
				aiFace* face = &(mesh->mFaces[currentFace]);
				for (unsigned int currentIndex = 0; currentIndex < face->mNumIndices; currentIndex++)
				{
					nearestIntersectedTriangle.push_back(&(mesh->mVertices[face->mIndices[currentIndex]]));
					triangleVertexNormals.push_back(&(mesh->mNormals[face->mIndices[currentIndex]]));
					// Always use first texture channel
					textureCoordinates.push_back(&(mesh->mTextureCoords[0][face->mIndices[currentIndex]]));
				}
				// TODO: Collect all intersections and push them back into a collection
				// Evaluate nearest intersection point and return
				bool intersectsCurrentTriangle = mathUtility::rayTriangleIntersection(ray, nearestIntersectedTriangle, &intersectionPoint, &uvCoordinates);
				// We can immediately return if the cast ray is a shadow ray
				if (ray.type == RayType::SHADOW)
				{
					return true;
				}
				float distanceToIntersectionPoint = (intersectionPoint - ray.pos).Length();
				if (intersectsCurrentTriangle && (distanceToIntersectionPoint < outIntersection.intersectionDistance))
				{
					outIntersection.intersectionDistance = distanceToIntersectionPoint;
					outIntersection.hitMesh = mesh;
					outIntersection.hitTriangle = nearestIntersectedTriangle;
					outIntersection.hitPoint = intersectionPoint;
					outIntersection.ray = ray;
					outIntersection.uv = uvCoordinates;
					// Always use first texture channel
					if (mesh->HasTextureCoords(0))
					{
						outIntersection.uvTextureCoords =
							(1 - uvCoordinates.x - uvCoordinates.y) *
							*(textureCoordinates[0]) + uvCoordinates.x *
							*(textureCoordinates[1]) + uvCoordinates.y *
							*(textureCoordinates[2]);
					}
					outIntersection.vertexNormals = triangleVertexNormals;
					outIntersection.textureCoordinates = textureCoordinates;
					intersects = true;
				}
				nearestIntersectedTriangle.clear();
				triangleVertexNormals.clear();
				textureCoordinates.clear();
			}
		}
		return intersects;
	}


	aiColor3D RayTracer::tracePath(aiRay& ray, uint8_t rayDepth /*= 0*/)
	{
		IntersectionInformation intersectionInformation;
		if (rayDepth > this->renderSettings.getMaxRayDepth())
		{
			// TODO: Get scene background color
			return { .1f, .1f, .1f };
		}
#if USE_ACCELERATION_STRUCTURE
		bool intersects = this->accelerationStructure->calculateIntersection(ray, intersectionInformation);
#else
		bool intersects = this->calculateIntersection(ray, intersectionInformation);
#endif
		if (intersects)
		{
			// Calculate color at the intersection
			return this->sampleLight(intersectionInformation, rayDepth);
		}
		else
		{
			// TODO: Get scene background color
			return { .1f, .1f, .1f };
		}
	}

	aiColor3D RayTracer::traceRay(aiRay& ray, uint8_t rayDepth /*= 0*/)
	{
		IntersectionInformation intersectionInformation;
		if (rayDepth > this->renderSettings.getMaxRayDepth())
		{
			// TODO: Get scene background color
			return { .1f, .1f, .1f };
		}
#if USE_ACCELERATION_STRUCTURE
		bool intersects = this->accelerationStructure->calculateIntersection(ray, intersectionInformation);
#else
		bool intersects = this->calculateIntersection(ray, intersectionInformation);
#endif
		if (intersects)
		{
			// Calculate color at the intersection
			return this->shadePixel(intersectionInformation, rayDepth);
		}
		else
		{
			// TODO: Get scene background color
			return { .1f, .1f, .1f };
		}
	}

	void RayTracer::createJobs()
	{
		for (uint16_t numberOfTileY = 0; numberOfTileY*TILE_SIZE < this->renderHeight; numberOfTileY++)
		{
			for (uint16_t numberOfTileX = 0; numberOfTileX < this->renderWidth / TILE_SIZE; numberOfTileX++)
			{
				RenderJob job(
					numberOfTileX*TILE_SIZE,
					numberOfTileY*TILE_SIZE,
					numberOfTileX*TILE_SIZE + this->TILE_SIZE,
					numberOfTileY*TILE_SIZE + this->TILE_SIZE);
				this->renderJobs.pushBack(job);
			}
		}
	}

} // end of namespace raytracing