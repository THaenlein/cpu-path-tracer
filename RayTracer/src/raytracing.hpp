/*
 * raytracing.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>
#include <algorithm>

#include "assimp\types.h"
#include "assimp\mesh.h"
#include "assimp\material.h"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

#define ANTI_ALIASING 1
#define MULTI_THREADING 1
#define USE_ACCELERATION_STRUCTURE 1
#define PATH_TRACE 1

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

		std::vector<uint8_t> clamp(const aiColor3D& value)
		{
			uint8_t red = static_cast<uint8_t>(std::clamp(value.r, 0.f, 1.f) * 255U);
			uint8_t green = static_cast<uint8_t>(std::clamp(value.g, 0.f, 1.f) * 255U);
			uint8_t blue = static_cast<uint8_t>(std::clamp(value.b, 0.f, 1.f) * 255U);
			return { red, green, blue };
		}

		std::vector<uint8_t> clamp(const aiVector3D& value)
		{
			uint8_t red = static_cast<uint8_t>(std::clamp(value.x, 0.f, 1.f) * 255U);
			uint8_t green = static_cast<uint8_t>(std::clamp(value.y, 0.f, 1.f) * 255U);
			uint8_t blue = static_cast<uint8_t>(std::clamp(value.z, 0.f, 1.f) * 255U);
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
			vertexNormals(),
			intersectionDistance((std::numeric_limits<float>::max)()), // Paranthesize to prevent call of macro "max"
			uv(),
			index(0),
			ray()
		{}
		
		const aiMesh* hitMesh;

		aiVector3D hitPoint;

		std::vector<aiVector3D*> hitTriangle;
		
		std::vector<aiVector3D*> vertexNormals;

		float intersectionDistance;

		aiVector2D uv;

		uint32_t index;

		aiRay ray;
	};

	typedef enum Axis : int8_t
	{
		NONE = -1,
		X = 0,
		Y = 1,
		Z = 2
	}Axis;

	typedef enum ChildSide
	{
		LEFT,
		RIGHT,
		BOTH,
		UNDEFINED
	}ChildSide;

	typedef struct KdTriangle
	{
		KdTriangle(std::pair<aiFace*, aiMesh*> pair, ChildSide childSide):
			faceMeshPair(pair), side(childSide)
		{}

		std::pair<aiFace*, aiMesh*> faceMeshPair;
		ChildSide side;
	}KdTriangle;

	/*--------------------------------< Constants >-----------------------------------------*/

	static const struct RenderSettings
	{
		// Defines the anti aliasing resolution. This number splits every pixel into n^2 subpixel.
		// Increasing this number smoothes edges of the rendered image, but increases render time.
		static const unsigned int antiAliasingResolution = 4;
		static constexpr float bias = 0.001f;
		static const uint8_t maxRayDepth = 8;
		static const uint8_t maxBounces = 10;
		static const uint16_t maxSamples = 1;
	}renderSettings;

	/*--------------------------------< Public methods >------------------------------------*/

	static const char* getShadingModelString(int enumerator)
	{
		static const char* strings[] = {"Undefined", "Flat", "Gouraud", "Phong", "Blinn", "Toon", "Oren-Nayar", "Minnaert", "Cook-Torrance", "Shadeless", "Fresnel"};
		return strings[enumerator];
	}
	
	/*--------------------------------< Protected methods >---------------------------------*/
	
	/*--------------------------------< Private methods >-----------------------------------*/
	
	/*--------------------------------< Public members >------------------------------------*/
	
	/*--------------------------------< Protected members >---------------------------------*/
	
	/*--------------------------------< Private members >-----------------------------------*/

	
} // end of namespace raytracing