/*
 * RayTracer.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>
#include <math.h>
#include <random>

#include "RayTracer.hpp"
#include "exceptions.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/


	void RayTracer::initialize()
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

	void RayTracer::renderSingleThreaded()
	{
		this->render();
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
#ifdef DEPTH_OF_FIELD
						// Uniform random point on the aperture
						float angle = getRandomFloat(0.f, 1.f) * 2.0f * M_PI;
						float radius = sqrt(getRandomFloat(0.f, 1.f));
						aiVector2D offset = aiVector2D(cos(angle), sin(angle)) * radius * this->renderSettings.getAperture();
						
						// Find to intersection point with the focal plane
						aiVector3D focalPoint(cameraPosition + this->renderSettings.getFocalDistance() * currentRay.dir);
						aiVector3D focalRayDir(focalPoint - (cameraPosition + (offset.x + offset.y)));
						focalRayDir.Normalize();

						// Calculate position of focal ray
						aiVector3D focalRayPos(cameraPosition);
						focalRayPos = focalRayPos + (offset.x + offset.y);

						currentRay.pos = focalRayPos;
						currentRay.dir = focalRayDir;
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


	bool RayTracer::rayTriangleIntersection(const aiRay& ray, std::vector<aiVector3D*> vecTriangle, aiVector3D* outIntersectionPoint, aiVector2D* outUV)
	{
		const float EPSILON = 1e-6f;
		// Invariant: A face always consists of 3 vertices
		// See: aiProcess_Triangulate flag in scene import
		aiVector3D* vertex0 = vecTriangle[0];
		aiVector3D* vertex1 = vecTriangle[1];
		aiVector3D* vertex2 = vecTriangle[2];
		aiVector3D edge1, edge2, pVec, tVec, qVec;
		float determinant, invDet, u, v;
		edge1 = *vertex1 - *vertex0;
		edge2 = *vertex2 - *vertex0;
		pVec = ray.dir ^ edge2;
		determinant = edge1 * pVec;
		if (determinant > -EPSILON && determinant < EPSILON)
		{
			return false; // This ray is parallel to this triangle.
		}
		invDet = 1.f / determinant;
		tVec = ray.pos - *vertex0;
		u = invDet * tVec * pVec;
		if (u < 0.0 || u > 1.0)
		{
			return false;
		}
		qVec = tVec ^ edge1;
		v = invDet * ray.dir * qVec;
		if (v < 0.0 || u + v > 1.0)
		{
			return false;
		}
		// At this stage we can compute t to find out where the intersection point is on the line.
		float t = invDet * edge2 * qVec;
		if (t > EPSILON) // ray intersection
		{
			*outIntersectionPoint = ray.pos + ray.dir * t;
			outUV->x = u;
			outUV->y = v;
			return true;
		}
		else // This means that there is a line intersection but not a ray intersection.
		{
			return false;
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

		// Calculate vertex normal for smooth shading
		aiVector3D smoothNormal = (1 - intersectionInformation.uv.x - intersectionInformation.uv.y) *
			*intersectionInformation.vertexNormals[0] + intersectionInformation.uv.x *
			*intersectionInformation.vertexNormals[1] + intersectionInformation.uv.y *
			*intersectionInformation.vertexNormals[2];
		smoothNormal.Normalize();

		const float pdf = 1 / (2 * M_PI);

		// Return immediately if hit object is emissive
		if (aiColor3D{0.f, 0.f, 0.f} < materialColorEmission)
		{
			return materialColorEmission;
		}

		// Russian roulette
		brdf = materialColorDiffuse / M_PI;
		const float probability = 
			(materialColorDiffuse.r > materialColorDiffuse.g) && (materialColorDiffuse.r>materialColorDiffuse.b) ? 
				materialColorDiffuse.r : materialColorDiffuse.g > materialColorDiffuse.b ? 
					materialColorDiffuse.g : materialColorDiffuse.b;
		float random = getRandomFloat(0.f, 1.f);
		if ((rayDepth + 1) > 5)
		{
			if (random < (probability * 0.9f))
			{
				brdf = brdf * (0.9f / probability);
			}
			else
			{
				// Russian roulette takes effect!
				return materialColorEmission;
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
			const float rx = getRandomFloat(0.f, 1.f);
			const float ry = getRandomFloat(0.f, 1.f);
			const float rz = getRandomFloat(0.f, 1.f);

			// Calculate reflection direction
			newRayDirection = calculateReflectionDirection(intersectionInformation.ray.dir, smoothNormal);
			newRayDirection = aiVector3D(
				newRayDirection.x + (rx - .5f ) * roughness,
				newRayDirection.y + (ry - .5f ) * roughness,
				newRayDirection.z + (rz - .5f ) * roughness);

			brdf = colorReflective / M_PI;
			newRayPosition = intersectionInformation.hitPoint + (newRayDirection * this->renderSettings.getBias());
			sampleRay = { newRayPosition, newRayDirection, RayType::REFLECTION };
		}
		else
		{
			// Perfect diffuse reflection
			const float r1 = getRandomFloat(0.f, 1.f);
			const float r2 = getRandomFloat(0.f, 1.f);
			newRayDirection = uniformSampleHemisphere(r1, r2);
			createCoordinateSystem(smoothNormal, Nt, Nb);

			// step 3: transform sample from world space to shaded point local coordinate system
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
				ai_real fresnelResult = fresnel(intersectionInformation.ray.dir, smoothNormal, refractionIndex);
				bool outside = (intersectionInformation.ray.dir * smoothNormal) < 0;
				aiVector3D bias = this->renderSettings.getBias() * smoothNormal;

				if (fresnelResult < 1.f)
				{
					// Ignore total internal reflection
					aiVector3D refractionDirection = calculateRefractionDirection(intersectionInformation.ray.dir, smoothNormal, refractionIndex);
					refractionDirection.Normalize();
					aiVector3D refractionPoint = outside ? intersectionInformation.hitPoint - bias : intersectionInformation.hitPoint + bias;
					aiRay refractionRay(refractionPoint, refractionDirection);
					refractionColor = traceRay(refractionRay, rayDepth + 1) * (1 - fresnelResult);
				}

				if ((fresnelResult > EPSILON) && (reflectivity > 0.f))
				{
					// Surface also is reflective. Cast reflection ray and blend color with fresnel
					aiVector3D reflectionDirection = calculateReflectionDirection(intersectionInformation.ray.dir, smoothNormal);
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
				aiVector3D reflectionDirection = calculateReflectionDirection(intersectionInformation.ray.dir, smoothNormal);
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
		float leastDistanceIntersection{ std::numeric_limits<float>::max() };
		std::vector<aiVector3D*> nearestIntersectedTriangle;
		std::vector<aiVector3D*> triangleVertexNormals;
		aiVector3D intersectionPoint;
		aiVector2D uvCoordinates;
		bool intersects{ false };
		// Iterate through meshes and faces and call rayTriangleIntersection()
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
				}
				// TODO: Collect all intersections and push them back into a collection
				// Evaluate nearest intersection point and return
				bool intersectsCurrentTriangle = this->rayTriangleIntersection(ray, nearestIntersectedTriangle, &intersectionPoint, &uvCoordinates);
				// We can immediately return if the cast ray is a shadow ray
				if (ray.type == RayType::SHADOW)
				{
					return true;
				}
				float distanceToIntersectionPoint = (intersectionPoint - ray.pos).Length();
				if (intersectsCurrentTriangle && (distanceToIntersectionPoint < leastDistanceIntersection))
				{
					leastDistanceIntersection = distanceToIntersectionPoint;
					outIntersection.hitMesh = mesh;
					outIntersection.hitTriangle = nearestIntersectedTriangle;
					outIntersection.hitPoint = intersectionPoint;
					outIntersection.ray = ray;
					outIntersection.uv = uvCoordinates;
					outIntersection.vertexNormals = triangleVertexNormals;
					intersects = true;
				}
				nearestIntersectedTriangle.clear();
				triangleVertexNormals.clear();
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
				RenderJob job(numberOfTileX*TILE_SIZE, numberOfTileY*TILE_SIZE,
					numberOfTileX*TILE_SIZE + this->TILE_SIZE, numberOfTileY*TILE_SIZE + this->TILE_SIZE);
				this->renderJobs.pushBack(job);
			}
		}
	}

	//     I->       N         R->
	//      \        |        /
	//       \       |       /
	//        \      |      /
	//         \     |     /
	//          \    |    /
	//           \   |   /
	//            \  |  /
	//             \ | /
	//   ___________\|/___________
	//          G R O U N D 
	aiVector3D RayTracer::calculateReflectionDirection(const aiVector3D& incidenceVector, const aiVector3D& incidenceNormal)
	{
		return { incidenceVector - 2 * (incidenceVector * incidenceNormal) * incidenceNormal };
	}

	aiVector3D RayTracer::calculateRefractionDirection(const aiVector3D& incidenceVector, const aiVector3D& incidenceNormal, const ai_real ior)
	{
		ai_real cosIncidence = std::clamp(incidenceVector * incidenceNormal, -1.0f, 1.0f);
		ai_real etaIncidence = 1.f;
		ai_real etaTransmission = ior;
		aiVector3D nRefraction(incidenceNormal);
		if (cosIncidence < 0.f)
		{
			// Inside medium
			cosIncidence = -cosIncidence;
		}
		else
		{
			// Outside medium
			std::swap(etaIncidence, etaTransmission);
			nRefraction = -incidenceNormal;
		}
		ai_real eta = etaIncidence / etaTransmission;
		ai_real criticalAngle = 1.f - eta * eta * (1.f - cosIncidence * cosIncidence);
		if (criticalAngle < 0.f)
		{
			return { 0.f, 0.f, 0.f};
		}
		else
		{
			return { eta * incidenceVector + (eta * cosIncidence - sqrtf(criticalAngle)) * nRefraction };
		}
	}

	ai_real RayTracer::fresnel(const aiVector3D& incidenceVector, const aiVector3D& incidenceNormal, const ai_real ior)
	{
		ai_real cosIncidence = std::clamp(incidenceVector * incidenceNormal, -1.f, 1.f);
		ai_real etaIncidence = 1.f;
		ai_real etaTransmission = ior;
		if (cosIncidence > 0.f) { std::swap(etaIncidence, etaTransmission); }
		// Snell's law
		ai_real sinTransmission = etaIncidence / etaTransmission * sqrtf(std::max(0.f, 1.f - cosIncidence * cosIncidence));
		
		if (sinTransmission >= 1.f) 
		{
			// Total internal reflection
			return 1.f;
		}
		else 
		{
			ai_real cosTransmission = sqrtf(std::max(0.f, 1.f - sinTransmission * sinTransmission));
			cosIncidence = fabsf(cosIncidence);
			ai_real Rs = ((etaTransmission * cosIncidence) - (etaIncidence * cosTransmission)) / ((etaTransmission * cosIncidence) + (etaIncidence * cosTransmission));
			ai_real Rp = ((etaIncidence * cosIncidence) - (etaTransmission * cosTransmission)) / ((etaIncidence * cosIncidence) + (etaTransmission * cosTransmission));
			ai_real reflectedLight = (Rs * Rs + Rp * Rp) / 2.f;
			return reflectedLight;
		}
	}

	void RayTracer::createCoordinateSystem(const aiVector3D& N, aiVector3D& Nt, aiVector3D& Nb)
	{
		if (std::fabs(N.x) > std::fabs(N.y))
		{
			Nt = aiVector3D(N.z, 0, -N.x) / sqrtf(N.x * N.x + N.z * N.z);
		}
		else
		{
			Nt = aiVector3D(0, -N.z, N.y) / sqrtf(N.y * N.y + N.z * N.z);
		}
		
		Nb = N ^ Nt;
	}

	aiVector3D RayTracer::uniformSampleHemisphere(const float r1, const float r2)
	{
		// cos(theta) = r1 = y
		// cos^2(theta) + sin^2(theta) = 1 -> sin(theta) = srtf(1 - cos^2(theta))
		
		float sinTheta = sqrtf(1 - r1 * r1);
		float phi = 2 * M_PI * r2;
		float x = sinTheta * cosf(phi);
		float z = sinTheta * sinf(phi);

		return aiVector3D(x, r1, z);
	}

	float RayTracer::getRandomFloat(unsigned int lowerBound, unsigned int upperBound)
	{
		static std::default_random_engine randomEngine;
		static std::uniform_real_distribution<> uniformDistribution(lowerBound, upperBound);
		return uniformDistribution(randomEngine);
	}

} // end of namespace raytracing