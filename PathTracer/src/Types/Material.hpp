/*
 * Material.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <memory>

#include "assimp\types.h"
#include "assimp\material.h"

#include "Textures\Texture.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class Material
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		Material(
			aiColor3D _colorDiffuse,
			aiColor3D _colorReflective,
			aiColor3D _colorEmissive,
			aiColor3D _colorAmbient,
			aiColor3D _colorSpecular,
			aiColor3D _colorTransparent,
			float _reflectivity,
			float _opacity,
			float _refractionIndex,
			float _shininess,
			float _shininessStrength,
			float _transparency,
			int _shadingModel
		) :
			colorDiffuse{ _colorDiffuse },
			colorReflective{ _colorReflective },
			colorEmissive{ _colorEmissive },
			colorAmbient{ _colorAmbient },
			colorSpecular{ _colorSpecular },
			colorTransparent{ _colorTransparent },

			reflectivity{ _reflectivity },
			opacity{ _opacity },
			refractionIndex{ _refractionIndex },
			shininess{ _shininess },
			shininessStrength{ _shininessStrength },
			transparency{ _transparency },

			shadingModel{ _shadingModel },

			hasDiffuseTexture{ false },
			diffuseTexture{ }
		{};

		Material(
			aiMaterial* material
		) :
			colorDiffuse{ },
			colorReflective{ },
			colorEmissive{ },
			colorAmbient{ },
			colorSpecular{ },
			colorTransparent{ },

			reflectivity{ 0.f },
			opacity{ 1.f },
			refractionIndex{ 1.f },
			shininess{ 0.f },
			shininessStrength{ 0.f },
			transparency{ 0.f },

			shadingModel{ 0 },

			hasDiffuseTexture{ false },
			diffuseTexture{}
		{
			initialize(material);
		};

		void setDiffuseTexture(std::unique_ptr<raytracing::Texture> texture);

		void resetDiffuseTexture();

		const aiColor3D& getDiffuse(const aiVector3D& uv) const
		{
			return this->hasDiffuseTexture ? this->diffuseTexture->getColor(uv) : this->colorDiffuse;
		}

		inline const aiColor3D& getReflective() const
		{
			return this->colorReflective;
		}

		inline const aiColor3D& getEmissive() const
		{
			return this->colorEmissive;
		}

		inline const aiColor3D& getAmbient() const
		{
			return this->colorAmbient;
		}

		inline const aiColor3D& getSpecular() const
		{
			return this->colorSpecular;
		}

		inline const aiColor3D& getTransparent() const
		{
			return this->colorTransparent;
		}

		inline float getReflectivity() const
		{
			return this->reflectivity;
		}

		inline float getOpacity() const
		{
			return this->opacity;
		}

		inline float getRefractionIndex() const
		{
			return this->refractionIndex;
		}

		inline float getShininess() const
		{
			return this->shininess;
		}

		inline float getShininessStrength() const
		{
			return this->shininessStrength;
		}

		inline float getTransparency() const
		{
			return this->transparency;
		}

		inline int getShadingModel() const
		{
			return this->shadingModel;
		}

		inline bool getHasDiffuseTexture() const
		{
			return this->hasDiffuseTexture;
		}

		const Texture* getDiffuseTexture() const
		{
			return hasDiffuseTexture ? this->diffuseTexture.get() : nullptr;
		}

	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:

		void initialize(aiMaterial* material);
	
	/*--------------------------------< Public members >------------------------------------*/
	public:
	
	/*--------------------------------< Protected members >---------------------------------*/
	protected:
	
	/*--------------------------------< Private members >-----------------------------------*/
	private:

		aiColor3D colorDiffuse;
		aiColor3D colorReflective;
		aiColor3D colorEmissive;
		aiColor3D colorAmbient;
		aiColor3D colorSpecular;
		aiColor3D colorTransparent;

		float reflectivity;
		float opacity;
		float refractionIndex;
		float shininess;
		float shininessStrength;
		float transparency;

		int shadingModel;

		bool hasDiffuseTexture;
		std::unique_ptr<raytracing::Texture> diffuseTexture;

	};
	
} // end of namespace raytracer