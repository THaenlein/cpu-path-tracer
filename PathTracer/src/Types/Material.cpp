/*
 * Material.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include "sdl2/SDL.h"

#include "Material.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	void Material::setDiffuseTexture(std::unique_ptr<raytracing::Texture> texture)
	{
		if (this->hasDiffuseTexture)
		{
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Material already has a diffuse texture. Overwriting...");
		}
		this->diffuseTexture = std::move(texture);
		this->hasDiffuseTexture = true;
	}

	void Material::resetDiffuseTexture()
	{
		if (this->hasDiffuseTexture)
		{
			this->diffuseTexture.reset();
			this->hasDiffuseTexture = false;
		}
	}
		
	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

	void Material::initialize(aiMaterial* material)
	{
		material->Get(AI_MATKEY_COLOR_DIFFUSE, this->colorDiffuse);
		material->Get(AI_MATKEY_COLOR_REFLECTIVE, this->colorReflective);
		material->Get(AI_MATKEY_COLOR_EMISSIVE, this->colorEmissive);
		material->Get(AI_MATKEY_COLOR_AMBIENT, this->colorAmbient);
		material->Get(AI_MATKEY_COLOR_SPECULAR, this->colorSpecular);
		material->Get(AI_MATKEY_COLOR_TRANSPARENT, this->colorTransparent);

		material->Get(AI_MATKEY_REFLECTIVITY, this->reflectivity);
		material->Get(AI_MATKEY_OPACITY, this->opacity);
		material->Get(AI_MATKEY_REFRACTI, this->refractionIndex);
		material->Get(AI_MATKEY_SHININESS, this->shininess);
		material->Get(AI_MATKEY_SHININESS_STRENGTH, this->shininessStrength);
		material->Get(AI_MATKEY_TRANSPARENCYFACTOR, this->transparency);

		material->Get(AI_MATKEY_SHADING_MODEL, this->shadingModel);
	}
	
} // end of namespace raytracer