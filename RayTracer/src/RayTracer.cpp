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


namespace raytracer
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	bool RayTracer::rayTriangleIntersection(aiRay& ray, std::vector<aiVector3D*> vecTriangle, aiVector3D* outIntersectionPoint)
	{
		const float EPSILON = 10e-7;//0.0000001;
		// Invariant: A face always consists of 3 vertices
		// See: aiProcess_Triangulate flag in scene import
		aiVector3D* vertex0 = vecTriangle[0];//*(inTriangle[0]);
		aiVector3D* vertex1 = vecTriangle[1];//*(inTriangle[1]);
		aiVector3D* vertex2 = vecTriangle[2];//*(inTriangle[2]);
		aiVector3D edge1, edge2, pVec, tVec, qVec;
		float determinant, invDet, u, v;
		edge1 = *vertex1 - *vertex0;
		edge2 = *vertex2 - *vertex0;
		pVec = ray.dir ^ edge2;
		determinant = edge1 * pVec;
		if (determinant > -EPSILON && determinant < EPSILON)
		{
			return false;    // This ray is parallel to this triangle.
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


	Uint24 RayTracer::shadePixel(aiVector3D* intersectionPoint, const aiScene* scene, std::vector<aiVector3D*> triangle, aiMesh* mesh)
	{
		// Get mesh properties
		aiVector3D edge1 = *triangle[1] - *triangle[0];
		aiVector3D edge2 = *triangle[2] - *triangle[0];
		aiVector3D faceNormal = (edge1 ^ edge2).Normalize();
		unsigned int materialIndex = mesh->mMaterialIndex;
		aiMaterial* material = scene->mMaterials[materialIndex];
		aiColor3D materialColorDiffuse;
		material->Get(AI_MATKEY_COLOR_DIFFUSE, materialColorDiffuse);

		// TODO: Calculate color for multiple light sources
		aiLight* light = scene->mLights[0];
		aiVector3D lightDirection = (light->mPosition - *intersectionPoint).Normalize();
		// TODO: Fix broken shading for high intensity lights
		aiColor3D intersectionColor = (materialColorDiffuse * light->mColorDiffuse * std::max(0.f, faceNormal * lightDirection));
		return { intersectionColor };
	}


	bool RayTracer::calculateIntersection(
		aiRay& ray, 
		const aiScene* scene, 
		std::vector<aiVector3D*>& outIntersectedTriangle, 
		aiMesh*& outIntersectedMesh, 
		aiVector3D* outIntersectionPoint)
	{
		float leastDistanceIntersection{std::numeric_limits<float>::max()};
		std::vector<aiVector3D*> nearestIntersectedTriangle;
		bool intersects{ false };
		// Iterate through meshes and faces and call rayTriangleIntersection()
		for (unsigned int currentMesh = 0; currentMesh < scene->mNumMeshes; currentMesh++)
		{
			aiString name = scene->mMeshes[currentMesh]->mName;
			//ErrorHandler::getInstance().reportDebug("Calculating intersections for mesh: ", name.C_Str());
			aiMesh* mesh = scene->mMeshes[currentMesh];

			//std::cout << "Name of Mesh " << currentMesh << ": " << name.C_Str() << std::endl;
			for (unsigned int currentFace = 0; currentFace < mesh->mNumFaces; currentFace++)
			{
				aiFace* face = &(mesh->mFaces[currentFace]);
				// TODO: Change to std::unique_ptr if possible
				for (unsigned int currentIndex = 0; currentIndex < face->mNumIndices; currentIndex++)
				{
					nearestIntersectedTriangle.push_back(&(mesh->mVertices[face->mIndices[currentIndex]]));
				}
				// TODO: Collect all intersections and push them back into a collection
				// Evaluate nearest intersection point and return
				bool intersectsCurrentTriangle = this->rayTriangleIntersection(ray, nearestIntersectedTriangle, outIntersectionPoint);
				float distanceToIntersectionPoint = (*outIntersectionPoint - ray.pos).SquareLength();
				if (intersectsCurrentTriangle && (distanceToIntersectionPoint < leastDistanceIntersection))
				{
					leastDistanceIntersection = distanceToIntersectionPoint;
					outIntersectedMesh = mesh;
					outIntersectedTriangle = nearestIntersectedTriangle;
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
		bool intersects = this->calculateIntersection(ray, scene, vecTriangle, intersectedMesh, &intersection);
		if (!intersects)
		{
			// TODO: Get scene background color
			return { 0x000000 };
		}
		else
		{
			// Calculate color at the intersection
			return this->shadePixel(&intersection, scene, vecTriangle, intersectedMesh);
		}
	}


	void RayTracer::render(Application* application, const aiScene* scene)
	{
		uint16_t renderWidth = application->getRenderWidth();
		uint16_t renderHeight = application->getRenderHeight();

		// Using first camera in scene
		aiCamera* camera = (*scene->mCameras);

		float aspectRatio = camera->mAspect;
		float fieldOfView = camera->mHorizontalFOV;
		if (aspectRatio != 1.0f)
		{
			raytracer::ErrorHandler::getInstance().reportError("Aspect ratio of camera is not 1!");
		}

		aiVector3D cameraUp = camera->mUp.Normalize();
		aiVector3D lookAt = camera->mLookAt.Normalize();
		aiVector3D cameraRight = (cameraUp ^ lookAt).Normalize();

		float halfViewportWidth = std::tan(fieldOfView / 2.0f);
		float halfViewportHeight = halfViewportWidth * aspectRatio;

		aiVector3D pixelShiftX = ((2 * halfViewportWidth) / (renderWidth)) * cameraRight;
		aiVector3D pixelShiftY = ((2 * halfViewportHeight) / (renderHeight)) * cameraUp;
		aiVector3D bottomLeftPixel = lookAt - (halfViewportWidth * cameraRight) - (halfViewportHeight * cameraUp);

		// TODO: Evaluate how much memory is internally used: RGB (3 Byte) or RGBA (4 Byte) per pixel?
		// Using 2d-array of 24-bit integer to encode color
		// rrrrrrrr gggggggg bbbbbbbb
		Uint24* pixels = new Uint24[renderWidth * renderHeight];
		Uint24 mintGreen{ 0x99E8CA };
		
		clock_t start = clock();

		for (uint16_t x = 0; x < renderWidth; x++)
		{
			for (uint16_t y = 0; y < renderHeight; y++)
			{
				uint32_t currentPixel = y * renderWidth + x;
				// TODO: Check why rayDirection in first pixel is different from bottomLeftPixel?
				aiVector3D rayDirection = (bottomLeftPixel + (pixelShiftX * static_cast<float>(x)) + (pixelShiftY * static_cast<float>(y))).Normalize();
				aiRay currentRay(camera->mPosition, rayDirection);

				pixels[currentPixel] = this->traceRay(currentRay, scene);
			}
			std::cout << "Done with row: " << x << std::endl;
		}

		clock_t end = clock();
		double elapsedTimeInSec = double(end - start) / CLOCKS_PER_SEC;
		BOOST_LOG_TRIVIAL(info) << "Elapsed time for rendering scene: " << elapsedTimeInSec << " seconds.";

		application->presentRender(pixels);
		delete[] pixels;
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/


} // end of namespace raytracer