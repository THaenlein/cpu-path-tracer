/*
 * raytracing.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>

#include "assimp\types.h"
#include "assimp\mesh.h"

#include "FCollada.h"
#include "FCDocument\FCDLight.h"
#include "FCDocument\FCDEffectStandard.h"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

#define ANTI_ALIASING 0
#define MULTI_THREADING 1

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
			uint8_t red = static_cast<uint8_t>(value.r * 255U);
			uint8_t green = static_cast<uint8_t>(value.g * 255U);
			uint8_t blue = static_cast<uint8_t>(value.b * 255U);
			this->color = (red << 16) | (green << 8) | (blue << 0);
		}

		Uint24(aiVector3D value)
		{
			uint8_t red = static_cast<uint8_t>(value.x * 255U);
			uint8_t green = static_cast<uint8_t>(value.y * 255U);
			uint8_t blue = static_cast<uint8_t>(value.z * 255U);
			this->color = (red << 16) | (green << 8) | (blue << 0);
		}

		// Adding up colors in Uint24 should not be done, because risk of overflow!
		Uint24& operator+=(const Uint24& other)
		{
			unsigned int newColor{ this->color + other.color};
			this->color = newColor;
			return *this;
		}

		// Adding up colors in Uint24 should not be done, because risk of overflow!
		friend Uint24& operator+(Uint24 left, const Uint24& right)
		{
			left += right;
			return left;
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


	static const struct RenderSettings
	{
		// Defines the anti aliasing resolution. This number splits every pixel into n^2 subpixel.
		// Increasing this number smoothes edges of the rendered image, but increases render time.
		static const unsigned int antiAliasingResolution = 4;
	}renderSettings;

	/*--------------------------------< Constants >-----------------------------------------*/

	/*--------------------------------< Public methods >------------------------------------*/

	// TODO: Fix cyclic dependency
	inline const char* getEnumText(FCDLight::LightType type)
	{
		static const char* lightTypeStrings[] = { "Point", "Spot", "Ambient", "Directional" };
		return lightTypeStrings[type];
	}

	inline const char* getEnumText(FCDEffectStandard::LightingType type)
	{
		static const char* lightingTypeStrings[] = { "Constant", "Lambert", "Phong", "Blinn", "Unknown" };
		return lightingTypeStrings[type];
	}
	
	/*--------------------------------< Protected methods >---------------------------------*/
	
	/*--------------------------------< Private methods >-----------------------------------*/
	
	/*--------------------------------< Public members >------------------------------------*/
	
	/*--------------------------------< Protected members >---------------------------------*/
	
	/*--------------------------------< Private members >-----------------------------------*/

	
} // end of namespace raytracing