/*
 * CheckerTexture.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <math.h>

#include "CheckerTexture.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	aiColor3D CheckerTexture::getColor(const float u, const float v) const
	{
		static const float scale{5.f};
		bool pattern = (fmod(v * scale, 1) < 0.5) ^ (fmod(u * scale, 1) < 0.5);
		return pattern ? even : odd;
	}
	
	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/
	
} // end of namespace raytracer