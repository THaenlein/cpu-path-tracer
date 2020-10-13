/*
 * CheckerTexture.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include "assimp/types.h"

#include "Textures/Texture.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class CheckerTexture : public Texture
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:
		CheckerTexture(aiColor3D colorOdd, aiColor3D colorEven, const float scale = 5.f):
			odd(colorOdd), even(colorEven), textureScale(scale)
		{}

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

		aiColor3D odd;

		aiColor3D even;

		const float textureScale;

	};
	
} // end of namespace raytracer