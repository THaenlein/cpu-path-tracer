/*
 * KdNode.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include "KdNode.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	/*static*/ KdNode* KdNode::buildTree(std::vector<std::pair<aiFace*, aiMesh*>>& triangles)
	{
		// Create BBox of full scene
		BoundingBox rootBox(triangles);

		return build(triangles, rootBox);
	}

	/*static*/ KdNode* KdNode::build(std::vector<std::pair<aiFace*, aiMesh*>>& triangles, BoundingBox bBox, unsigned int depth/* = 0*/)
	{
		const ai_real EPSILON = 1e-3f;

		if ((triangles.size() <= MAX_TRIANGLES_PER_LEAF) || (depth == MAX_DEPTH))
		{
			// Return leaf node
			return new KdNode(triangles, bBox);
		}

		// Find plane to split
		// TODO: Implement SAH

		// Split box
		BoundingBox leftBox;
		BoundingBox rightBox;
		bBox.split(leftBox, rightBox, depth % 3);

		// Add left and overlapping tris to left
		// Add right and overlapping tris to right
		std::vector<std::pair<aiFace*, aiMesh*>> trianglesLeft;
		std::vector<std::pair<aiFace*, aiMesh*>> trianglesRight;
		aiVector3D centerPoint{ bBox.getCenter() };

		for (std::pair<aiFace*, aiMesh*>& currentPair : triangles)
		{
			BoundingBox triangleBounds{ currentPair };

			if (triangleBounds.getMin()[depth % 3] < (centerPoint[depth % 3] + EPSILON))
			{
				trianglesLeft.push_back(currentPair);
			}
			if (triangleBounds.getMax()[depth % 3] >= (centerPoint[depth % 3] - EPSILON))
			{
				trianglesRight.push_back(currentPair);
			}
		}

		// To avoid endless recursion for some cases
		bool leftIndivisible = trianglesLeft.size() == triangles.size();
		bool rightIndivisible = trianglesRight.size() == triangles.size();
		if ((leftIndivisible && rightIndivisible) && (depth >= MIN_DEPTH))
		{
			// No further division of both branches possible
			return new KdNode(centerPoint, new KdNode(trianglesLeft, leftBox), new KdNode(trianglesRight, rightBox), bBox);
		}
		else if (leftIndivisible && (depth >= MIN_DEPTH))
		{
			// No further division of left branch possible
			return new KdNode(centerPoint, new KdNode(trianglesLeft, leftBox), build(trianglesRight, rightBox, depth + 1), bBox);
		}
		else if (rightIndivisible && (depth >= MIN_DEPTH))
		{
			// No further division of right branch possible
			return new KdNode(centerPoint, build(trianglesLeft, leftBox, depth + 1), new KdNode(trianglesRight, rightBox), bBox);
		}

		return new KdNode(centerPoint, build(trianglesLeft, leftBox, depth + 1), build(trianglesRight, rightBox, depth + 1), bBox);
	}

	bool KdNode::calculateIntersection(aiRay& ray, IntersectionInformation& outIntersection)
	{
		if (!this->boundingBox.intersects(ray))
		{
			// Ray does not intersect with this trees bounding box 
			// and therefore neither with a bounding box of a subtree
			return false;
		}

		bool leftHit{ false }, rightHit{ false };
		if (this->left && this->right) // If one of them is true, the other one is true too
		{
			// This node has subtrees
			leftHit = this->left->calculateIntersection(ray, outIntersection);
			rightHit = this->right->calculateIntersection(ray, outIntersection);
			return leftHit || rightHit;
		}

		bool intersects{ false };
		std::vector<aiVector3D*> nearestIntersectedTriangle;
		std::vector<aiVector3D*> triangleVertexNormals;
		aiVector3D intersectionPoint;
		aiVector2D uvCoordinates;

		if (!this->left && !this->right)
		{
			// This is a leaf node
			// Check triangles for intersection
			for (std::pair<aiFace*, aiMesh*> currentPair : this->containedTriangles)
			{
				aiFace* currentFace{ currentPair.first };
				aiMesh* associatedMesh{ currentPair.second };

				for (unsigned int currentIndex = 0; currentIndex < currentFace->mNumIndices; currentIndex++)
				{
					nearestIntersectedTriangle.push_back(&(associatedMesh->mVertices[currentFace->mIndices[currentIndex]]));
					triangleVertexNormals.push_back(&(associatedMesh->mNormals[currentFace->mIndices[currentIndex]]));
				}
				bool intersectsCurrentTriangle = rayTriangleIntersection(ray, nearestIntersectedTriangle, &intersectionPoint, &uvCoordinates);
				// We can immediately return if the cast ray is a shadow ray
				if (intersectsCurrentTriangle && (ray.type == RayType::SHADOW))
				{
					return true;
				}
				float distanceToIntersectionPoint = (intersectionPoint - ray.pos).Length();
				if (intersectsCurrentTriangle && (distanceToIntersectionPoint < outIntersection.intersectionDistance))
				{
					outIntersection.intersectionDistance = distanceToIntersectionPoint;
					outIntersection.hitMesh = associatedMesh;
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
			return intersects;
		}
		return leftHit || rightHit || intersects; // This case may not exist
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

} // end of namespace raytracer