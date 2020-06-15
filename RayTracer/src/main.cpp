#include <iostream>
#include <memory>
#include <exception>
#include <thread>
#include <chrono>
#include <Windows.h>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "SDL.h"

#include "main.hpp"
#include "Application.hpp"
#include "exceptions.hpp"
#include "ErrorHandler.hpp"
#include "RayTracer.hpp"

void handleEvents(raytracing::Application& app)
{
	SDL_Event sdlEvent;
	bool continueRendering{ true };

	while (continueRendering)
	{
		while (SDL_PollEvent(&sdlEvent))
		{
			switch (sdlEvent.type)
			{
			case SDL_QUIT:
				continueRendering = false;
				break;

			case SDL_WINDOWEVENT:
				switch (sdlEvent.window.event)
				{
				case SDL_WINDOWEVENT_MOVED:
					SDL_SetWindowPosition(app.getWindow(), sdlEvent.window.data1, sdlEvent.window.data2);
					break;
				}
				break;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}


double get_wall_time() {
	LARGE_INTEGER time, freq;
	if (!QueryPerformanceFrequency(&freq)) {
		//  Handle error
		return 0;
	}
	if (!QueryPerformanceCounter(&time)) {
		//  Handle error
		return 0;
	}
	return (double)time.QuadPart / freq.QuadPart;
}


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
		raytracing::ErrorHandler::getInstance().reportError(exception);
		app.cleanUp();
		return 0;
	}

	Assimp::Importer assetImporter;
	const std::string sceneFilePath("F:/Dokumente/GitHub/ray-tracer/RayTracer/res/testScene_cube.dae");
	const aiScene* scene = assetImporter.ReadFile(sceneFilePath, 0);
	if (!scene)
	{
		raytracing::ErrorHandler::getInstance().reportError("Import of 3D scene failed: ", assetImporter.GetErrorString());
		app.cleanUp();
		return 0;
	}

	// Apply post processing

	// Remove single points and lines not forming a face
#define AI_CONFIG_PP_SBP_REMOVE aiPrimitiveType_POINTS | aiPrimitiveType_LINES;
	assetImporter.ApplyPostProcessing(
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_FindDegenerates |
		aiProcess_SortByPType);

	for (unsigned int currentCamera = 0; currentCamera < scene->mNumCameras; currentCamera++)
	{
		aiMatrix4x4 transMatrix;
		aiMatrix4x4 rotMatrix;
		aiCamera* camera = scene->mCameras[currentCamera];
		aiString name = camera->mName;
		aiNode* cameraNode = scene->mRootNode->FindNode(name);

		transMatrix *= cameraNode->mTransformation;
		rotMatrix = transMatrix;
		rotMatrix.a4 = rotMatrix.b4 = rotMatrix.c4 = 0.f;
		rotMatrix.d4 = 1.f;
		camera->mPosition *= transMatrix;

		camera->mUp *= rotMatrix;
		// TODO: Evaluate why mUp has to be flipped
		camera->mUp *= -1;
		camera->mUp.Normalize();

		camera->mLookAt *= rotMatrix;
		camera->mLookAt.Normalize();
	}

	for (unsigned int currentLight = 0; currentLight < scene->mNumLights; currentLight++)
	{
		aiMatrix4x4 transMatrix;
		aiMatrix4x4 rotMatrix;
		aiLight* light = scene->mLights[currentLight];
		aiString name = light->mName;
		aiNode* lightNode = scene->mRootNode->FindNode(name);

		transMatrix *= lightNode->mTransformation;
		rotMatrix = transMatrix;
		rotMatrix.a4 = rotMatrix.b4 = rotMatrix.c4 = 0.f;
		rotMatrix.d4 = 1.f;
		light->mPosition *= transMatrix;

		light->mUp *= rotMatrix;

		light->mUp.Normalize();

		light->mDirection *= rotMatrix;
		light->mDirection.Normalize();
	}

	if (scene->HasMeshes())
	{
		aiMesh** meshes = scene->mMeshes;
		unsigned int numberOfMeshes = scene->mNumMeshes;
		for (unsigned int currentMesh=0; currentMesh < numberOfMeshes; currentMesh++)
		{
			aiMesh* mesh = meshes[currentMesh];
			aiString name = mesh->mName;
			std::cout << "Name of Mesh " << currentMesh << ": " << name.C_Str() << std::endl;
			
			for (unsigned int currentFace = 0; currentFace < mesh->mNumFaces; currentFace++)
			{
				aiFace* face = (mesh->mFaces)+currentFace;
				*(face->mIndices),20;
				//std::cout << "Face: " << mesh->mVertices[face->mIndices[0]].x << ", " << mesh->mVertices[face->mIndices[0]].y << ", " << mesh->mVertices[face->mIndices[0]].z << std::endl;
				//std::cout << "Face: " << mesh->mVertices[face->mIndices[1]].x << ", " << mesh->mVertices[face->mIndices[1]].y << ", " << mesh->mVertices[face->mIndices[1]].z << std::endl;
				//std::cout << "Face: " << mesh->mVertices[face->mIndices[2]].x << ", " << mesh->mVertices[face->mIndices[2]].y << ", " << mesh->mVertices[face->mIndices[2]].z << std::endl;
				//std::cout << std::endl;
			}
		}

		std::cout << std::endl;

		aiCamera** cameras = scene->mCameras;
		unsigned int numberOfCameras = scene->mNumLights;
		for (unsigned int currentCamera = 0; currentCamera < numberOfCameras; currentCamera++)
		{
			aiCamera* camera = cameras[currentCamera];
			aiString name = camera->mName;
			aiVector3D pos = camera->mPosition;
			aiVector3D up = camera->mUp;
			aiVector3D right = (camera->mUp ^ camera->mLookAt).Normalize();
			aiVector3D lookAt = camera->mLookAt;
			std::cout << "Name of Camera " << currentCamera << ": " << name.C_Str() << std::endl;
			std::cout << '\t' << "Position: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
			std::cout << '\t' << "Up: " << up.x << ", " << up.y << ", " << up.z << std::endl;
			std::cout << '\t' << "Right: " << right.x << ", " << right.y << ", " << right.z << std::endl;
			std::cout << '\t' << "Look at: " << lookAt.x << ", " << lookAt.y << ", " << lookAt.z << std::endl;
		}

		std::cout << std::endl;
		
		aiLight** lights = scene->mLights;
		unsigned int numberOfLights = scene->mNumLights;
		for (unsigned int currentLight = 0; currentLight < numberOfLights; currentLight++)
		{
			aiLight* light = lights[currentLight];
			aiString name = light->mName;
			aiColor3D color = light->mColorDiffuse;
			aiVector3D pos = light->mPosition;
			aiVector3D dir = light->mDirection;
			std::cout << "Name of Light " << currentLight << ": " << name.C_Str() << std::endl;
			std::cout << '\t' << "Diffuse color: " << color.r << ", " << color.g << ", " << color.b << std::endl;
			std::cout << '\t' << "Position: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
			std::cout << '\t' << "Direction: " << dir.x << ", " << dir.y << ", " << dir.z << std::endl;
		}

		std::cout << std::endl;

		aiMaterial** materials = scene->mMaterials;
		unsigned int numberOfMaterials = scene->mNumMaterials;
		for (unsigned int currentMaterial = 0; currentMaterial < numberOfMaterials; currentMaterial++)
		{
			aiMaterial* material = materials[currentMaterial];
			aiString name = material->GetName();
			const char* type;
			unsigned int pMax = 1;
			aiColor3D colorDiffuse;
			material->Get(AI_MATKEY_COLOR_DIFFUSE, colorDiffuse);
			aiColor3D colorReflective;
			material->Get(AI_MATKEY_COLOR_REFLECTIVE, colorReflective);
			aiColor3D colorSpecular;
			material->Get(AI_MATKEY_COLOR_SPECULAR, colorSpecular);
			std::cout << "Name of Material " << currentMaterial << ": " << name.C_Str() << std::endl;
			std::cout << '\t' << "Diffuse color: " << colorDiffuse.r << ", " << colorDiffuse.g << ", " << colorDiffuse.b << std::endl;
			std::cout << '\t' << "Reflective color: " << colorReflective.r << ", " << colorReflective.g << ", " << colorReflective.b << std::endl;
			std::cout << '\t' << "Specular color: " << colorSpecular.r << ", " << colorSpecular.g << ", " << colorSpecular.b << std::endl;
		}
	}
	
	raytracing::RayTracer rayTracer(app, scene);
	rayTracer.initialize();
	// 4 threads render this scene fastet on my Intel Xeon X5675 system
	unsigned int numberOfThreads(4/*std::thread::hardware_concurrency()*/);
	std::vector<std::thread> threads;
	double start = get_wall_time();
	for (int i = 0; i < numberOfThreads; i++)
	{
		threads.push_back(rayTracer.renderThread());
	}
	//handleEvents(app);
	for (unsigned int threadToJoin = 0; threadToJoin < threads.size(); threadToJoin++)
	{
		threads[threadToJoin].join();
	}
	double end = get_wall_time();
	double elapsedTimeInSec = end - start;
	BOOST_LOG_TRIVIAL(info) << "Elapsed time for rendering scene: " << elapsedTimeInSec << " seconds.";
	app.presentRender(rayTracer.getViewport());

	raytracing::ErrorHandler::getInstance().reportInfo("Success. Press Enter to exit SDL...");
	std::cin.get();

	app.cleanUp();
	assetImporter.FreeScene();

	raytracing::ErrorHandler::getInstance().reportInfo("Press Enter to quit...");
	std::cin.get();

	return 0;
}