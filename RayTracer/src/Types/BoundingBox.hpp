/*
 * BoundingBox.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <memory>
#include <vector>

#include "assimp\types.h"
#include "assimp\mesh.h"

#include "raytracing.hpp"
#include "Types\Plane.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class BoundingBox
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		BoundingBox() :
			min{ std::numeric_limits<float>::max() },
			max{ -std::numeric_limits<float>::max() }
		{};

		BoundingBox(aiMesh* mesh);

		BoundingBox(std::vector<KdTriangle>& triangles);
		
		BoundingBox(const KdTriangle& triangles);

		BoundingBox(std::pair<aiFace*, aiMesh*>& triangle);

		BoundingBox(aiVector3D& min, aiVector3D& max) :
			min{ min },
			max{ max }
		{};

		BoundingBox(const BoundingBox& other):
			min{other.min},
			max{other.max}
		{};

		void reset();

		void split(BoundingBox& left, BoundingBox& right);
		
		void split(BoundingBox& left, BoundingBox& right, uint8_t splitAxis);

		void split(BoundingBox& left, BoundingBox& right, Plane& splitPlane);

		bool contains(std::vector<aiVector3D*> triangle);
		
		bool contains(std::pair<aiFace*, aiMesh*>& triangleMeshPair);
		
		bool contains(aiVector3D* point);
		
		bool exclusiveContains(std::vector<aiVector3D*> triangle);

		bool exclusiveContains(std::pair<aiFace*, aiMesh*>& triangleMeshPair);

		bool intersects(aiRay& ray);

		inline const aiVector3D& getMin()
		{
			return this->min;
		}

		inline const aiVector3D& getMax()
		{
			return this->max;
		}

		inline const aiMesh* getContainedMesh()
		{
			return this->containedMesh;
		}

		const Axis getLongestAxis()
		{
			aiVector3D length = this->max - this->min;
			return length.x >= length.y ? (length.x >= length.z ? Axis::X : Axis::Z) : (length.y >= length.z ? Axis::Y : Axis::Z);
		}

		const float getSurfaceArea();
		
		aiVector3D getCenter();

		bool isPlanar(float epsilon = 1e-6);

		float length(const Axis axis);

		bool operator==(const BoundingBox& other) const;
	
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

		aiVector3D min;

		aiVector3D max;

		aiMesh* containedMesh;

	};
	
} // end of namespace raytracer