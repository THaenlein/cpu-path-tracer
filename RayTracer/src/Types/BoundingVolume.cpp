/*
 * <template>.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include "BoundingVolume.hpp"

#include "Utility\mathUtility.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	using namespace utility;

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	void BoundingVolume::initialize(const aiScene* scene)
	{
		if (scene->HasMeshes())
		{
			aiMesh** meshes = scene->mMeshes;
			unsigned int numberOfMeshes = scene->mNumMeshes;
			for (unsigned int currentMesh = 0; currentMesh < numberOfMeshes; currentMesh++)
			{
				aiMesh* mesh = meshes[currentMesh];
				this->bBoxes.push_back(
					std::unique_ptr<BoundingBox>(new raytracing::BoundingBox(mesh)));
			}
		}
	}

	bool BoundingVolume::calculateIntersection(aiRay& ray, IntersectionInformation& outIntersection)
	{
		float leastDistanceIntersection{ std::numeric_limits<float>::max() };
		std::vector<aiVector3D*> nearestIntersectedTriangle;
		std::vector<aiVector3D*> triangleVertexNormals;
		aiVector3D intersectionPoint;
		aiVector2D uvCoordinates;
		bool intersects{ false };

		for (std::unique_ptr<BoundingBox>& box : this->bBoxes)
		{
			if (box->intersects(ray))
			{
				const aiMesh* intersectedMesh = box->getContainedMesh();
				for (unsigned int currentFace = 0; currentFace < intersectedMesh->mNumFaces; currentFace++)
				{
					aiFace* face = &(intersectedMesh->mFaces[currentFace]);
					for (unsigned int currentIndex = 0; currentIndex < face->mNumIndices; currentIndex++)
					{
						nearestIntersectedTriangle.push_back(&(intersectedMesh->mVertices[face->mIndices[currentIndex]]));
						triangleVertexNormals.push_back(&(intersectedMesh->mNormals[face->mIndices[currentIndex]]));
					}
					// Evaluate nearest intersection point and return
					bool intersectsCurrentTriangle = mathUtility::rayTriangleIntersection(ray, nearestIntersectedTriangle, &intersectionPoint, &uvCoordinates);
					// We can immediately return if the cast ray is a shadow ray
					if (intersectsCurrentTriangle && (ray.type == RayType::SHADOW))
					{
						return true;
					}
					float distanceToIntersectionPoint = (intersectionPoint - ray.pos).Length();
					if (intersectsCurrentTriangle && (distanceToIntersectionPoint < leastDistanceIntersection))
					{
						leastDistanceIntersection = distanceToIntersectionPoint;
						outIntersection.hitMesh = intersectedMesh;
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
		}
		return intersects;
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/


} // end of namespace raytracer