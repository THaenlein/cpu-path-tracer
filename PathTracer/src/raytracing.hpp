/*
 * raytracing.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>
#include <algorithm>

#include "assimp/types.h"
#include "assimp/mesh.h"
#include "assimp/material.h"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	// Force packing of struct to avoid padding bytes
	struct Uint24
	{
		Uint24() :
			color{0x00, 0x00, 0x00}
		{}

		// (value << 24) is cut off!
		Uint24(uint32_t value) :
			color{ 
			static_cast<uint8_t>(value << 16),
			static_cast<uint8_t>(value << 8 ), 
			static_cast<uint8_t>(value << 0 )}
		{}

		explicit Uint24(const aiColor3D& value)
		{
			std::vector<uint8_t> clampedValues = clamp(value);
			std::copy(clampedValues.begin(), clampedValues.end(), this->color);
		}

		explicit Uint24(const aiVector3D& value)
		{
			std::vector<uint8_t> clampedValues = clamp(value);
			std::copy(clampedValues.begin(), clampedValues.end(), this->color);
		}

		Uint24& operator=(const aiColor3D& value)
		{
			std::vector<uint8_t> clampedValues = clamp(value);
			std::copy(clampedValues.begin(), clampedValues.end(), this->color);
			return *this;
		}

		Uint24& operator=(const aiVector3D& value)
		{
			std::vector<uint8_t> clampedValues = clamp(value);
			std::copy(clampedValues.begin(), clampedValues.end(), this->color);
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

		uint8_t color[3];
	};

	struct IntersectionInformation
	{
		IntersectionInformation() :
			hitMesh(nullptr),
			hitPoint(),
			hitTriangle(),
			vertexNormals(),
			textureCoordinates(),
			intersectionDistance((std::numeric_limits<float>::max)()), // Paranthesize to prevent call of macro "max"
			uv(),
			uvTextureCoords(),
			index(0),
			ray()
		{}
		
		const aiMesh* hitMesh;
		aiVector3D hitPoint;
		std::vector<aiVector3D*> hitTriangle;
		std::vector<aiVector3D*> vertexNormals;
		std::vector<aiVector3D*> textureCoordinates;
		float intersectionDistance;
		aiVector2D uv;
		aiVector3D uvTextureCoords;
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

	typedef enum EventType : uint8_t
	{
		END = 0,
		PLANAR = 1,
		START = 2
	}EventType;

	typedef struct KdTriangle
	{
		KdTriangle(std::pair<aiFace*, aiMesh*> pair, ChildSide childSide):
			faceMeshPair(pair), side(childSide)
		{}

		std::pair<aiFace*, aiMesh*> faceMeshPair;
		ChildSide side;
	}KdTriangle;

	/*--------------------------------< Constants >-----------------------------------------*/

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