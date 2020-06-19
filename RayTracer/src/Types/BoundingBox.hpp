/*
 * BoundingBox.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/

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

		BoundingBox() = default;

		BoundingBox(aiVector3D& min, aiVector3D& max):
			min(min), max(max) 
		{};

		void split(BoundingBox& left, BoundingBox& right);

		bool intersects(aiRay& ray);

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

	};
	
} // end of namespace raytracer