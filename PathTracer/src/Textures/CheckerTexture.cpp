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

	aiColor3D CheckerTexture::getColor(const aiVector3D& uv) const
	{
		bool pattern = 
			(fmod(uv.y * this->textureScale, 1) < 0.5) ^ 
			(fmod(uv.x * this->textureScale, 1) < 0.5);
		return pattern ? even : odd;
	}
	
	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/
	
} // end of namespace raytracer