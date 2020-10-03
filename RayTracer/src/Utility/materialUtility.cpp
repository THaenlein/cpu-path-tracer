/*
 * materialUtility.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <experimental\filesystem>

#include "SDL.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_DEFINE
#include "stb.h"

#include "materialUtility.hpp"
#include "exceptions.hpp"
#include "Textures\ImageTexture.hpp"

namespace filesystem = std::experimental::filesystem;


namespace utility
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	using namespace raytracing;

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	void materialUtility::createMaterialMapping(
		const std::string textureDirPath,
		const aiScene* scene,
		std::unordered_map<aiMaterial*, std::unique_ptr<Material>>* outTextureMapping)
	{
		if (!outTextureMapping)
		{
			throw Utility("TextureMapping was null pointer!");
		}

		// Iterate through materials and load texture if it has one
		for (unsigned int currentMaterial = 0; currentMaterial < scene->mNumMaterials; currentMaterial++)
		{
			aiMaterial* material = scene->mMaterials[currentMaterial];
			std::unique_ptr<Material> newMaterial(new Material(material));
			unsigned int diffuseTextureCount = material->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE);
			if (diffuseTextureCount > 0)
			{
				aiString diffuseTexturePath;
				aiTextureMapping diffuseTextureMapping;
				if (material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexturePath, &diffuseTextureMapping) == aiReturn_SUCCESS)
				{
					if (diffuseTextureMapping != aiTextureMapping::aiTextureMapping_UV)
					{
						// Ignore anything other than UV-Mapping
						continue;
					}

					int width, height, components;
					filesystem::path texturePath = textureDirPath;
					texturePath /= diffuseTexturePath.C_Str();

					// Load image from file
					uint8_t* image = stbi_load(texturePath.string().c_str(), &width, &height, &components, 0);

					if (!image)
					{
						SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Tried to load image from file: %s", texturePath.string().c_str());
						throw Utility("Error on image texture load!");
					}
					else
					{
						SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Image texture successfully loaded: %s", texturePath.string().c_str());
					}

					// Add texture to material
					std::unique_ptr<ImageTexture> diffuseTexture(new ImageTexture(image, width, height));
					newMaterial->setDiffuseTexture(std::move(diffuseTexture));
				}
			}
			(*outTextureMapping)[material] = std::move(newMaterial);
		}
	}
		
	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/
	
} // end of namespace raytracer