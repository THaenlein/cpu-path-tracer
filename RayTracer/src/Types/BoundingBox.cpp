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

	bool BoundingBox::operator==(const BoundingBox & other) const
	{
		return this->min.Equal(other.min) && this->max.Equal(other.max);
	}

	BoundingBox::BoundingBox(aiMesh* mesh)
	{
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

	BoundingBox::BoundingBox(std::vector<KdTriangle>& triangles)
	{
		aiVector3D meshMin{ std::numeric_limits<float>::max() };
		aiVector3D meshMax{ -std::numeric_limits<float>::max() };
		
		for (KdTriangle& currentPair : triangles)
		{
			aiFace* triangle{ currentPair.faceMeshPair.first };
			aiMesh* associatedMesh{ currentPair.faceMeshPair.second };

			for (unsigned int currentIndex = 0; currentIndex < triangle->mNumIndices; currentIndex++)
			{
				aiVector3D& vertex = associatedMesh->mVertices[triangle->mIndices[currentIndex]];
				meshMin.x = meshMin.x < vertex.x ? meshMin.x : vertex.x;
				meshMin.y = meshMin.y < vertex.y ? meshMin.y : vertex.y;
				meshMin.z = meshMin.z < vertex.z ? meshMin.z : vertex.z;

				meshMax.x = meshMax.x < vertex.x ? vertex.x : meshMax.x;
				meshMax.y = meshMax.y < vertex.y ? vertex.y : meshMax.y;
				meshMax.z = meshMax.z < vertex.z ? vertex.z : meshMax.z;
			}
			this->min = meshMin;
			this->max = meshMax;
		}
	}

	BoundingBox::BoundingBox(const KdTriangle& triangle)
	{
		aiVector3D triMin{ std::numeric_limits<float>::max() };
		aiVector3D triMax{ -std::numeric_limits<float>::max() };

		aiFace* tri = triangle.faceMeshPair.first;
		aiMesh* associatedMesh = triangle.faceMeshPair.second;

		for (unsigned int currentIndex = 0; currentIndex < tri->mNumIndices; currentIndex++)
		{
			aiVector3D& vertex = associatedMesh->mVertices[tri->mIndices[currentIndex]];
			triMin.x = triMin.x < vertex.x ? triMin.x : vertex.x;
			triMin.y = triMin.y < vertex.y ? triMin.y : vertex.y;
			triMin.z = triMin.z < vertex.z ? triMin.z : vertex.z;

			triMax.x = triMax.x < vertex.x ? vertex.x : triMax.x;
			triMax.y = triMax.y < vertex.y ? vertex.y : triMax.y;
			triMax.z = triMax.z < vertex.z ? vertex.z : triMax.z;
		}

		this->min = triMin;
		this->max = triMax;
	}

	BoundingBox::BoundingBox(std::pair<aiFace*, aiMesh*>& triangle)
	{
		aiVector3D triMin{ std::numeric_limits<float>::max() };
		aiVector3D triMax{ -std::numeric_limits<float>::max() };

		aiFace* tri = triangle.first;
		aiMesh* associatedMesh = triangle.second;
		
		for (unsigned int currentIndex = 0; currentIndex < tri->mNumIndices; currentIndex++)
		{
			aiVector3D& vertex = associatedMesh->mVertices[tri->mIndices[currentIndex]];
			triMin.x = triMin.x < vertex.x ? triMin.x : vertex.x;
			triMin.y = triMin.y < vertex.y ? triMin.y : vertex.y;
			triMin.z = triMin.z < vertex.z ? triMin.z : vertex.z;

			triMax.x = triMax.x < vertex.x ? vertex.x : triMax.x;
			triMax.y = triMax.y < vertex.y ? vertex.y : triMax.y;
			triMax.z = triMax.z < vertex.z ? vertex.z : triMax.z;
		}

		this->min = triMin;
		this->max = triMax;
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
		leftMax[indexOfAxisToSplitAlong] -= splitAt;
		left.min = leftMin;
		left.max = leftMax;
		
		// Right box
		aiVector3D rightMin = this->min;
		aiVector3D rightMax = this->max;
		rightMin[indexOfAxisToSplitAlong] += splitAt;
		right.min = rightMin;
		right.max = rightMax;
	}

	void BoundingBox::split(BoundingBox& left, BoundingBox& right, uint8_t splitAxis)
	{
		// Evaluate longest axis
		aiVector3D length = this->max - this->min;
		ai_real splitAt = length[splitAxis] / 2.f;

		// Left box
		aiVector3D leftMin = this->min;
		aiVector3D leftMax = this->max;
		leftMax[splitAxis] -= splitAt;
		left.min = leftMin;
		left.max = leftMax;

		// Right box
		aiVector3D rightMin = this->min;
		aiVector3D rightMax = this->max;
		rightMin[splitAxis] += splitAt;
		right.min = rightMin;
		right.max = rightMax;
	}

	void BoundingBox::split(BoundingBox& left, BoundingBox& right, const Plane& splitPlane) const
	{
		float planePosition = splitPlane.getPosition();
		Axis planeAxis = splitPlane.getAxis();

		// TODO: Check if plane is in box

		left = *this;
		right = *this;

		left.max[planeAxis] = planePosition;
		right.min[planeAxis] = planePosition;
	}

	bool BoundingBox::contains(std::vector<aiVector3D*> triangle) const
	{
		return contains(triangle[0]) || contains(triangle[1]) || contains(triangle[2]);
	}

	bool BoundingBox::contains(std::pair<aiFace*, aiMesh*>& triangleMeshPair) const
	{
		aiFace* triangle{ triangleMeshPair.first };
		aiMesh* associatedMesh{ triangleMeshPair.second };
		return contains(&associatedMesh->mVertices[triangle->mIndices[0]]) ||
			   contains(&associatedMesh->mVertices[triangle->mIndices[1]]) ||
			   contains(&associatedMesh->mVertices[triangle->mIndices[2]]);
	}

	bool BoundingBox::contains(aiVector3D* point) const
	{
		bool xInRange = (this->min.x <= point->x) && (point->x <= this->max.x);
		bool yInRange = (this->min.y <= point->y) && (point->y <= this->max.y);
		bool zInRange = (this->min.z <= point->z) && (point->z <= this->max.z);
		return xInRange && yInRange && zInRange;
	}

	bool BoundingBox::exclusiveContains(std::vector<aiVector3D*> triangle) const
	{
		return contains(triangle[0]) && contains(triangle[1]) && contains(triangle[2]);
	}

	bool BoundingBox::exclusiveContains(std::pair<aiFace*, aiMesh*>& triangleMeshPair) const
	{
		aiFace* triangle{ triangleMeshPair.first };
		aiMesh* associatedMesh{ triangleMeshPair.second };
		return contains(&associatedMesh->mVertices[triangle->mIndices[0]]) &&
			   contains(&associatedMesh->mVertices[triangle->mIndices[1]]) &&
			   contains(&associatedMesh->mVertices[triangle->mIndices[2]]);
	}

	bool BoundingBox::intersects(const aiRay& ray) const
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

	const float BoundingBox::getSurfaceArea() const
	{
		aiVector3D length = this->max - this->min;
		return (length.x * length.y + length.y * length.z + length.x * length.z) * 2;
	}

	aiVector3D BoundingBox::getCenter() const
	{
		return aiVector3D((this->min + this->max) / 2.f);
	}

	bool BoundingBox::isPlanar(float epsilon/* = 1e-6*/) const
	{
		return 
			std::abs(this->max.x - this->min.x) <= epsilon ||
			std::abs(this->max.y - this->min.y) <= epsilon ||
			std::abs(this->max.z - this->min.z) <= epsilon;
	}

	bool BoundingBox::isPlanar(Axis axis, float epsilon/* = 1e-6*/) const
	{
		return std::abs(this->max[axis] - this->min[axis]) <= epsilon;
	}

	float BoundingBox::length(const Axis axis) const
	{
		switch (axis)
		{
		case Axis::X:
			return std::abs(this->max.x - this->min.x);
			break;
	
		case Axis::Y:
			return std::abs(this->max.y - this->min.y);
			break;
	
		case Axis::Z:
			return std::abs(this->max.z - this->min.z);
			break;
		default:
			return std::abs((this->max - this->min).Length());
		}
	}

	void BoundingBox::clipToBox(const BoundingBox& box)
	{
		for (int k = 0; k < 3; k++) 
		{
			if (box.min[k] > this->min[k])
			{
				this->min[k] = box.min[k];
			}
			if (box.max[k] < this->max[k])
			{
				this->max[k] = box.max[k];
			}
		}
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

} // end of namespace raytracer