#include <iostream>
#include <memory>
#include <exception>
#include <thread>
#include <chrono>

#include "assimp\Importer.hpp"
#include "assimp\postprocess.h"
#include "assimp\scene.h"

#include "FCollada.h"
#include "FCDocument\FCDocument.h"
#include "FCDocument\FCDExtra.h"
#include "FCDocument\FCDLibrary.h"
#include "FCDocument\FCDLight.h"
#include "FCDocument\FCDGeometry.h"
#include "FCDocument\FCDGeometryMesh.h"
#include "FCDocument\FCDGeometryPolygons.h"
#include "FCDocument\FCDCamera.h"
#include "FCDocument\FCDMaterial.h"
#include "FCDocument\FCDEffect.h"
#include "FCDocument\FCDEffectParameter.h"
#include "FCDocument\FCDGeometrySource.h"
#include "FCDocument\FCDGeometryPolygonsInput.h"
#include "FCDocument\FCDSceneNode.h"
#include "FCDocument\FCDEntityReference.h"
#include "FCDocument\FCDEffectProfile.h"

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

	FCDocument* colladaFile = new FCDocument();
	const char* filename = "F:/Dokumente/GitHub/ray-tracer/RayTracer/res/testScene_cube.dae";
	FCollada::Initialize();
	FCollada::LoadDocumentFromFile(colladaFile, filename);
	if (!colladaFile)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Import of 3D scene from fcollada failed! \n");
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

	// Apply post processing

	// TODO: Post process with fcollada

	// Remove single points and lines not forming a face
#ifdef AI_CONFIG_PP_SBP_REMOVE
	#undef AI_CONFIG_PP_SBP_REMOVE
