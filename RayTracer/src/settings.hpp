/*
 * settings.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/

#include <cstdint>

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

#define ANTI_ALIASING 1
#define MULTI_THREADING 1
#define USE_ACCELERATION_STRUCTURE 1
#define PATH_TRACE 1

	// Depth of field requires anti aliasing
#define DEPTH_OF_FIELD 1

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class Settings
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		Settings(uint8_t aa = 8, uint8_t maxDepth = 3, float offset = 0.001f, const float aperture = 0.f, const float fDist = 0.f) :
			antiAliasingResolution(aa), maxRayDepth(maxDepth), bias(offset), apertureRadius(aperture), focalDistance(fDist)
		{};

		inline uint8_t getAAResolution() const
		{
			return this->antiAliasingResolution;
		}

		inline uint8_t getMaxRayDepth() const
		{
			return this->maxRayDepth;
		}

		inline float getBias() const
		{
			return this->bias;
		}

		inline float getAperture() const
		{
			return this->apertureRadius;
		}
		
		inline float getFocalDistance() const
		{
			return this->focalDistance;
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

		// Defines the anti aliasing resolution. This number splits every pixel into n^2 subpixel.
		// Increasing this number smoothes edges of the rendered image, but increases render time.
		const uint8_t antiAliasingResolution;

		const uint8_t maxRayDepth;
		
		const float bias;

		const float apertureRadius;

		const float focalDistance;
	};
	
} // end of namespace raytracer