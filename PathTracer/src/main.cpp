#include <iostream>
#include <memory>
#include <exception>
#include <thread>
#include <chrono>
#include <string>
#include <filesystem>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "SDL.h"

#include "main.hpp"
#include "Application.hpp"
#include "exceptions.hpp"
#include "settings.hpp"
#include "PathTracer.hpp"
#include "Timer.hpp"
#include "Types/BoundingVolume.hpp"
#include "Types/KdNode.hpp"
#include "Utility/ArgParser.hpp"

namespace filesystem = std::filesystem;


int main(int argc, char* argv[])
{
#ifdef DEBUG
	char* n_argv[] = {
		"F:\\Dokumente\\GitHub\\path-tracer\\PathTracer\\Build\\Debug\\PathTracer.exe",
		"--use-anti-aliasing",
		"--input",
		"F:\\Dokumente\\GitHub\\path-tracer\\PathTracer\\res\\testScene_path_trace.dae",
		"--output",
		"F:\\Dokumente\\GitHub\\path-tracer\\PathTracer\\out",
		"--threading",
		"4",
		"--max-depth",
		"4",
		"--max-samples",
		"4"
		//"--aperture",
		//"0.02",
		//"--focal",
		//"1.31191"
	};
	argc = 12; //16
	argv = n_argv;
#endif

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
		width = static_cast<uint16_t>(std::stoi(widthStr));
	}

	uint16_t height{ 512U };
	const std::string& heightStr(options.getCmdOption("--height"));
	if (heightStr.empty())
	{
		// No height provided
	}
	else
	{
		height = static_cast<uint16_t>(std::stoi(heightStr));
	}

	uint8_t samples{ 4U };
	const std::string& samplesStr(options.getCmdOption("--max-samples"));
	if (samplesStr.empty())
	{
		// No samples provided
	}
	else
	{
		samples = static_cast<uint8_t>(std::stoi(samplesStr));
	}

	uint8_t depth{ 4U };
	const std::string& depthStr(options.getCmdOption("--max-depth"));
	if (depthStr.empty())
	{
		// No depth provided
	}
	else
	{
		depth = static_cast<uint8_t>(std::stoi(depthStr));
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

	filesystem::path outputDir{};
	const std::string& outputDirStr(options.getCmdOption("--output"));
	if (outputDirStr.empty())
	{
		// No output dir provided
		filesystem::path out(argv[0]);
		out = out.remove_filename() / "out";
		outputDir = out;
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "No output directory prodvided! Using standard output %s", outputDir.string().c_str());
	}
	else
	{
		outputDir = outputDirStr;
	}
	if (!filesystem::create_directories(outputDir))
	{
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to create output directory %s. Maybe it already exists? Proceeding..", outputDir.string().c_str());
	}
	filesystem::permissions(outputDir, filesystem::perms::all);

	uint8_t threadCount{ 1U };
	const std::string& threadsStr(options.getCmdOption("--threading"));
	if (threadsStr.empty())
	{
		// No thread count provided
	}
	else
	{
		threadCount = static_cast<uint8_t>(std::stoi(threadsStr));
	}

	raytracing::Settings renderSettings(width, height, samples, depth, bias, aperture, fDist, useDOF, useAA);
	raytracing::Application app(renderSettings);

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
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Import of 3D scene failed: %s", assetImporter.GetErrorString());
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
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Post processing of 3D scene failed: %s", assetImporter.GetErrorString());
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
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Building KD-Tree..");
	raytracing::Timer::getInstance().start();
	std::unique_ptr<raytracing::KdNode> kdTree;
	try
	{
		kdTree.reset(raytracing::KdNode::buildTreeSAH(triangleMeshCollection));
	}
	catch(raytracing::AccStructure& exception)
	{
		SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Building kd-tree failed: %s", exception.what());
		app.cleanUp();
		return 1;
	}
	double kdBuildingTime = raytracing::Timer::getInstance().stop();
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Done. Took %.2f seconds", kdBuildingTime);
	triangleMeshCollection.~vector();

	raytracing::PathTracer rayTracer(app, scene, renderSettings, std::move(kdTree));

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

	unsigned int numberOfThreads(threadCount);
	std::vector<std::thread> threadPool;
	std::atomic<uint8_t> threadsTerminated(0);
	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Start rendering..");
	raytracing::Timer::getInstance().start();
	for (unsigned int i = 0; i < numberOfThreads; i++)
	{
		threadPool.push_back(rayTracer.createRenderThread(threadsTerminated));
	}
	app.handleEvents(rayTracer.getViewport(), threadPool, threadsTerminated, outputDir);
	
	assetImporter.FreeScene();

	// Calculate Intersection/Traversal cost

	//std::vector<std::vector<aiVector3D*>> triangles;
	//std::vector<raytracing::BoundingBox> bBoxes;
	//for (raytracing::KdTriangle tri : triangleMeshCollection)
	//{
	//	aiFace* currentFace{ tri.faceMeshPair.first };
	//	aiMesh* associatedMesh{ tri.faceMeshPair.second };
	//	std::vector<aiVector3D*> faceTris;
	//	for (unsigned int currentIndex = 0; currentIndex < currentFace->mNumIndices; currentIndex++)
	//	{
	//		faceTris.push_back(&(associatedMesh->mVertices[currentFace->mIndices[currentIndex]]));
	//	}
	//	triangles.push_back(faceTris);
	//	bBoxes.push_back(raytracing::BoundingBox(tri));
	//}
	//
	//aiRay someRay({ 7.1867f, 0.f, 1.66125f }, {-0.939693f, 0.f, -0.34202f});
	//aiVector3D tempIs;
	//aiVector2D tempUv;
	//
	//raytracing::Timer::getInstance().start();
	//for (std::vector<aiVector3D*>& tri : triangles)
	//{
	//	utility::mathUtility::rayTriangleIntersection(someRay, tri, &tempIs, &tempUv);
	//
	//}
	//double intersectionTime = raytracing::Timer::getInstance().stop();
	//double averageIsTime = intersectionTime / static_cast<double>(triangles.size());
	//
	//raytracing::Timer::getInstance().start();
	//for (raytracing::BoundingBox& box : bBoxes)
	//{
	//	box.intersects(someRay);
	//}
	//double travTime = raytracing::Timer::getInstance().stop();
	//double averageTravTime = travTime / static_cast<double>(bBoxes.size());
	//
	//
	//SDL_Log("Intersection time: %f seconds", intersectionTime);
	//SDL_Log("Average intersection time: %f seconds", averageIsTime);
	//SDL_Log("Average trav time: %f seconds", averageTravTime);
	//SDL_Log("Traversal time: %f seconds", travTime);
	//std::cin.get();

	return 0;
}