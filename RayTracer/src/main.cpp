#include <iostream>
#include <memory>
#include <exception>
#include <thread>
#include <chrono>

#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

#include "SDL.h"

#include "main.hpp"
#include "Application.hpp"
#include "exceptions.hpp"
#include "settings.hpp"
#include "RayTracer.hpp"
#include "Timer.hpp"
#include "Types\BoundingVolume.hpp"
#include "Types\KdNode.hpp"


int main(int argc, char* argv[])
{
	raytracing::Application app;

	try
	{
		app.initialize();
		app.setUpSdl();
		app.createScreenTexture();
	}
	catch(raytracing::SdlException& exception)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, exception.what(), ": %s", exception.getSdlError());
		app.cleanUp();
		std::cin.get();
		return 1;
	}

	Assimp::Importer assetImporter;
	const std::string sceneFilePath("F:/Dokumente/GitHub/ray-tracer/RayTracer/res/testScene_path_trace.dae");
	const aiScene* scene = assetImporter.ReadFile(sceneFilePath, 0);
	if (!scene)
	{
		SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Import of 3D scene failed: %s", assetImporter.GetErrorString());
		app.cleanUp();
		std::cin.get();
		return 1;
	}

	// Remove single points and lines not forming a face
#ifdef AI_CONFIG_PP_SBP_REMOVE
	#undef AI_CONFIG_PP_SBP_REMOVE
#endif
#define AI_CONFIG_PP_SBP_REMOVE aiPrimitiveType_POINTS | aiPrimitiveType_LINES;
	// Apply post processing
	scene = assetImporter.ApplyPostProcessing(
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_FindDegenerates |
		aiProcess_SortByPType);
	if (!scene)
	{
		SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Post processing of 3D scene failed: %s", assetImporter.GetErrorString());
		app.cleanUp();
		std::cin.get();
		return 1;
	}

	// Transform cameras to world space
	for (unsigned int currentCamera = 0; currentCamera < scene->mNumCameras; currentCamera++)
	{
		aiCamera* camera = scene->mCameras[currentCamera];
		aiString name = camera->mName;
		aiNode* cameraNode = scene->mRootNode->FindNode(name);
		aiMatrix4x4& transMatrix = cameraNode->mTransformation;

		camera->Transform(transMatrix);
	}

	// Transform lights to world space
	for (unsigned int currentLight = 0; currentLight < scene->mNumLights; currentLight++)
	{
		aiLight* light = scene->mLights[currentLight];
		aiString name = light->mName;
		aiNode* lightNode = scene->mRootNode->FindNode(name);
		aiMatrix4x4& transMatrix = lightNode->mTransformation;

		light->Transform(transMatrix);
	}

#ifdef DEBUG
	// Cameras
	for (unsigned int currentCamera = 0; currentCamera < scene->mNumCameras; currentCamera++)
	{
		scene->mCameras[currentCamera]->print(std::cout);
	}

	// Lights
	for (unsigned int currentLight = 0; currentLight < scene->mNumLights; currentLight++)
	{
		scene->mLights[currentLight]->print(std::cout);
	}

	// Meshes
	for (unsigned int currentMesh = 0; currentMesh < scene->mNumMeshes; currentMesh++)
	{
		scene->mMeshes[currentMesh]->print(std::cout);
	}

	// Materials
	for (unsigned int currentMaterial = 0; currentMaterial < scene->mNumMaterials; currentMaterial++)
	{
		scene->mMaterials[currentMaterial]->print(std::cout);
	}

	// Metadata
	{
		scene->mMetaData->print(std::cout);
	}
#endif

	// Create Kd-Tree
	std::vector<raytracing::KdTriangle> triangleMeshCollection;
	for (unsigned int currentMesh = 0; currentMesh < scene->mNumMeshes; currentMesh++)
	{
		aiMesh* mesh = scene->mMeshes[currentMesh];
		if (mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE)
		{
			// Ignore points, lines and faces with more than 3 edges
			continue;
		}
		for (unsigned int currentFace = 0; currentFace < mesh->mNumFaces; currentFace++)
		{
			aiFace* face = (mesh->mFaces)+currentFace;
			triangleMeshCollection.push_back({ std::make_pair(face, mesh), raytracing::ChildSide::UNDEFINED});
		}
	}
	std::unique_ptr<raytracing::KdNode> kdTree(raytracing::KdNode::buildTree(triangleMeshCollection));

	raytracing::Settings renderSettings(8, 3, 0.001f, 0.02f, 1.71333f);
	raytracing::RayTracer rayTracer(app, scene, renderSettings, std::move(kdTree));
	
	try
	{
		rayTracer.initialize("F:/Dokumente/GitHub/ray-tracer/RayTracer/res");
	}
	catch(std::exception& exception)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, exception.what());
		app.cleanUp();
		std::cin.get();
		return 1;
	}

#if MULTI_THREADING
	// 4 threads render scenes generally fastet on my Intel Xeon X5675 system
	unsigned int numberOfThreads(4/*std::thread::hardware_concurrency()*/);
#else
	unsigned int numberOfThreads(1);
#endif

	std::vector<std::thread> threadPool;
	std::atomic<uint8_t> threadsTerminated(0);
	raytracing::Timer::getInstance().start();
	for (unsigned int i = 0; i < numberOfThreads; i++)
	{
		threadPool.push_back(rayTracer.createRenderThread(threadsTerminated));
	}
	app.handleEvents(rayTracer.getViewport(), threadPool, threadsTerminated);

	assetImporter.FreeScene();

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Press Enter to quit...");
	std::cin.get();

	return 0;
}