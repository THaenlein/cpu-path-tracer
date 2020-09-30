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
		int i =      uv.x * width;
		int j = (1 - uv.y) * height - 0.001f;

		i = std::clamp(i, 0, width - 1);
		j = std::clamp(j, 0, height - 1);

		float red   = static_cast<float>(this->texture[3 * i + 3 * width * j    ]) / 255.f;
		float green = static_cast<float>(this->texture[3 * i + 3 * width * j + 1]) / 255.f;
		float blue  = static_cast<float>(this->texture[3 * i + 3 * width * j + 2]) / 255.f;

		return { red, green, blue };
	}
		
	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/
	
} // end of namespace raytracer