/*
 * RayTracer.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>
#include <math.h>
#include <ctime>

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

		// Using first camera in scene
		aiCamera* camera = (*this->scene->mCameras);

		float aspectRatio = camera->mAspect;
		float fieldOfView = camera->mHorizontalFOV;
		if (aspectRatio != 1.0f)
		{
			SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Aspect ratio of camera is not 1!");
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

		this->pixels = new Uint24[renderWidth * renderHeight];
	}


	void RayTracer::renderMultiThreaded()
	{
		RenderJob job;
		while (this->renderJobs.popFront(job))
		{
			SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Rendering Tile.");
#if ANTI_ALIASING
			this->renderAntiAliased(this->application, this->scene, job);
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

				this->pixels[currentPixel] = this->traceRay(currentRay);
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

				this->pixels[currentPixel] = this->traceRay(currentRay);
			}
		}
	}

	void RayTracer::renderAntiAliased(RenderJob& renderJob)
	{
		const unsigned int aa = RenderSettings::antiAliasingResolution;

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
						float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
						float aaShiftX = x + (p + r) / aa;
						float aaShiftY = y + (q + r) / aa;
						aiVector3D nextPixelX = this->pixelShiftX * static_cast<float>(aaShiftX);
						aiVector3D nextPixelY = this->pixelShiftY * static_cast<float>(aaShiftY);
						aiVector3D rayDirection = (this->topLeftPixel + nextPixelX + nextPixelY).Normalize();
						aiRay currentRay((*this->scene->mCameras)->mPosition, rayDirection);
						pixelAverage = pixelAverage + this->traceRay(currentRay);
					}
				}
				pixelAverage = pixelAverage * static_cast<ai_real>(1.f/std::pow(aa, 2U));
				this->pixels[currentPixel] = pixelAverage;
			}
		}
	}


	bool RayTracer::rayTriangleIntersection(const aiRay& ray, std::vector<aiVector3D*> vecTriangle, aiVector3D* outIntersectionPoint)
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
			return true;
		}
		else // This means that there is a line intersection but not a ray intersection.
		{
			return false;
		}
	}


	aiColor3D RayTracer::shadePixel(IntersectionInformation& intersectionInformation)
	{
		// Get mesh properties
		aiVector3D edge1 = *(intersectionInformation.hitTriangle[1]) - *(intersectionInformation.hitTriangle[0]);
		aiVector3D edge2 = *(intersectionInformation.hitTriangle[2]) - *(intersectionInformation.hitTriangle[0]);
		aiVector3D faceNormal = (edge1 ^ edge2).Normalize();
		unsigned int materialIndex = intersectionInformation.hitMesh->mMaterialIndex;
		aiMaterial* material = this->scene->mMaterials[materialIndex];
		aiColor3D materialColorDiffuse{}, materialColorEmission{}, materialColorAmbient{};
		int shadingModel{};
		material->Get(AI_MATKEY_COLOR_DIFFUSE, materialColorDiffuse);
		material->Get(AI_MATKEY_COLOR_EMISSIVE, materialColorEmission);
		material->Get(AI_MATKEY_COLOR_AMBIENT, materialColorAmbient);
		material->Get(AI_MATKEY_SHADING_MODEL, shadingModel);
		aiColor3D intersectionColor{ 0.f, 0.f, 0.f };

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

				aiRay shadowRay(intersectionInformation.hitPoint + (faceNormal * RenderSettings::bias), lightDirection);
				IntersectionInformation newIntersectionInfo;
				bool vis = !calculateIntersection(shadowRay, newIntersectionInfo);
				intersectionColor +=  materialColorDiffuse * vis * lightIntensity * std::max(0.f, faceNormal * lightDirection);
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
		aiVector3D intersectionPoint;
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
				}
				// TODO: Collect all intersections and push them back into a collection
				// Evaluate nearest intersection point and return
				bool intersectsCurrentTriangle = this->rayTriangleIntersection(ray, nearestIntersectedTriangle, &intersectionPoint);
				float distanceToIntersectionPoint = (intersectionPoint - ray.pos).Length();
				if (intersectsCurrentTriangle && (distanceToIntersectionPoint < leastDistanceIntersection))
				{
					leastDistanceIntersection = distanceToIntersectionPoint;
					outIntersection.hitMesh = mesh;
					outIntersection.hitTriangle = nearestIntersectedTriangle;
					outIntersection.hitPoint = intersectionPoint;
					// TODO: Evaluate if mNormals[currentFace] is ok since mNormals is mNumVertices in size
					// else calculate face normals
					//outFaceNormal = &mesh->mNormals[currentFace];
					intersects = true;
				}
				nearestIntersectedTriangle.clear();
			}
		}
		return intersects;
	}


	aiColor3D RayTracer::traceRay(aiRay& ray)
	{
		IntersectionInformation intersectionInformation;
		bool intersects = this->calculateIntersection(ray, intersectionInformation);
		if (intersects)
		{
			// Calculate color at the intersection
			return this->shadePixel(intersectionInformation);
		}
		else
		{
			// TODO: Get scene background color
			return { 0.f, 0.f, 0.f };
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

} // end of namespace raytracing