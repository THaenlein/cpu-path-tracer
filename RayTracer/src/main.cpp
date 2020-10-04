#include <iostream>
#include <memory>
#include <exception>
#include <thread>
#include <chrono>
#include <string>
#include <experimental\filesystem>

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
#include "Utility\ArgParser.hpp"

namespace filesystem = std::experimental::filesystem;


int main(int argc, char* argv[])
{
	utility::ArgParser options(argc, argv);

	if (options.cmdOptionExists("-h"))
	{
		// Print usage and exit
		std::cout << "Usage: "
			"--input <path to collada scene file> "
			"[--width <render-width>] "
			"[--height <render-width>] "
			"[--max-samples <number of max samples>] "
			"[--max-depth <max ray depth>] "
			"[--bias <bias as float>] "
			"[--aperture <aperture as float>] "
			"[--focal <focal distance as float>] "
			"[--threading <number of threads for rendering>] " << std::endl;
		return 0;
	}

	bool useAA{ false };
	if (options.cmdOptionExists("--use-anti-aliasing"))
	{
		// Print usage and exit
		useAA = true;
	}

	uint16_t width{ 512U };
	const std::string& widthStr(options.getCmdOption("--width"));
	if (widthStr.empty())
	{
		// No width provided
	}
	else
	{
		width = std::stoi(widthStr);
	}

	uint16_t height{ 512U };
	const std::string& heightStr(options.getCmdOption("--height"));
	if (heightStr.empty())
	{
		// No height provided
	}
	else
	{
		height = std::stoi(heightStr);
	}

	uint8_t samples{ 4U };
	const std::string& samplesStr(options.getCmdOption("--max-samples"));
	if (samplesStr.empty())
	{
		// No samples provided
	}
	else
	{
		samples = std::stoi(samplesStr);
	}

	uint8_t depth{ 4U };
	const std::string& depthStr(options.getCmdOption("--max-depth"));
	if (depthStr.empty())
	{
		// No depth provided
	}
	else
	{
		depth = std::stoi(depthStr);
	}

	float bias{ 0.001f };
	const std::string& biasStr(options.getCmdOption("--bias"));
	if (biasStr.empty())
	{
		// No bias provided
	}
	else
	{
		bias = std::stof(biasStr);
	}

	float aperture{ 0.f };
	bool useDOF{ false };
	const std::string& apertureStr(options.getCmdOption("--aperture"));
	if (apertureStr.empty())
	{
		// No aperture provided
		useDOF = false;
	}
	else
	{
		aperture = std::stof(apertureStr);
		useDOF = true;
	}

	float fDist{ 0.f };
	const std::string& fDistStr(options.getCmdOption("--focal"));
	if (fDistStr.empty())
	{
		// No focal distance provided
		useDOF = false;
	}
	else
	{
		fDist = std::stof(fDistStr);
		useDOF = true;
	}

	filesystem::path scenePath{};
	const std::string& scenePathStr(options.getCmdOption("--input"));
	if (scenePathStr.empty())
	{
		// No input provided
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No scene file prodvided! Please specifiy a collada scene file using option \"--input\". Exiting..");
		return 1;
	}
	else
	{
		scenePath = scenePathStr;
		if (scenePath.extension().string() != std::string(".dae"))
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Specified input file is not in collada format. Please provide *.dae file. Exiting..");
			return 1;
		}
	}

	uint8_t threadCount{ 1U };
	const std::string& threadsStr(options.getCmdOption("--threading"));
	if (threadsStr.empty())
	{
		// No thread count provided
	}
	else
	{
		threadCount = std::stoi(threadsStr);
	}


	raytracing::Application app(width, height);

	try
	{
		app.initialize();
		app.setUpSdl();
		app.createScreenTexture();
	}
	catch (raytracing::SdlException& exception)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, exception.what(), ": %s", exception.getSdlError());
		app.cleanUp();
		return 1;
	}

	Assimp::Importer assetImporter;
	const aiScene* scene = assetImporter.ReadFile(scenePath.string(), 0);
	if (!scene)
	{
		SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Import of 3D scene failed: %s", assetImporter.GetErrorString());
		app.cleanUp();
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
			aiFace* face = (mesh->mFaces) + currentFace;
			triangleMeshCollection.push_back({ std::make_pair(face, mesh), raytracing::ChildSide::UNDEFINED });
		}
	}
	std::unique_ptr<raytracing::KdNode> kdTree(raytracing::KdNode::buildTreeSAH(triangleMeshCollection));

	raytracing::Settings renderSettings(samples, depth, bias, aperture, fDist, useDOF, useAA);
	raytracing::RayTracer rayTracer(app, scene, renderSettings, std::move(kdTree));

	try
	{
		filesystem::path sceneDir = scenePath.remove_filename();
		rayTracer.initialize(sceneDir.string());
	}
	catch (std::exception& exception)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, exception.what());
		app.cleanUp();
		return 1;
	}

#if MULTI_THREADING
	// 4 threads render scenes generally fastet on my Intel Xeon X5675 system
	unsigned int numberOfThreads(threadCount);
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

	return 0;
}