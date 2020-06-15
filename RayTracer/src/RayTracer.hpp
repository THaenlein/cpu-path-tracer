/*
 * RayTracer.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <thread>
#include <mutex>

#include "assimp/scene.h"

#include "Application.hpp"
#include "Types\SynchronizedQueue.hpp"
#include "Types\RenderJob.hpp"
#include "raytracing.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/


	class RayTracer
	{
		static const uint16_t TILE_SIZE = 32;

		/*--------------------------------< Public methods >------------------------------------*/
	public:

		RayTracer(Application& app, const aiScene* scene):
			application(app),
			scene(scene)
		{

		}

		void initialize();

		void renderMultiThreaded();

		void renderSingleThreaded();

		std::thread renderThread()
		{
			return std::thread([=] { renderMultiThreaded(); });
		}

		inline Uint24* getViewport()
		{
			return this->pixels;
		}

		/*--------------------------------< Protected methods >---------------------------------*/
	protected:

		/*--------------------------------< Private methods >-----------------------------------*/
	private:

		void render(Application& application, const aiScene* scene);

		void render(Application& application, const aiScene* scene, RenderJob& renderJob);

		bool rayTriangleIntersection(aiRay& ray, std::vector<aiVector3D*> vecTriangle, aiVector3D* outIntersectionPoint);

		Uint24 shadePixel(const aiScene* scene, IntersectionInformation& intersectionInformation);

		bool calculateIntersection(
			aiRay& ray,
			const aiScene* scene,
			IntersectionInformation& outIntersection);

		Uint24 traceRay(aiRay& ray, const aiScene* scene);

		void createJobs();

		/*--------------------------------< Public members >------------------------------------*/
	public:

		/*--------------------------------< Protected members >---------------------------------*/
	protected:

		/*--------------------------------< Private members >-----------------------------------*/
	public:

		// TODO: Evaluate how much memory is internally used: RGB (3 Byte) or RGBA (4 Byte) per pixel?
		// Using 2d-array of 24-bit integer to encode color
		// rrrrrrrr gggggggg bbbbbbbb
		Uint24* pixels;

		SynchronizedQueue<RenderJob> renderJobs;

		uint16_t renderWidth;

		uint16_t renderHeight;

		aiVector3D pixelShiftX;

		aiVector3D pixelShiftY;

		aiVector3D bottomLeftPixel;

		Application& application;

		const aiScene* scene;

	};
	
} // end of namespace raytracing