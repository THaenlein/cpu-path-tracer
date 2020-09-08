/*
 * BoundingBox.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <iostream>

#include "BoundingBox.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	BoundingBox::BoundingBox(aiMesh* mesh)
	{
		aiString name = mesh->mName;
		std::cout << "Calculating BBox of Mesh: " << name.C_Str() << std::endl;

		aiVector3D meshMin{ std::numeric_limits<float>::max() };
		aiVector3D meshMax{ -std::numeric_limits<float>::max() };
		if (mesh->HasPositions())
		{
			for (unsigned int currentVertex = 0; currentVertex < mesh->mNumVertices; currentVertex++)
			{
				aiVector3D& vertex = mesh->mVertices[currentVertex];

				meshMin.x = meshMin.x < vertex.x ? meshMin.x : vertex.x;
				meshMin.y = meshMin.y < vertex.y ? meshMin.y : vertex.y;
				meshMin.z = meshMin.z < vertex.z ? meshMin.z : vertex.z;

				meshMax.x = meshMax.x < vertex.x ? vertex.x : meshMax.x;
				meshMax.y = meshMax.y < vertex.y ? vertex.y : meshMax.y;
				meshMax.z = meshMax.z < vertex.z ? vertex.z : meshMax.z;
			}
		}

		this->min = meshMin;
		this->max = meshMax;
		this->containedMesh = mesh;
	}

	void BoundingBox::reset()
	{
		this->min = aiVector3D{ std::numeric_limits<float>::max() };
		this->max = aiVector3D{ -std::numeric_limits<float>::max() };
	}
		
	void BoundingBox::split(BoundingBox& left, BoundingBox& right)
	{
		// Evaluate longest axis
		aiVector3D length = this->max - this->min;
		uint8_t indexOfAxisToSplitAlong = length.x >= length.y ? (length.x >= length.z ? 0 : 2) : (length.y >= length.z ? 1 : 2);
		ai_real splitAt = length[indexOfAxisToSplitAlong] / 2.f;

		// Left box
		aiVector3D leftMin = this->min;
		aiVector3D leftMax = this->max;
		leftMax[indexOfAxisToSplitAlong] = splitAt;
		left.min = leftMin;
		left.max = leftMax;
		
		// Right box
		aiVector3D rightMin = this->min;
		aiVector3D rightMax = this->max;
		rightMin[indexOfAxisToSplitAlong] = splitAt;
		right.min = rightMin;
		right.max = rightMax;
	}

	bool BoundingBox::contains(std::vector<aiVector3D*> triangle)
	{
		return contains(triangle[0]) || contains(triangle[1]) || contains(triangle[2]);
	}

	bool BoundingBox::intersects(aiRay& ray)
	{
		aiVector3D tMin((this->min - ray.pos) / ray.dir);
		aiVector3D tMax((this->max - ray.pos) / ray.dir);
		float tMinimum = std::min(tMin[0], tMax[0]);
		float tMaximum = std::max(tMin[0], tMax[0]);

		tMinimum = std::max(tMinimum, std::min(tMin[1], tMax[1]));
		tMaximum = std::min(tMaximum, std::max(tMin[1], tMax[1]));
		tMinimum = std::max(tMinimum, std::min(tMin[2], tMax[2]));
		tMaximum = std::min(tMaximum, std::max(tMin[2], tMax[2]));

		return tMaximum >= tMinimum;
	}

	bool BoundingBox::operator==(const BoundingBox & other) const
	{
		return this->min.Equal(other.min) && this->max.Equal(other.max);
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

	bool BoundingBox::contains(aiVector3D* point)
	{
		bool xInRange = (this->min.x <= point->x) && (point->x <= this->max.x);
		bool yInRange = (this->min.y <= point->y) && (point->y <= this->max.y);
		bool zInRange = (this->min.z <= point->z) && (point->z <= this->max.z);
		return xInRange && yInRange && zInRange;
	}

	aiVector3D BoundingBox::getCenter()
	{
		return aiVector3D((this->min + this->max) * 1.f/2.f);
	}

} // end of namespace raytracer