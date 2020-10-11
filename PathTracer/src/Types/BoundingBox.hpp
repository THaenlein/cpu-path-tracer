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

		bool operator==(const BoundingBox& other) const;

		void reset();

		void split(BoundingBox& left, BoundingBox& right);
		
		void split(BoundingBox& left, BoundingBox& right, uint8_t splitAxis);

		void split(BoundingBox& left, BoundingBox& right, const Plane& splitPlane) const;

		bool contains(std::vector<aiVector3D*> triangle) const;
		
		bool contains(std::pair<aiFace*, aiMesh*>& triangleMeshPair) const;
		
		bool contains(aiVector3D* point) const;
		
		bool exclusiveContains(std::vector<aiVector3D*> triangle) const;

		bool exclusiveContains(std::pair<aiFace*, aiMesh*>& triangleMeshPair) const;

		bool intersects(const aiRay& ray) const;

		inline const aiVector3D& getMin() const
		{
			return this->min;
		}

		inline const aiVector3D& getMax() const
		{
			return this->max;
		}

		inline const aiMesh* getContainedMesh() const
		{
			return this->containedMesh;
		}

		const Axis getLongestAxis() const
		{
			aiVector3D length = this->max - this->min;
			return length.x >= length.y ? (length.x >= length.z ? Axis::X : Axis::Z) : (length.y >= length.z ? Axis::Y : Axis::Z);
		}

		const float getSurfaceArea() const;
		
		aiVector3D getCenter() const;

		bool isPlanar(float epsilon = 1e-6) const;

		bool isPlanar(Axis axis, float epsilon = 1e-6) const;

		float length(const Axis axis) const;

		void clipToBox(const BoundingBox& box);

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