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

	typedef struct Event
	{
		Event(float pos, Axis k, EventType t, KdTriangle& tri) :
			position(pos), dimension(k), type(t), triangle(tri)
		{};

		Event& operator= (const Event& event)
		{
			this->position = event.position;
			this->dimension = event.dimension;
			this->type = event.type;
			this->triangle = event.triangle;
			return *this;
		}

		float position;
		Axis dimension;
		EventType type;
		KdTriangle triangle;
	}Event;

	/*--------------------------------< Constants >-----------------------------------------*/

	class KdNode : public AccelerationStructure
	{

	static constexpr unsigned int MAX_TRIANGLES_PER_LEAF = 32;

	static constexpr unsigned int MAX_DEPTH = 512;
	
	static constexpr unsigned int MIN_DEPTH = 32;

	static constexpr float TRAVERSIAL_COST = 1.0f;

	static constexpr float INTERSECTION_COST = 5.25f;

	static constexpr uint8_t MAX_TREE_DIMENSION = 3;
	
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		KdNode() = default;

		KdNode(Plane& location, KdNode* leftChild, KdNode* rightChild, BoundingBox& box) :
			splittingPlane(location), left(leftChild), right(rightChild), boundingBox(box)
		{};

		KdNode(std::vector<KdTriangle>& triangles, BoundingBox& box) :
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

		static KdNode* buildTreeSAH(std::vector<KdTriangle>& triangles);
		
		virtual bool calculateIntersection(const aiRay& ray, IntersectionInformation* outIntersection) override;

	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:
		
		static KdNode* build(std::vector<KdTriangle>& triangles, BoundingBox& bBox, unsigned int depth = 1);

		static KdNode* buildSAH(
			std::vector<KdTriangle>& triangles,
			BoundingBox bBox,
			unsigned int depth = 0);

		static std::pair<float, ChildSide> SAH(
			const Plane& plane, 
			const BoundingBox& box,
			const int64_t triangleCountLeft,
			const int64_t triangleCountRight,
			const int64_t triangleCountOverlap);

		static float costHeuristic(
			const float probabilityLeft, 
			const float probabilityRight,
			const int64_t triangleCountLeft,
			const int64_t triangleCountRight);

		static std::pair<Plane, ChildSide> findPlane(
			std::vector<KdTriangle>& triangles,
			const BoundingBox& bBox,
			float* outCost);

		static void classifyTriangles(
			std::vector<KdTriangle>& triangles,
			std::pair<Plane, ChildSide>& splittingPlane,
			std::vector<KdTriangle>* outTrianglesLeft,
			std::vector<KdTriangle>* outTrianglesRight);

		static bool compareEvents(const Event& e1, const Event& e2);

		static bool terminate(const unsigned int triangleCount, const float minCost);

	/*--------------------------------< Public members >------------------------------------*/
	public:
	
	/*--------------------------------< Protected members >---------------------------------*/
	protected:
	
	/*--------------------------------< Private members >-----------------------------------*/
	private:

		// Bounding box of this tree node, containing the bounding box of the left as well 
		// as the bounding box of the right sub tree
		BoundingBox boundingBox;
		
		// The plane at which the BB of this node was split along
		Plane splittingPlane;

		// Left subtree
		KdNode* left;

		// Right subtree
		KdNode* right;
		
		// Triangles contained by a leaf
		std::vector<KdTriangle> containedTriangles;

	};
	
} // end of namespace raytracer