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

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class Settings
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		Settings(uint8_t aa = 4, uint8_t maxDepth = 10, uint8_t samples = 1, float offset = 0.001f) :
			antiAliasingResolution(aa), maxRayDepth(maxDepth), maxSamples(samples), bias(offset)
		{};

		inline uint8_t getAAResolution() const
		{
			return this->antiAliasingResolution;
		}

		inline uint8_t getMaxRayDepth() const
		{
			return this->maxRayDepth;
		}

		inline uint8_t getMaxSamples() const
		{
			return this->maxSamples;
		}

		inline float getBias() const
		{
			return this->bias;
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

		const uint16_t maxSamples;
		
		const float bias;
	};
	
} // end of namespace raytracer