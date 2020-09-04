#include <iostream>
#include <memory>
#include <exception>
#include <thread>
#include <chrono>

#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

#include "SDL.h"
//#include "SDL_Log.h"

#include "main.hpp"
#include "Application.hpp"
#include "exceptions.hpp"
#include "RayTracer.hpp"
#include "Timer.hpp"

void handleEvents(raytracing::Application& app, raytracing::RayTracer& rayTracer, std::vector<std::thread>& threadPool, std::atomic<uint8_t>& threadsTerminated)
{
	SDL_Event sdlEvent;
	bool doneRendering{ false };
	bool quitApplication{ false };

	while (!quitApplication)
	{
			while (SDL_PollEvent(&sdlEvent))
			{
				switch (sdlEvent.type)
				{
				case SDL_QUIT:
					quitApplication = true;
					if (!doneRendering)
					{
						doneRendering = true;
						SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Terminated application before finishing render! Waiting for render threads to finish...");
						SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
							"Still running.",
							"Waiting for render threads to finish...",
							app.getWindow());
						// TODO: Evaluate if threads can safely be terminated instead of joining them
					}
					for (unsigned int threadToJoin = 0; threadToJoin < threadPool.size(); threadToJoin++)
					{
						threadPool[threadToJoin].join();
					}
					app.cleanUp();
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

			if (!doneRendering)
			{
				if (threadsTerminated == threadPool.size())
				{
					doneRendering = true;
					double renderingTime = raytracing::Timer::getInstance().stop();
					SDL_Log("Elapsed time for rendering scene: %.2f seconds", renderingTime);
				}
				app.updateRender(rayTracer.getViewport());
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
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
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, exception.what());
		app.cleanUp();
		return 0;
	}

	Assimp::Importer assetImporter;
	const std::string sceneFilePath("F:/Dokumente/GitHub/ray-tracer/RayTracer/res/testScene_cube.dae");
	const aiScene* scene = assetImporter.ReadFile(sceneFilePath, 0);
	if (!scene)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Import of 3D scene failed: %s", assetImporter.GetErrorString());
		app.cleanUp();
		return 0;
	}

	// Remove single points and lines not forming a face
#ifdef AI_CONFIG_PP_SBP_REMOVE
	#undef AI_CONFIG_PP_SBP_REMOVE
#endif
#define AI_CONFIG_PP_SBP_REMOVE aiPrimitiveType_POINTS | aiPrimitiveType_LINES;

	// Apply post processing
	assetImporter.ApplyPostProcessing(
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_FindDegenerates |
		aiProcess_SortByPType);

	for (unsigned int currentCamera = 0; currentCamera < scene->mNumCameras; currentCamera++)
	{
		aiCamera* camera = scene->mCameras[currentCamera];
		aiString name = camera->mName;
		aiNode* cameraNode = scene->mRootNode->FindNode(name);

		/* Transformation matrix layout
		 *
		 * [Right.x] [Up.x] [Back.x] [Position.x]
		 * [Right.y] [Up.y] [Back.y] [Position.y]
		 * [Right.z] [Up.z] [Back.z] [Position.Z]
		 * [       ] [    ] [      ] [Unit Scale]
		 */
		aiMatrix4x4& transMatrix = cameraNode->mTransformation;
		camera->mRight = { transMatrix.a1, transMatrix.b1, transMatrix.c1 };
		camera->mUp = { transMatrix.a2, transMatrix.b2, transMatrix.c2 };
		camera->mLookAt = { -transMatrix.a3, -transMatrix.b3, -transMatrix.c3 };
		camera->mPosition = { transMatrix.a4, transMatrix.b4, transMatrix.c4 };

		// Normalize vectors
		camera->mRight.Normalize();
		camera->mUp.Normalize();
		camera->mLookAt.Normalize();
	}

	for (unsigned int currentLight = 0; currentLight < scene->mNumLights; currentLight++)
	{
		aiLight* light = scene->mLights[currentLight];
		aiString name = light->mName;
		aiNode* lightNode = scene->mRootNode->FindNode(name);

		/* Transformation matrix layout
		*
		* [Right.x] [Up.x] [Back.x] [Position.x]
		* [Right.y] [Up.y] [Back.y] [Position.y]
		* [Right.z] [Up.z] [Back.z] [Position.Z]
		* [       ] [    ] [      ] [Unit Scale]
		*/
		aiMatrix4x4& transMatrix = lightNode->mTransformation;
		if (!(light->mType == aiLightSource_POINT)) // Up and direction vectors are undefined for point lights
		{
			light->mUp = { transMatrix.a2, transMatrix.b2, transMatrix.c2 };
			light->mDirection = { -transMatrix.a3, -transMatrix.b3, -transMatrix.c3 };
			// Normalize vectors
			light->mUp.Normalize();
			light->mDirection.Normalize();
		}

		light->mPosition = { transMatrix.a4, transMatrix.b4, transMatrix.c4 };
	}

	if (scene->HasMeshes())
	{
		aiMesh** meshes = scene->mMeshes;
		unsigned int numberOfMeshes = scene->mNumMeshes;
		for (unsigned int currentMesh = 0; currentMesh < numberOfMeshes; currentMesh++)
		{
			aiMesh* mesh = meshes[currentMesh];
			aiString name = mesh->mName;
			std::cout << "Name of Mesh " << currentMesh << ": " << name.C_Str() << std::endl;

			for (unsigned int currentFace = 0; currentFace < mesh->mNumFaces; currentFace++)
			{
				aiFace* face = (mesh->mFaces) + currentFace;
				*(face->mIndices), 20;
				//std::cout << "Face: " << mesh->mVertices[face->mIndices[0]].x << ", " << mesh->mVertices[face->mIndices[0]].y << ", " << mesh->mVertices[face->mIndices[0]].z << std::endl;
				//std::cout << "Face: " << mesh->mVertices[face->mIndices[1]].x << ", " << mesh->mVertices[face->mIndices[1]].y << ", " << mesh->mVertices[face->mIndices[1]].z << std::endl;
				//std::cout << "Face: " << mesh->mVertices[face->mIndices[2]].x << ", " << mesh->mVertices[face->mIndices[2]].y << ", " << mesh->mVertices[face->mIndices[2]].z << std::endl;
				//std::cout << std::endl;
			}
		}
		std::cout << std::endl;
	}

	if (scene->HasCameras())
	{
		aiCamera** cameras = scene->mCameras;
		unsigned int numberOfCameras = scene->mNumCameras;
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
	}
		
	if (scene->HasLights())
	{
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
	}

	if(scene->HasMaterials())
	{
		aiMaterial** materials = scene->mMaterials;
		unsigned int numberOfMaterials = scene->mNumMaterials;
		for (unsigned int currentMaterial = 0; currentMaterial < numberOfMaterials; currentMaterial++)
		{
			aiMaterial* material = materials[currentMaterial];
			aiString name = material->GetName();
			aiColor3D colorDiffuse{};
			material->Get(AI_MATKEY_COLOR_DIFFUSE, colorDiffuse);
			aiColor3D colorReflective{};
			material->Get(AI_MATKEY_COLOR_REFLECTIVE, colorReflective);
			aiColor3D colorSpecular{};
			material->Get(AI_MATKEY_COLOR_SPECULAR, colorSpecular);
			aiColor3D colorAmbient{};
			material->Get(AI_MATKEY_COLOR_AMBIENT, colorAmbient);
			aiColor3D colorEmissive{};
			material->Get(AI_MATKEY_COLOR_EMISSIVE, colorEmissive);
			aiColor3D colorTransparent{};
			material->Get(AI_MATKEY_COLOR_TRANSPARENT, colorTransparent);
			ai_real reflectivity{};
			material->Get(AI_MATKEY_REFLECTIVITY, reflectivity);
			ai_real refractionIndex{};
			material->Get(AI_MATKEY_REFRACTI, refractionIndex);
			ai_real opacity{};
			material->Get(AI_MATKEY_OPACITY, opacity);
			ai_real shininess{};
			material->Get(AI_MATKEY_SHININESS, shininess);
			ai_real shininessStrength{};
			material->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength);
			ai_real transparency{};
			material->Get(AI_MATKEY_TRANSPARENCYFACTOR, transparency);
			int shadingModel{};
			material->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

			std::cout << "Name of Material " << currentMaterial << ": " << name.C_Str() << std::endl;
			std::cout << '\t' << "Diffuse color: " << colorDiffuse.r << ", " << colorDiffuse.g << ", " << colorDiffuse.b << std::endl;
			std::cout << '\t' << "Specular color: " << colorSpecular.r << ", " << colorSpecular.g << ", " << colorSpecular.b << std::endl;
			std::cout << '\t' << "Reflective color: " << colorReflective.r << ", " << colorReflective.g << ", " << colorReflective.b << std::endl;
			std::cout << '\t' << "Ambient color: " << colorAmbient.r << ", " << colorAmbient.g << ", " << colorAmbient.b << std::endl;
			std::cout << '\t' << "Transparent color: " << colorTransparent.r << ", " << colorTransparent.g << ", " << colorTransparent.b << std::endl;
			std::cout << '\t' << "Emissive color: " << colorEmissive.r << ", " << colorEmissive.g << ", " << colorEmissive.b << std::endl;

			std::cout << '\t' << "Reflectivity: " << reflectivity << std::endl;
			std::cout << '\t' << "Shininess: " << shininess << std::endl; // Or Hardness
			std::cout << '\t' << "Shininess strength: " << shininessStrength << std::endl;
			std::cout << '\t' << "Opacity: " << opacity << std::endl; // Alpha blending
			std::cout << '\t' << "Transparency: " << transparency << std::endl;
			std::cout << '\t' << "Refraction Index: " << refractionIndex << std::endl;

			std::cout << '\t' << "Shading model: " << raytracing::getShadingModelString(shadingModel) << std::endl;

		}
		std::cout << std::endl;
	}

	{
		aiMetadata* metaData = scene->mMetaData;
		unsigned int propertyCount = metaData->mNumProperties;
		std::cout << "Scene meta data " << std::endl;
		for (unsigned int currentProperty = 0; currentProperty < propertyCount; currentProperty++)
		{
			aiMetadataEntry* entry;
			entry = &metaData->mValues[currentProperty];
			switch (entry->mType)
			{
				case aiMetadataType::AI_AISTRING:
				{
					if (aiString* decodedEntry = static_cast<aiString*>(entry->mData))
					{
						std::cout << '\t' << metaData->mKeys[currentProperty].C_Str() << ": " << decodedEntry->C_Str() << std::endl;
					}
				}break;

				case aiMetadataType::AI_AIVECTOR3D:
				{
					if (aiVector3D* decodedEntry = static_cast<aiVector3D*>(entry->mData))
					{
						std::cout << '\t' << ": " << decodedEntry->x << ", " << decodedEntry->y << ", " << decodedEntry->z << std::endl;
					}
				}break;

				case aiMetadataType::AI_BOOL:
				{
					if (bool* decodedEntry = static_cast<bool*>(entry->mData))
					{
						std::cout << ": " << decodedEntry << std::endl;
					}
				}break;

				case aiMetadataType::AI_DOUBLE:
				case aiMetadataType::AI_FLOAT:
				{
					if (ai_real* decodedEntry = static_cast<ai_real*>(entry->mData))
					{
						std::cout << '\t' << ": " << decodedEntry << std::endl;
					}
				}break;

				case aiMetadataType::AI_INT32:
				{
					if (int32_t* decodedEntry = static_cast<int32_t*>(entry->mData))
					{
						std::cout << '\t' << ": " << decodedEntry << std::endl;
					}
				}break;

				case aiMetadataType::AI_UINT64:
				{
					if (uint64_t* decodedEntry = static_cast<uint64_t*>(entry->mData))
					{
						std::cout << '\t' << ": " << decodedEntry << std::endl;
					}
				}break;

				default:
					std::cout << '\t' << "Invalid meta data entry type!" << std::endl;
			}
		}
	}
	
	raytracing::RayTracer rayTracer(app, scene);
	rayTracer.initialize();

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
		threadPool.push_back(rayTracer.renderThread(threadsTerminated));
	}
	handleEvents(app, rayTracer, threadPool, threadsTerminated);

	assetImporter.FreeScene();

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Press Enter to quit...");
	std::cin.get();

	return 0;
}