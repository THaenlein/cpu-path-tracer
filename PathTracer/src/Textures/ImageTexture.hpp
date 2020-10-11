/*
 * ImageTexture.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include "Texture.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class ImageTexture : public Texture
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		ImageTexture(uint8_t* data, int dataWidth, int dataHeight) :
			texture(data),
			width(dataWidth),
			height(dataHeight)
		{};

		~ImageTexture()
		{
			delete[] this->texture;
		}

		virtual aiColor3D getColor(const aiVector3D& uv) const override;
	
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

		uint8_t* texture;

		int width;
		
		int height;

	};
	
} // end of namespace raytracer