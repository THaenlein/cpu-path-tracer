/*
 * RayTracer.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>

#include "RayTracer.hpp"
#include "exceptions.hpp"


namespace raytracer
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	void RayTracer::render(Application& application)
	{
		uint16_t renderWidth = application.getRenderWidth();
		uint16_t renderHeight = application.getRenderHeight();

		// Using 2d-array of 24-bit integer to encode color
		// rrrrrrrr gggggggg bbbbbbbb
		Uint24* pixels = new Uint24[renderWidth * renderHeight];
		
		for (uint16_t x = 0; x < renderWidth; x++)
		{
			for (uint16_t y = 0; y < renderHeight; y++)
			{
				Uint24 mintGreen{ 0x99E8CA };
				uint32_t currentPixel = y * renderWidth + x;
				pixels[currentPixel] = mintGreen;
			}
		}

		// In case of an exception in presentRender(), there is a memory leak in pixels[]
		// TODO: Fix this
		application.presentRender(pixels);
		delete[] pixels;
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/


} // end of namespace raytracer