/*
 * BoundingBox.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <memory>
#include <vector>

#include "assimp\types.h"
#include "assimp\mesh.h"

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

		BoundingBox(aiVector3D& min, aiVector3D& max) :
			min{ min },
			max{ max }
		{};

		BoundingBox(const BoundingBox& other):
			min{other.min},
			max{other.max}
		{};

		void reset();

		//void setDimensions(aiFace* triangle);

		void split(BoundingBox& left, BoundingBox& right);

		bool contains(std::vector<aiVector3D*> triangle);

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

		bool operator==(const BoundingBox& other) const;
	
	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:

		bool contains(aiVector3D* point);

		aiVector3D getCenter();

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