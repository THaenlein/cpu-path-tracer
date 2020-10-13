/*
 * <template>.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include "raytracing.hpp"

#include "assimp/camera.h"


namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class AccelerationStructure
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		virtual bool calculateIntersection(const aiRay& ray, IntersectionInformation* outIntersection) = 0;
	
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

	};
	
} // end of namespace raytracer