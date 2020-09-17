/*
 * KdNode.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/

#include <vector>

#include "assimp/types.h"
#include "assimp/mesh.h"

#include "raytracing.hpp"
#include "AccelerationStructure.hpp"
#include "BoundingBox.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/
	
	static const unsigned int MAX_TRIANGLES_PER_LEAF = 32;

	static const unsigned int MAX_DEPTH = 512;
	
	static const unsigned int MIN_DEPTH = 32;


	class KdNode : public AccelerationStructure
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		KdNode() = default;

		KdNode(Plane location, KdNode* leftChild, KdNode* rightChild, BoundingBox box) :
			splittingPlane(location), left(leftChild), right(rightChild), boundingBox(box)
		{};

		KdNode(std::vector<KdTriangle>& triangles, BoundingBox box) :
			containedTriangles(triangles), boundingBox(box), left(nullptr), right(nullptr)
		{};

		~KdNode()
		{
			if (this->left && this->right)
			{
				this->left->~KdNode();
				this->right->~KdNode();
			}
			delete left;
			delete right;
		}

		static KdNode* buildTree(std::vector<KdTriangle>& triangles);

		static KdNode* build(std::vector<KdTriangle>& triangles, BoundingBox bBox, unsigned int depth = 1);


		virtual bool calculateIntersection(aiRay& ray, IntersectionInformation& outIntersection) override;

	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:

	/*--------------------------------< Public members >------------------------------------*/
	public:
	
	/*--------------------------------< Protected members >---------------------------------*/
	protected:
	
	/*--------------------------------< Private members >-----------------------------------*/
	private:

		// Bounding box of this tree node, containing the bounding box of the left as well 
		// as the bounding box of the right sub tree
		BoundingBox boundingBox;
		
		// The point at which the BB of this node was split along the axis = depth % 3
		Plane splittingPlane;

		// Left subtree
		KdNode* left;

		// Right subtree
		KdNode* right;
		
		// Triangles contained by a leaf
		std::vector<KdTriangle> containedTriangles;

	};
	
} // end of namespace raytracer