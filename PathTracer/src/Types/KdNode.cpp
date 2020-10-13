/*
 * KdNode.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <algorithm>
#include <iostream>

#include "KdNode.hpp"
#include "Utility/mathUtility.hpp"
#include "exceptions.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	using namespace utility;

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	/*static*/ KdNode* KdNode::buildTree(std::vector<KdTriangle>& triangles)
	{
		// Create BBox of full scene
		BoundingBox rootBox(triangles);

		// Start recursive build of tree
		return build(triangles, rootBox);
	}

	/*static*/ KdNode* KdNode::buildTreeSAH(std::vector<KdTriangle>& triangles)
	{
		// Create BBox of full scene
		BoundingBox rootBox(triangles);

		// Start recursive build of tree using surface area heuristic
		return buildSAH(triangles, rootBox);
	}

	bool KdNode::calculateIntersection(const aiRay& ray, IntersectionInformation* outIntersection)
	{
		if (!this->boundingBox.intersects(ray))
		{
			// Ray does not intersect with this trees bounding box 
			// and therefore neither with a bounding box of a subtree
			return false;
		}

		bool leftHit{ false }, rightHit{ false };
		// If a node is no leaf node, it always has two subtrees
		if (this->left && this->right)
		{
			// This node has subtrees
			leftHit = this->left->calculateIntersection(ray, outIntersection);
			rightHit = this->right->calculateIntersection(ray, outIntersection);
			return leftHit || rightHit;
		}

		bool intersects{ false };
		std::vector<aiVector3D*> nearestIntersectedTriangle;
		std::vector<aiVector3D*> triangleVertexNormals;
		std::vector<aiVector3D*> textureCoordinates;
		aiVector3D intersectionPoint;
		aiVector2D uvCoordinates;

		if (!this->left && !this->right)
		{
			// This is a leaf node
			// Check triangles for intersection
			for (KdTriangle& currentPair : this->containedTriangles)
			{
				aiFace* currentFace{ currentPair.faceMeshPair.first };
				aiMesh* associatedMesh{ currentPair.faceMeshPair.second };

				for (unsigned int currentIndex = 0; currentIndex < currentFace->mNumIndices; currentIndex++)
				{
					nearestIntersectedTriangle.push_back(&(associatedMesh->mVertices[currentFace->mIndices[currentIndex]]));
					triangleVertexNormals.push_back(&(associatedMesh->mNormals[currentFace->mIndices[currentIndex]]));
					// Always use first texture channel
					textureCoordinates.push_back(&(associatedMesh->mTextureCoords[0][currentFace->mIndices[currentIndex]]));
				}
				bool intersectsCurrentTriangle = mathUtility::rayTriangleIntersection(ray, nearestIntersectedTriangle, &intersectionPoint, &uvCoordinates);
				// We can immediately return if the cast ray is a shadow ray
				if (intersectsCurrentTriangle && (ray.type == RayType::SHADOW))
				{
					return true;
				}
				float distanceToIntersectionPoint = (intersectionPoint - ray.pos).Length();
				if (intersectsCurrentTriangle && (distanceToIntersectionPoint < outIntersection->intersectionDistance))
				{
					outIntersection->intersectionDistance = distanceToIntersectionPoint;
					outIntersection->hitMesh = associatedMesh;
					outIntersection->hitTriangle = nearestIntersectedTriangle;
					outIntersection->hitPoint = intersectionPoint;
					outIntersection->ray = ray;
					outIntersection->uv = uvCoordinates;
					// Always use first texture channel
					if (associatedMesh->HasTextureCoords(0))
					{
						outIntersection->uvTextureCoords =
							(1 - uvCoordinates.x - uvCoordinates.y) * 
							*(textureCoordinates[0]) + uvCoordinates.x *
							*(textureCoordinates[1]) + uvCoordinates.y *
							*(textureCoordinates[2]);
					}
					outIntersection->vertexNormals = triangleVertexNormals;
					outIntersection->textureCoordinates = textureCoordinates;
					intersects = true;
				}
				nearestIntersectedTriangle.clear();
				triangleVertexNormals.clear();
				textureCoordinates.clear();
			}
			return intersects;
		}
		return leftHit || rightHit || intersects; // This case may not exist
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

	/*static*/ KdNode* KdNode::build(std::vector<KdTriangle>& triangles, BoundingBox& bBox, unsigned int depth/* = 1*/)
	{
		const ai_real EPSILON = 1e-3f;

		if ((triangles.size() <= MAX_TRIANGLES_PER_LEAF) || (depth == MAX_DEPTH))
		{
			// Return leaf node
			return new KdNode(triangles, bBox);
		}

		// Find plane to split
		Axis longestAxis = bBox.getLongestAxis();

		// Sort triangles
		auto compareTriangles = [longestAxis](const KdTriangle& t1, const KdTriangle& t2) -> bool
		{
			aiFace* t1Face = t1.faceMeshPair.first;
			aiMesh* t1Mesh = t1.faceMeshPair.second;
			aiFace* t2Face = t2.faceMeshPair.first;
			aiMesh* t2Mesh = t2.faceMeshPair.second;
			return
				((t1Mesh->mVertices[t1Face->mIndices[0]][longestAxis] +
					t1Mesh->mVertices[t1Face->mIndices[1]][longestAxis] +
					t1Mesh->mVertices[t1Face->mIndices[2]][longestAxis]) / 3.f) <
					((t2Mesh->mVertices[t2Face->mIndices[0]][longestAxis] +
						t2Mesh->mVertices[t2Face->mIndices[1]][longestAxis] +
						t2Mesh->mVertices[t2Face->mIndices[2]][longestAxis]) / 3.f);
		};
		sort(triangles.begin(), triangles.end(), compareTriangles);

		// Find median to split
		const float WEIGHT = 0.1f;
		BoundingBox medianTriBox{ triangles[triangles.size() / 2].faceMeshPair };
		aiVector3D center = bBox.getCenter();
		aiVector3D medianCenter = medianTriBox.getCenter();
		float planePosition = center[longestAxis] - ((center[longestAxis] - medianCenter[longestAxis]) * WEIGHT);
		Plane splittingPlane(planePosition, longestAxis);

		// Split box in longest axis
		BoundingBox leftBox;
		BoundingBox rightBox;
		bBox.split(leftBox, rightBox, splittingPlane);

		// Add left and overlapping tris to left
		// Add right and overlapping tris to right
		std::vector<KdTriangle> trianglesLeft;
		std::vector<KdTriangle> trianglesRight;

		for (KdTriangle& triangle : triangles)
		{
			BoundingBox triangleBounds{ triangle };

			if (triangleBounds.getMin()[longestAxis] < (splittingPlane.getPosition() + EPSILON))
			{
				trianglesLeft.push_back(triangle);
			}
			if (triangleBounds.getMax()[longestAxis] >= (splittingPlane.getPosition() - EPSILON))
			{
				trianglesRight.push_back(triangle);
			}
		}

		// To avoid endless recursion for some cases
		bool leftIndivisible = trianglesLeft.size() == triangles.size();
		bool rightIndivisible = trianglesRight.size() == triangles.size();
		if ((leftIndivisible && rightIndivisible) && (depth >= MIN_DEPTH))
		{
			// No further division of both branches possible
			return new KdNode(splittingPlane, new KdNode(trianglesLeft, leftBox), new KdNode(trianglesRight, rightBox), bBox);
		}
		else if (leftIndivisible && (depth >= MIN_DEPTH))
		{
			// No further division of left branch possible
			return new KdNode(splittingPlane, new KdNode(trianglesLeft, leftBox), build(trianglesRight, rightBox, depth + 1), bBox);
		}
		else if (rightIndivisible && (depth >= MIN_DEPTH))
		{
			// No further division of right branch possible
			return new KdNode(splittingPlane, build(trianglesLeft, leftBox, depth + 1), new KdNode(trianglesRight, rightBox), bBox);
		}

		return new KdNode(splittingPlane, build(trianglesLeft, leftBox, depth + 1), build(trianglesRight, rightBox, depth + 1), bBox);
	}

	/*static*/ KdNode* KdNode::buildSAH(
		std::vector<KdTriangle>& triangles,
		BoundingBox bBox,
		unsigned int depth/* = 0*/)
	{
		// Find plane to split
		float minCost;
		std::pair<Plane, ChildSide> bestPlane = findPlane(triangles, bBox, &minCost);

		if ((terminate(triangles.size(), minCost)))
		{
			return new KdNode(triangles, bBox);
		}

		// Split root box
		BoundingBox leftBox;
		BoundingBox rightBox;
		bBox.split(leftBox, rightBox, bestPlane.first);

		// Classify triangles corresponding to splitting plane
		std::vector<KdTriangle> trianglesLeft;
		std::vector<KdTriangle> trianglesRight;
		classifyTriangles(triangles, bestPlane, &trianglesLeft, &trianglesRight);
		triangles.~vector();

		return new KdNode(
			bestPlane.first, 
			buildSAH(trianglesLeft, leftBox, depth + 1),
			buildSAH(trianglesRight, rightBox, depth + 1),
			bBox);
	}

	std::pair<float, ChildSide> KdNode::SAH(
		const Plane& plane,
		const BoundingBox& box,
		const int64_t triangleCountLeft,
		const int64_t triangleCountRight,
		const int64_t triangleCountOverlap)
	{
		BoundingBox leftBox;
		BoundingBox rightBox;
		box.split(leftBox, rightBox, plane);
		if (leftBox.isPlanar() || rightBox.isPlanar())
		{
			return std::make_pair<float, ChildSide>(std::numeric_limits<float>::infinity(), ChildSide::UNDEFINED);
		}
		const float probabilityLeft = leftBox.getSurfaceArea() / box.getSurfaceArea();
		const float probabilityRight = rightBox.getSurfaceArea() / box.getSurfaceArea();
		const float costLeft = costHeuristic(probabilityLeft, probabilityRight, triangleCountLeft + triangleCountOverlap, triangleCountRight);
		const float costRight = costHeuristic(probabilityLeft, probabilityRight, triangleCountLeft, triangleCountRight + triangleCountOverlap);
		return std::make_pair<float, ChildSide>(costLeft < costRight ? costLeft, ChildSide::LEFT : costRight, ChildSide::RIGHT);
	}

	float KdNode::costHeuristic(
		const float probabilityLeft,
		const float probabilityRight,
		const int64_t triangleCountLeft,
		const int64_t triangleCountRight)
	{
		auto lambdaP = [&] () -> float
		{
			return (triangleCountLeft == 0) || (triangleCountRight == 0) ? 0.8f : 1.f;
		};

		return 
			lambdaP() *
			(TRAVERSIAL_COST + INTERSECTION_COST *
			((probabilityLeft * triangleCountLeft) + (probabilityRight * triangleCountRight)));
	}

	std::pair<Plane, ChildSide> KdNode::findPlane(
		std::vector<KdTriangle>& triangles,
		const BoundingBox& bBox,
		float* outCost)
	{
		*outCost = std::numeric_limits<float>::infinity();
		Plane bestPlane;
		ChildSide childSide{ ChildSide::UNDEFINED };

		for (unsigned int k = 0; k < MAX_TREE_DIMENSION; k++)
		{
			Axis axis = static_cast<Axis>(k);
			std::vector<Event> eventlist;
			for (KdTriangle& triangle : triangles)
			{
				BoundingBox triBox(triangle);
				triBox.clipToBox(bBox);
				if (triBox.isPlanar(axis))
				{
					eventlist.push_back({ triBox.getMin()[k], axis, EventType::PLANAR, triangle});
				}
				else
				{
					eventlist.push_back({ triBox.getMin()[k], axis, EventType::START, triangle });
					eventlist.push_back({ triBox.getMax()[k], axis, EventType::END, triangle });
				}
			}
			sort(eventlist.begin(), eventlist.end(), compareEvents);

			// iteratively "sweep" plane over all split candidates
			uint64_t triangleCountLeft = 0;
			uint64_t triangleCountOverlap = 0;
			uint64_t triangleCountRight = triangles.size(); // start with all tris on right

			for (unsigned int i = 0; i < eventlist.size(); i++)
			{
				Axis eventDimension = eventlist.at(i).dimension; // TODO: remove?
				Plane p(eventlist.at(i).position, eventDimension);
				int64_t start{ 0 };
				int64_t end{ 0 };
				int64_t inPlane{ 0 };

				while ((i < eventlist.size()) && (eventlist.at(i).position == p.getPosition()) && (eventlist.at(i).type == EventType::END))
				{
					end++;
					i++;
				}

				while ((i < eventlist.size()) && (eventlist.at(i).position == p.getPosition()) && (eventlist.at(i).type == EventType::PLANAR))
				{
					inPlane++;
					i++;
				}

				while ((i < eventlist.size()) && (eventlist.at(i).position == p.getPosition()) && (eventlist.at(i).type == EventType::START))
				{
					start++;
					i++;
				}

				// Now the next plane p is found with start, end and inPlane
				// move plane onto p
				triangleCountOverlap = inPlane;
				triangleCountRight -= inPlane;
				triangleCountRight -= end;
				std::pair<float, ChildSide> result = SAH(p, bBox, triangleCountLeft, triangleCountRight, triangleCountOverlap);
				if (result.first < *outCost)
				{
					*outCost = result.first;
					bestPlane = p;
					childSide = result.second;
				}
				triangleCountLeft += start;
				triangleCountLeft += inPlane;
				triangleCountOverlap = 0; // move plane over p
			}
		}

		return std::make_pair(bestPlane, childSide);
	}

	/*static*/ void KdNode::classifyTriangles(
		std::vector<KdTriangle>& triangles,
		std::pair<Plane, ChildSide>& splittingPlane,
		std::vector<KdTriangle>* outTrianglesLeft,
		std::vector<KdTriangle>* outTrianglesRight)
	{
		Plane& plane{ splittingPlane.first };
		ChildSide planeSide{ splittingPlane.second };

		for (KdTriangle& tri : triangles)
		{
			BoundingBox triBox(tri);
			if ((triBox.getMin()[plane.getAxis()] == plane.getPosition()) &&
				(triBox.getMax()[plane.getAxis()] == plane.getPosition()))
			{
				if (planeSide == ChildSide::LEFT)
				{
					outTrianglesLeft->push_back(tri);
				}
				else if (planeSide == ChildSide::RIGHT)
				{
					outTrianglesRight->push_back(tri);
				}
				else
				{
					// This case should not occur!
					throw AccStructure("Error classifying triangles");
				}
			}
			else
			{
				if (triBox.getMin()[plane.getAxis()] < plane.getPosition())
				{
					outTrianglesLeft->push_back(tri);
				}
				if (triBox.getMax()[plane.getAxis()] > plane.getPosition())
				{
					outTrianglesRight->push_back(tri);
				}
			}
		}
	}

	/*static*/ bool KdNode::compareEvents(const Event& e1, const Event& e2)
	{
		return
			// Primary condition: sort by position
			(e1.position < e2.position) ||
			// Secondary condition: sort by type
			((e1.position < e2.position) && (e1.type < e2.type));
	}

	/*static*/ bool KdNode::terminate(const unsigned int triangleCount, const float minCost)
	{
		return minCost > INTERSECTION_COST * static_cast<float>(triangleCount);
	}

} // end of namespace raytracer
