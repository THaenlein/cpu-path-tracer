/*
 * mathUtility.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>
#include <random>
#include <unordered_map>

#include "assimp/types.h"
#include "assimp/scene.h"

#include "raytracing.hpp"
#include "Types/Material.hpp"
#include "Textures/Texture.hpp"

namespace utility
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class materialUtility
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		static void createMaterialMapping(
			const std::string textureDirPath, 
			const aiScene* scene,
			std::unordered_map<aiMaterial*, std::unique_ptr<raytracing::Material>>* outTextureMapping);

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

	};
	
} // end of namespace raytracer