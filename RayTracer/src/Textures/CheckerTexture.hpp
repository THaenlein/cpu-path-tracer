/*
 * CheckerTexture.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include "assimp\types.h"

#include "Textures\Texture.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class CheckerTexture : public Texture
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:
		CheckerTexture(aiColor3D colorOdd, aiColor3D colorEven):
			odd(colorOdd), even(colorEven)
		{}

		virtual aiColor3D getColor(const float u, const float v) const override;
	
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

		aiColor3D odd;

		aiColor3D even;

	};
	
} // end of namespace raytracer