/*
 * raytracing.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>
#include <algorithm>

#include "assimp\types.h"
#include "assimp\mesh.h"

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

		explicit Uint24(const aiColor3D& value)
		{
			std::vector<uint8_t> clampedValues = clamp(value);
			this->color = (clampedValues[0] << 16) | (clampedValues[1] << 8) | (clampedValues[2] << 0);
		}

		explicit Uint24(const aiVector3D& value)
		{
			std::vector<uint8_t> clampedValues = clamp(value);
			this->color = (clampedValues[0] << 16) | (clampedValues[1] << 8) | (clampedValues[2] << 0);
		}

		Uint24& operator=(const aiColor3D& value)
		{
			std::vector<uint8_t> clampedValues = clamp(value);
			this->color = (clampedValues[0] << 16) | (clampedValues[1] << 8) | (clampedValues[2] << 0);
			return *this;
		}

		Uint24& operator=(const aiVector3D& value)
		{
			std::vector<uint8_t> clampedValues = clamp(value);
			this->color = (clampedValues[0] << 16) | (clampedValues[1] << 8) | (clampedValues[2] << 0);
			return *this;
		}

		// TODO: This won't work. Fix this
		//Uint24& operator+=(const Uint24& other)
		//{
		//	unsigned int newColor{ this->color + other.color};
		//	this->color = newColor;
		//	return *this;
		//}

		// TODO: This won't work. Fix this
		//friend Uint24& operator+(Uint24& left, const Uint24& right)
		//{
		//	left += right;
		//	return left;
		//}

		std::vector<uint8_t> clamp(const aiColor3D& color)
		{
			uint8_t red = static_cast<uint8_t>(std::clamp(color.r, 0.f, 1.f) * 255U);
			uint8_t green = static_cast<uint8_t>(std::clamp(color.g, 0.f, 1.f) * 255U);
			uint8_t blue = static_cast<uint8_t>(std::clamp(color.b, 0.f, 1.f) * 255U);
			return { red, green, blue };
		}

		std::vector<uint8_t> clamp(const aiVector3D& color)
		{
			uint8_t red = static_cast<uint8_t>(std::clamp(color.x, 0.f, 1.f) * 255U);
			uint8_t green = static_cast<uint8_t>(std::clamp(color.y, 0.f, 1.f) * 255U);
			uint8_t blue = static_cast<uint8_t>(std::clamp(color.z, 0.f, 1.f) * 255U);
			return { red, green, blue };
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

	static const struct RenderSettings
	{
		// Defines the anti aliasing resolution. This number splits every pixel into n^2 subpixel.
		// Increasing this number smoothes edges of the rendered image, but increases render time.
		static const unsigned int antiAliasingResolution = 4;
		static constexpr float bias = 0.001f;
	}renderSettings;

	/*--------------------------------< Public methods >------------------------------------*/
	
	/*--------------------------------< Protected methods >---------------------------------*/
	
	/*--------------------------------< Private methods >-----------------------------------*/
	
	/*--------------------------------< Public members >------------------------------------*/
	
	/*--------------------------------< Protected members >---------------------------------*/
	
	/*--------------------------------< Private members >-----------------------------------*/

	
} // end of namespace raytracing