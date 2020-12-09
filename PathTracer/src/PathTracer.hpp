/*
 * RayTracer.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <thread>
#include <mutex>
#include <atomic>
#include <time.h>
#include <stdlib.h>
#include <unordered_map>

#include "assimp/scene.h"

#include "Application.hpp"
#include "raytracing.hpp"
#include "settings.hpp"
#include "Types/SynchronizedQueue.hpp"
#include "Types/RenderJob.hpp"
#include "Types/AccelerationStructure.hpp"
#include "Types/Material.hpp"
#include "Textures/Texture.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/


	class PathTracer
	{
		static const uint16_t TILE_SIZE = 32;

		/*--------------------------------< Public methods >------------------------------------*/
	public:

		PathTracer(Application& app, const aiScene* scene, Settings settings, std::unique_ptr<AccelerationStructure> accStruct):
			application(app),
			scene(scene),
			renderSettings(settings),
			accelerationStructure(std::move(accStruct))
		{

		}

		~PathTracer()
		{
			delete[] this->pixels;
		}

		void initialize(const std::string sceneFilePath);

		void renderMultiThreaded();

		std::thread createRenderThread(std::atomic<uint8_t>& threadsTerminated)
		{
			return std::thread([&] 
			{ 
				renderMultiThreaded(); 
				threadsTerminated++; 
			});
		}

		inline const Uint24* getViewport()
		{
			return this->pixels;
		}

		/*--------------------------------< Protected methods >---------------------------------*/
	protected:

		/*--------------------------------< Private methods >-----------------------------------*/
	private:

		void render();

		void render(RenderJob& renderJob);

		void renderAntiAliased(RenderJob& renderJob);

		aiColor3D sampleLight(IntersectionInformation& intersectionInformation, uint8_t rayDepth);

		aiColor3D shadePixel(IntersectionInformation& intersectionInformation, uint8_t& rayDepth);

		bool calculateIntersection(aiRay& ray, IntersectionInformation& outIntersection);

		aiColor3D traceRay(aiRay& ray, uint8_t rayDepth = 0);
		
		aiColor3D tracePath(aiRay& ray, uint8_t rayDepth = 0);

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

		aiVector3D pixelShiftX;

		aiVector3D pixelShiftY;

		aiVector3D topLeftPixel;

		Application& application;

		const aiScene* scene;

		const Settings renderSettings;

		std::unique_ptr<AccelerationStructure> accelerationStructure;

		std::unordered_map<aiMaterial*, std::unique_ptr<Material>> materialMapping;

	};
	
} // end of namespace raytracing