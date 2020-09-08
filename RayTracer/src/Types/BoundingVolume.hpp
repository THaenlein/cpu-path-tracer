/*
 * <template>.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include "assimp\scene.h"

#include "AccelerationStructure.hpp"
#include "BoundingBox.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class BoundingVolume : public AccelerationStructure
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		BoundingVolume() = default;

		void initialize(const aiScene* scene);

		bool calculateIntersection(aiRay& ray, IntersectionInformation& outIntersection) override;
	
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

		std::vector<std::unique_ptr<BoundingBox>> bBoxes;

	};
	
} // end of namespace raytracer