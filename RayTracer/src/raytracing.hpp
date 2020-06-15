/*
 * raytracing.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>

#include "assimp\types.h"
#include "assimp\mesh.h"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	// Force packing of struct to avoid padding bytes
#pragma pack(push, 1)
	struct Uint24
	{
		Uint24() :
			color(0x000000)
		{}

		Uint24(int value) :
			color(value)
		{}

		Uint24(aiColor3D value)
		{
			uint8_t red = static_cast<uint8_t>(value.r * 255);
			uint8_t green = static_cast<uint8_t>(value.g * 255);
			uint8_t blue = static_cast<uint8_t>(value.b * 255);
			this->color = (red << 16) | (green << 8) | (blue << 0);
		}

		unsigned int color : 24;
	};
#pragma pack(pop)


	struct IntersectionInformation
	{
		IntersectionInformation() :
			hitMesh(nullptr),
			hitPoint(),
			hitTriangle(),
			intersectionDistance((std::numeric_limits<float>::max)()), // Paranthesize to prevent call of macro "max"
			uv(),
			index(0)
		{}
		
		aiMesh* hitMesh;

		aiVector3D hitPoint;

		std::vector<aiVector3D*> hitTriangle;

		float intersectionDistance;

		aiVector2D uv;

		uint32_t index;
	};

	/*--------------------------------< Constants >-----------------------------------------*/

	/*--------------------------------< Public methods >------------------------------------*/
	
	/*--------------------------------< Protected methods >---------------------------------*/
	
	/*--------------------------------< Private methods >-----------------------------------*/
	
	/*--------------------------------< Public members >------------------------------------*/
	
	/*--------------------------------< Protected members >---------------------------------*/
	
	/*--------------------------------< Private members >-----------------------------------*/

	
} // end of namespace raytracing