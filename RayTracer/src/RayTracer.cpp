/*
 * RayTracer.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>
#include <math.h>
#include <random>
#include <experimental\filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_DEFINE
#include "stb.h"

#include "RayTracer.hpp"
#include "exceptions.hpp"
#include "Utility\mathUtility.hpp"

#include "Textures\CheckerTexture.hpp"
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
			SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Aspect ratio of camera is not 1! Proceeding...");
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


		// Iterate over materials and load texture if it has one
		for (unsigned int currentMaterial = 0; currentMaterial < scene->mNumMaterials; currentMaterial++)
		{
			aiMaterial* material = scene->mMaterials[currentMaterial];
			unsigned int diffuseTextureCount = material->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE);
			if (diffuseTextureCount > 0)
			{
				aiString diffuseTexturePath;
				aiTextureMapping diffuseTextureMapping;
				if (material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexturePath, &diffuseTextureMapping) == aiReturn_SUCCESS)
				{
					if (diffuseTextureMapping != aiTextureMapping::aiTextureMapping_UV)
					{
						// Ignore anything other than UV-Mapping
						continue;
					}

					int width, height, components;
					std::experimental::filesystem::path texturePath = sceneDirPath;
					texturePath /= diffuseTexturePath.C_Str();

					// Load image from file
					uint8_t* image = stbi_load(texturePath.string().c_str(), &width, &height, &components, 0);

					if (!image)
					{
						SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Tried to load image from file: %s", texturePath.string().c_str());
						throw Renderer("Error on image texture load!");
					}
					else
					{
						SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Image texture successfully loaded: %s", texturePath.string().c_str());
						textureMapping[material] = new ImageTexture(image, width, height);
					}
				}
			}
		}
	}


	void RayTracer::renderMultiThreaded()
	{
		RenderJob job;
		while (this->renderJobs.popFront(job))
		{
#if ANTI_ALIASING
			this->renderAntiAliased(job);
#else
			this->render(job);
#endif
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
		for (uint16_t x = renderJob.getTileStartX(); x < renderJob.getTileEndX(); x++)
		{
			for (uint16_t y = renderJob.getTileStartY(); y < renderJob.getTileEndY(); y++)
			{
				uint32_t currentPixel = y * renderWidth + x;
				aiVector3D nextPixelX = this->pixelShiftX * static_cast<float>(x);
				aiVector3D nextPixelY = this->pixelShiftY * static_cast<float>(y);
				aiVector3D rayDirection = (this->topLeftPixel + nextPixelX - nextPixelY).Normalize();
				aiRay currentRay((*this->scene->mCameras)->mPosition, rayDirection);

#if PATH_TRACE
				this->pixels[currentPixel] = this->tracePath(currentRay);
#else
				this->pixels[currentPixel] = this->traceRay(currentRay);
#endif
			}
		}
	}

	void RayTracer::renderAntiAliased(RenderJob& renderJob)
	{
		const uint8_t aa = this->renderSettings.getAAResolution();
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
						float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
						float aaShiftX = x + (p + r) / aa;
						float aaShiftY = y + (q + r) / aa;

						aiVector3D nextPixelX = this->pixelShiftX * static_cast<float>(aaShiftX);
						aiVector3D nextPixelY = this->pixelShiftY * static_cast<float>(aaShiftY);
						aiVector3D rayDirection = (this->topLeftPixel + nextPixelX - nextPixelY).Normalize();
						aiRay currentRay(cameraPosition, rayDirection);

						// DOF
#if DEPTH_OF_FIELD
						mathUtility::calculateDepthOfFieldRay(
							&currentRay, 
							this->renderSettings.getAperture(), 
							this->renderSettings.getFocalDistance());
#endif

#if PATH_TRACE
						pixelAverage = pixelAverage + this->tracePath(currentRay);
#else
						pixelAverage = pixelAverage + this->traceRay(currentRay);
#endif
					}
				}
				pixelAverage = pixelAverage * static_cast<ai_real>(1.f/std::pow(aa, 2U));
				this->pixels[currentPixel] = pixelAverage;
			}
		}
	}

	aiColor3D RayTracer::sampleLight(IntersectionInformation& intersectionInformation, uint8_t rayDepth)
	{
		unsigned int materialIndex = intersectionInformation.hitMesh->mMaterialIndex;
		aiMaterial* material = this->scene->mMaterials[materialIndex];
		aiColor3D materialColorDiffuse{}, materialColorEmission{}, materialColorAmbient{}, colorReflective{}, brdf{}, incomingLight{};
		ai_real reflectivity{}, opacity{ 1 }, refractionIndex{ 1 }, shininess{};
		material->Get(AI_MATKEY_SHININESS, shininess);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, materialColorDiffuse);
		material->Get(AI_MATKEY_COLOR_EMISSIVE, materialColorEmission);
		material->Get(AI_MATKEY_COLOR_AMBIENT, materialColorAmbient);
		material->Get(AI_MATKEY_REFLECTIVITY, reflectivity);
		material->Get(AI_MATKEY_COLOR_REFLECTIVE, colorReflective);
		material->Get(AI_MATKEY_OPACITY, opacity);
		material->Get(AI_MATKEY_REFRACTI, refractionIndex);
		Texture* materialTexture = this->textureMapping[material];
		if (materialTexture)
		{
			materialColorDiffuse = materialTexture->getColor(intersectionInformation.uvTextureCoords.x, intersectionInformation.uvTextureCoords.y);
		}

		// Calculate vertex normal for smooth shading
		aiVector3D smoothNormal = (1 - intersectionInformation.uv.x - intersectionInformation.uv.y) *
			*intersectionInformation.vertexNormals[0] + intersectionInformation.uv.x *
			*intersectionInformation.vertexNormals[1] + intersectionInformation.uv.y *
			*intersectionInformation.vertexNormals[2];
		smoothNormal.Normalize();

		const float pdf = 1.f / (2.f * PI);

		// Return immediately if hit object is emissive
		if (aiColor3D{0.f, 0.f, 0.f} < materialColorEmission)
		{
			return materialColorEmission;
		}

		// Russian roulette
		brdf = materialColorDiffuse / PI;
		const float probability = 
			(materialColorDiffuse.r > materialColorDiffuse.g) && (materialColorDiffuse.r>materialColorDiffuse.b) ? 
				materialColorDiffuse.r : materialColorDiffuse.g > materialColorDiffuse.b ? 
					materialColorDiffuse.g : materialColorDiffuse.b;
		if ((rayDepth + 1) > 5)
		{
			if (mathUtility::russianRoulette(probability, rayDepth))
			{
				// Russian roulette takes effect!
				return materialColorEmission;
			}
			else
			{
				brdf = brdf * (0.9f / probability);
			}
		}

		// Compute indirect light
		aiColor3D indirectDiffuse{ 0.f, 0.f, 0.f }, indirectLight{ 0.f, 0.f, 0.f };
		aiVector3D Nt{}, Nb{}, newRayDirection{}, newRayPosition{};
		aiRay sampleRay{};

		if (reflectivity > 0.f)
		{
			// Perfect specular reflection
			const float roughness = 0.f;
			const float rx = mathUtility::getRandomFloat(0.f, 1.f);
			const float ry = mathUtility::getRandomFloat(0.f, 1.f);
			const float rz = mathUtility::getRandomFloat(0.f, 1.f);

			// Calculate reflection direction
			newRayDirection = mathUtility::calculateReflectionDirection(intersectionInformation.ray.dir, smoothNormal);
			newRayDirection = aiVector3D(
				newRayDirection.x + (rx - .5f ) * roughness,
				newRayDirection.y + (ry - .5f ) * roughness,
				newRayDirection.z + (rz - .5f ) * roughness);

			brdf = colorReflective / PI;
			newRayPosition = intersectionInformation.hitPoint + (newRayDirection * this->renderSettings.getBias());
			sampleRay = { newRayPosition, newRayDirection, RayType::REFLECTION };
		}
		else
		{
			// Perfect diffuse reflection
			const float r1 = mathUtility::getRandomFloat(0.f, 1.f);
			const float r2 = mathUtility::getRandomFloat(0.f, 1.f);
			newRayDirection = mathUtility::uniformSampleHemisphere(r1, r2);
			mathUtility::createCoordinateSystem(smoothNormal, Nt, Nb);

			// Transform sample from world space to shaded point local coordinate system
			aiMatrix3x3 toLocalMatrix
			{ Nt.x, Nt.y, Nt.z,
				smoothNormal.x, smoothNormal.y, smoothNormal.z,
				Nb.x, Nb.y, Nb.z };
			newRayDirection *= toLocalMatrix;

			newRayPosition = intersectionInformation.hitPoint + (newRayDirection * this->renderSettings.getBias());
			sampleRay = { newRayPosition, newRayDirection, RayType::INDIRECT_DIFFUSE };
		}

		// Cast a ray in calculated direction
		incomingLight = tracePath(sampleRay, rayDepth + 1);
		float cos_theta = newRayDirection * smoothNormal;

		return  materialColorEmission + (brdf * incomingLight * cos_theta / pdf);
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
		ai_real reflectivity{}, opacity{ 1 }, refractionIndex{ 1 };
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
				const ai_real EPSILON = 1e-3f;
				aiColor3D refractionColor{}, reflectionColor{};
				ai_real fresnelResult = mathUtility::fresnel(intersectionInformation.ray.dir, smoothNormal, refractionIndex);
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