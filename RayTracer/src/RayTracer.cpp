/*
 * RayTracer.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>
#include <math.h>
#include <ctime>

#include "RayTracer.hpp"
#include "exceptions.hpp"
#include "ErrorHandler.hpp"


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
			raytracing::ErrorHandler::getInstance().reportError("Aspect ratio of camera is not 1!");
		}

		aiVector3D cameraUp = camera->mUp.Normalize();
		aiVector3D lookAt = camera->mLookAt.Normalize();
		aiVector3D cameraRight = (cameraUp ^ lookAt).Normalize();

		float halfViewportWidth = std::tan(fieldOfView / 2.0f);
		float halfViewportHeight = halfViewportWidth * aspectRatio;

		this->pixelShiftX = ((2 * halfViewportWidth) / (renderWidth)) * cameraRight;
		this->pixelShiftY = ((2 * halfViewportHeight) / (renderHeight)) * cameraUp;
		this->bottomLeftPixel = lookAt - (halfViewportWidth * cameraRight) - (halfViewportHeight * cameraUp);

		this->createJobs();

		this->pixels = new Uint24[renderWidth * renderHeight];
	}


	void RayTracer::renderMultiThreaded()
	{
		RenderJob job;
		while (this->renderJobs.popFront(job))
		{
			ErrorHandler::getInstance().reportDebug("Rendering Tile.");
			this->render(this->application, this->scene, job);
			std::this_thread::yield();
		}
	}

	void RayTracer::renderSingleThreaded()
	{
		this->render(this->application, this->scene);
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

	void RayTracer::render(Application& application, const aiScene* scene)
	{
		for (uint16_t x = 0; x < this->renderWidth; x++)
		{
			for (uint16_t y = 0; y < this->renderHeight; y++)
			{
				uint32_t currentPixel = y * renderWidth + x;
				aiVector3D rayDirection = (this->bottomLeftPixel + (this->pixelShiftX * static_cast<float>(x)) + (this->pixelShiftY * static_cast<float>(y))).Normalize();
				aiRay currentRay((*scene->mCameras)->mPosition, rayDirection);

				this->pixels[currentPixel] = this->traceRay(currentRay, scene);
			}
		}
	}


	void RayTracer::render(Application& application, const aiScene* scene, RenderJob& renderJob)
	{
		for (uint16_t x = renderJob.getTileStartX(); x < renderJob.getTileEndX(); x++)
		{
			for (uint16_t y = renderJob.getTileStartY(); y < renderJob.getTileEndY(); y++)
			{
				uint32_t currentPixel = y * renderWidth + x;
				aiVector3D nextPixelX = this->pixelShiftX * static_cast<float>(x);
				aiVector3D nextPixelY = this->pixelShiftY * static_cast<float>(y);
				aiVector3D rayDirection = (this->bottomLeftPixel + nextPixelX + nextPixelY).Normalize();
				aiRay currentRay((*scene->mCameras)->mPosition, rayDirection);

				this->pixels[currentPixel] = this->traceRay(currentRay, scene);
			}
		}
	}


	bool RayTracer::rayTriangleIntersection(aiRay& ray, std::vector<aiVector3D*> vecTriangle, aiVector3D* outIntersectionPoint)
	{
		const float EPSILON = 10e-7;
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
		invDet = 1.0 / determinant;
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


	Uint24 RayTracer::shadePixel(const aiScene* scene, IntersectionInformation& intersectionInformation)
	{
		// Get mesh properties
		aiVector3D edge1 = *(intersectionInformation.hitTriangle[1]) - *(intersectionInformation.hitTriangle[0]);
		aiVector3D edge2 = *(intersectionInformation.hitTriangle[2]) - *(intersectionInformation.hitTriangle[0]);
		aiVector3D calculatedNormal = (edge1 ^ edge2).Normalize();
		unsigned int materialIndex = intersectionInformation.hitMesh->mMaterialIndex;
		aiMaterial* material = scene->mMaterials[materialIndex];
		aiColor3D materialColorDiffuse;
		material->Get(AI_MATKEY_COLOR_DIFFUSE, materialColorDiffuse);

		// TODO: Calculate color for multiple light sources
		aiLight* light = scene->mLights[0];
		aiVector3D lightDirection = (light->mPosition - intersectionInformation.hitPoint);
		float r2 = lightDirection.SquareLength();
		float distance = lightDirection.Length();
		lightDirection /= distance;
		aiColor3D lightIntensity = light->mColorDiffuse * (1/(4 * M_PI * r2));

		// Only directional lights are supported atm!
		aiVector3D sunLightDirection = -light->mDirection;
		aiRay shadowRay(intersectionInformation.hitPoint + (calculatedNormal * 0.001f), lightDirection);
		aiVector3D* newFaceNormal{};
		IntersectionInformation newIntersectionInfo;
		bool vis = !calculateIntersection(shadowRay, scene, newIntersectionInfo);
		// TODO: Fix broken shading for high intensity lights
		aiColor3D intersectionColor =  materialColorDiffuse * vis /*TODO: Use vis*/ * light->mColorDiffuse * std::max(0.f, calculatedNormal * lightDirection);
		return { intersectionColor };
	}


	bool RayTracer::calculateIntersection(
		aiRay& ray,
		const aiScene* scene,
		IntersectionInformation& outIntersection)
	{
		float leastDistanceIntersection{ std::numeric_limits<float>::max() };
		std::vector<aiVector3D*> nearestIntersectedTriangle;
		aiVector3D intersectionPoint;
		bool intersects{ false };
		// Iterate through meshes and faces and call rayTriangleIntersection()
		for (unsigned int currentMesh = 0; currentMesh < scene->mNumMeshes; currentMesh++)
		{
			aiString name = scene->mMeshes[currentMesh]->mName;
			aiMesh* mesh = scene->mMeshes[currentMesh];

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


	Uint24 RayTracer::traceRay(aiRay& ray, const aiScene* scene)
	{
		std::vector<aiVector3D*> vecTriangle;
		aiMesh* intersectedMesh{};
		aiVector3D intersection;
		aiVector3D* normal{};
		IntersectionInformation intersectionInformation;
		bool intersects = this->calculateIntersection(ray, scene, intersectionInformation);
		if (intersects)
		{
			// Calculate color at the intersection
			return this->shadePixel(scene, intersectionInformation);
		}
		else
		{
			// TODO: Get scene background color
			return { 0x000000 };
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