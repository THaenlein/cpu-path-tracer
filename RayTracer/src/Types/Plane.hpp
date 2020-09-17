/*
 * Plane.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/

#include "assimp\types.h"

#include "raytracing.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class Plane
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		Plane() :
			position(0.f), alignedAxis(Axis::NONE)
		{};

		Plane(float pos, Axis axis) :
			position(pos), alignedAxis(axis)
		{};

		inline Axis getAxis()
		{
			return this->alignedAxis;
		}

		inline float getPosition()
		{
			return this->position;
		}
	
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

		// The position of the plane on the aligned axis
		float position;
		
		// The axis the plane is aligned to
		Axis alignedAxis;

	};
	
} // end of namespace raytracer