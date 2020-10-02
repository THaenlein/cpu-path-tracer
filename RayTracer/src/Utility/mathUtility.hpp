/*
 * mathUtility.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>
#include <random>

#include "assimp\types.h"

namespace utility
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	// This is a float appropriation of PI. 
	// Due to floats lower precision there is an error of 8.742278000372485661672049712E-8 which is considered tolerable
	const float PI{ 3.1415927410125732421875f };

	class mathUtility
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		static float getRandomFloat(float lowerBound, float upperBound);

		static aiVector3D uniformSampleHemisphere(const float r1, const float r2);
		
		static aiVector3D cosineSampleHemisphere(const float r1, const float r2);

		static void createCoordinateSystem(const aiVector3D& N, aiVector3D& Nt, aiVector3D& Nb);

		static ai_real fresnel(
			const aiVector3D& incidenceVector,
			const aiVector3D& incidenceNormal,
			const ai_real ior);

		static aiVector3D calculateRefractionDirection(
			const aiVector3D& incidenceVector,
			const aiVector3D& incidenceNormal,
			const ai_real ior);

		//     I->       N         R->
		//      \        |        /
		//       \       |       /
		//        \      |      /
		//         \     |     /
		//          \    |    /
		//           \   |   /
		//            \  |  /
		//             \ | /
		//   ___________\|/___________
		//          G R O U N D 
		static aiVector3D calculateReflectionDirection(
			const aiVector3D& incidenceVector, 
			const aiVector3D& incidenceNormal);

		static bool russianRoulette(const float probability, unsigned int depth);

		static void calculateDepthOfFieldRay(aiRay* cameraRay, const float aperature, const float focalDistance);

		static bool rayTriangleIntersection(
			const aiRay& ray,
			std::vector<aiVector3D*> vecTriangle,
			aiVector3D* outIntersectionPoint,
			aiVector2D* outUV);

		static aiVector3D calculateSmoothNormal(aiVector2D& uv, std::vector<aiVector3D*>& vertexNormals);
	
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