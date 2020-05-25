/*
 * RayTracer.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <thread>

#include "Application.hpp"
#include "assimp/scene.h"

namespace raytracer
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class RayTracer
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:
		void render(Application* application, const aiScene* scene);
		
		std::thread renderThread(Application* app, const aiScene* scene) 
		{
			return std::thread([=] { render(app, scene); });
		}
	
	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:

		bool rayTriangleIntersection(aiRay& ray, std::vector<aiVector3D*> vecTriangle, aiVector3D* outIntersectionPoint);

		Uint24 shadePixel(aiVector3D* intersectionPoint, const aiScene* scene, std::vector<aiVector3D*> triangle, aiMesh* mesh);

		bool calculateIntersection(
			aiRay& ray,
			const aiScene* scene,
			std::vector<aiVector3D*>& outIntersectedTriangle,
			aiMesh*& outIntersectedMesh,
			aiVector3D* outIntersectionPoint);

		Uint24 traceRay(aiRay& ray, const aiScene* scene);
	
	/*--------------------------------< Public members >------------------------------------*/
	public:
	
	/*--------------------------------< Protected members >---------------------------------*/
	protected:
	
	/*--------------------------------< Private members >-----------------------------------*/
	public:

	};
	
} // end of namespace raytracer