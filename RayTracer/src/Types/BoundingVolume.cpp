/*
 * <template>.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include "BoundingVolume.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

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
		aiVector3D intersectionPoint;
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
					}
					// Evaluate nearest intersection point and return
					bool intersectsCurrentTriangle = this->rayTriangleIntersection(ray, nearestIntersectedTriangle, &intersectionPoint);
					float distanceToIntersectionPoint = (intersectionPoint - ray.pos).Length();
					if (intersectsCurrentTriangle && (distanceToIntersectionPoint < leastDistanceIntersection))
					{
						leastDistanceIntersection = distanceToIntersectionPoint;
						outIntersection.hitMesh = intersectedMesh;
						outIntersection.hitTriangle = nearestIntersectedTriangle;
						outIntersection.hitPoint = intersectionPoint;
						outIntersection.ray = ray;
						intersects = true;
					}
					nearestIntersectedTriangle.clear();
				}
			}
		}
		return intersects;
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/


} // end of namespace raytracer