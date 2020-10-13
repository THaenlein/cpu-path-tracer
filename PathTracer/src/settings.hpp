/*
 * settings.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/

#include <cstdint>

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

#define USE_ACCELERATION_STRUCTURE 1
#define PATH_TRACE 1

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class Settings
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		Settings(uint16_t x, uint16_t y, uint8_t samples = 8, uint8_t maxDepth = 3, float offset = 0.001f, const float aperture = 0.f, const float fDist = 0.f, const bool dof = false, const bool aa = false) :
			width(x), height(y), maxSamples(samples), maxRayDepth(maxDepth), bias(offset), apertureRadius(aperture), focalDistance(fDist), useDOF(dof), useAA(aa)
		{};

		inline uint8_t getMaxSamples() const
		{
			return this->maxSamples;
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

		inline float getUseDOF() const
		{
			return this->useDOF;
		}

		inline float getUseAA() const
		{
			return this->useAA;
		}

		inline uint16_t getWidth() const
		{
			return this->width;
		}

		inline uint16_t getHeight() const
		{
			return this->height;
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
		const uint8_t maxSamples;

		const uint8_t maxRayDepth;
		
		const float bias;

		const float apertureRadius;

		const float focalDistance;

		const bool useDOF;

		const bool useAA;

		const uint16_t width;

		const uint16_t height;
	};
	
} // end of namespace raytracer