#endif
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

	// TODO: Output with fcollada
	FCDGeometryLibrary* sceneMeshes = colladaFile->GetGeometryLibrary();
	size_t numberOfMeshes = sceneMeshes->GetEntityCount();
	for (size_t currentMesh = 0; currentMesh < numberOfMeshes; currentMesh++)
	{
		FCDGeometry* geometry = sceneMeshes->GetEntity(currentMesh);
		const fstring& name = geometry->GetName();
		if (geometry->IsMesh())
		{
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Name of mesh: %s", name.c_str());
			FCDGeometryMesh* mesh = geometry->GetMesh();
			size_t numberOfFaces = mesh->GetFaceCount();
			size_t numberofVertices = mesh->GetFaceVertexCount();
			size_t vertexSourceCount = mesh->GetVertexSourceCount();
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t Mesh has %d faces with %d vertices", numberOfFaces, numberofVertices);
			
			for (size_t currentPolygon = 0; currentPolygon < mesh->GetPolygonsCount(); currentPolygon++)
			{
				FCDGeometryPolygons* polygon = mesh->GetPolygons(currentPolygon);
				if (polygon->GetPrimitiveType() == FCDGeometryPolygons::PrimitiveType::POLYGONS) // Only Polygons (Triangles) are supported
				{
					SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t Polygon %d", currentPolygon);
					for (size_t currentInputEntry = 0; currentInputEntry < polygon->GetInputCount(); currentInputEntry++)
					{
						FCDGeometryPolygonsInput* polygonInput = polygon->GetInput(currentInputEntry);
						size_t indexCount = polygonInput->GetIndexCount();
						uint32* indices = polygonInput->GetIndices();
						FCDGeometrySource* geometrySource = polygonInput->GetSource();
						FCDParameterListAnimatableFloat& vertexList = geometrySource->GetSourceData();
						uint32 stride = geometrySource->GetStride();
						FUDaeGeometryInput::Semantic vertexType = geometrySource->GetType();

						if (vertexType == FUDaeGeometryInput::Semantic::POSITION) // Only print position data
						{
							for (size_t currentIndex = 0; currentIndex < indexCount; currentIndex++)
							{
								std::string vertexString = 
									std::to_string(vertexList.at(*(indices + currentIndex) * stride + 0)) + ", " +
									std::to_string(vertexList.at(*(indices + currentIndex) * stride + 1)) + ", " +
									std::to_string(vertexList.at(*(indices + currentIndex) * stride + 2));
								SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t\t Face %d: Vertex %d has Coordinates xyz: %s", currentIndex/3, currentIndex, vertexString.c_str());
							}
						}

					}
				}
			}
		}
		std::cout << std::endl;
	}

	FCDCameraLibrary* sceneCameras = colladaFile->GetCameraLibrary();
	size_t numberOfCameras = sceneCameras->GetEntityCount();
	for (size_t currentCamera = 0; currentCamera < numberOfCameras; currentCamera++)
	{
		FCDCamera* camera = sceneCameras->GetEntity(currentCamera);
		const fstring& name = camera->GetName();
		FCDParameterAnimatableFloat& aspectRatio = camera->GetAspectRatio();
		
		// TODO: Get Position

		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Name of Camera %d: %s", currentCamera, name.c_str());
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t Aspect ratio: %f", *aspectRatio);
		// TODO: Print Position

		std::cout << std::endl;
	}

	FCDLightLibrary* sceneLights = colladaFile->GetLightLibrary();
	size_t numberOfLights = sceneLights->GetEntityCount();
	for (size_t currentLight = 0; currentLight < numberOfLights; currentLight++)
	{
		FCDLight* light = sceneLights->GetEntity(currentLight);
		const fstring& name = light->GetName();
		FCDParameterAnimatableColor3& color = light->GetColor();
		FCDParameterAnimatableFloat intensity = light->GetIntensity();
		FCDLight::LightType type = light->GetLightType();
		
		// TODO: Get Position
		FCDVisualSceneNodeLibrary* visualSceneNodes = colladaFile->GetVisualSceneLibrary();

		FCDSceneNode* sceneNode = visualSceneNodes->GetEntity(0);
		FCDSceneNode* lightNode = sceneNode->GetChild(4);
		FCDEntityInstance* instance = lightNode->GetInstance(0);
		FCDEntityReference* reference = instance->GetEntityReference();
		FCDEntity* ent = reference->GetEntity();
		FCDTransform* trans = lightNode->GetTransform(0);
		FMMatrix44 matrix = trans->ToMatrix();

		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Name of Light %d: %s", currentLight, name.c_str());
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t Color: %f %f %f", color->x, color->y, color->z);
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t Intensity: %f", *intensity);
		SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "\t Type: %s", type); // TODO: Get enum string
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t Position: %f, %f, %f", matrix[3][0], matrix[3][1], matrix[3][2]);
		// TODO: Print Position

		std::cout << std::endl;
	}

	FCDMaterialLibrary* sceneMaterials = colladaFile->GetMaterialLibrary();
	size_t numberOfMaterials = sceneMaterials->GetEntityCount();
	for (size_t currentMaterial = 0; currentMaterial < numberOfMaterials; currentMaterial++)
	{
		FCDMaterial* material = sceneMaterials->GetEntity(currentMaterial);
		const fstring& name = material->GetName();
		const FCDEffect* effect = material->GetEffect();
		const FCDEffectProfile* profile = effect->GetProfile(0); // TODO: Get Effects from here

		size_t effectParameterCount = effect->GetEffectParameterCount();
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Name of Material %d: %s", currentMaterial, name.c_str());
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t Effect count %d", effectParameterCount);
		
		for (size_t currentEffect = 0; currentEffect < effectParameterCount; currentEffect++)
		{
			const FCDEffectParameter* effectParameter = effect->GetEffectParameter(currentEffect);
			size_t annotationCount = effectParameter->GetAnnotationCount();
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t Effect %d", currentEffect);
			SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t\t Annotation count %d", annotationCount);

			for (size_t currentAnnotation = 0; currentAnnotation < annotationCount; currentAnnotation++)
			{
				const FCDEffectParameterAnnotation* effectParameterAnnotation = effectParameter->GetAnnotation(currentAnnotation);
				const fstring& effectParameterAnnotationName = effectParameterAnnotation->name;
				const FCDEffectParameterAnnotation::Parameter_type& effectParameterAnnotationType = effectParameterAnnotation->type;
				const fstring& effectParameterAnnotationValue = effectParameterAnnotation->value;

				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t\t Name of annotation %d: %s", currentAnnotation, effectParameterAnnotationName.c_str());
				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t\t Annotation type: %d", effectParameterAnnotationType);
				SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "\t\t Annotation value: %s", effectParameterAnnotationValue);
			}
		}


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