/*
 * ImageTexture.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include "ImageTexture.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	aiColor3D ImageTexture::getColor(const aiVector3D& uv) const
	{
		float x =      uv.x  * static_cast<float>(width);
		float y = (1 - uv.y) * static_cast<float>(height);

		x = std::clamp(x, 0.f, static_cast<float>(width)  - 1.f);
		y = std::clamp(y, 0.f, static_cast<float>(height) - 1.f);

		int uOffset = static_cast<int>(x);
		int vOffset = static_cast<int>(y);

		float red   = static_cast<float>(this->texture[3 * uOffset + 3 * width * vOffset    ]) / 255.f;
		float green = static_cast<float>(this->texture[3 * uOffset + 3 * width * vOffset + 1]) / 255.f;
		float blue  = static_cast<float>(this->texture[3 * uOffset + 3 * width * vOffset + 2]) / 255.f;

		return { red, green, blue };
	}
		
	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/
	
} // end of namespace raytracer