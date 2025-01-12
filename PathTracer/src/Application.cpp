/*
 * Application.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "stb_image_write.h"

#include "Application.hpp"
#include "Timer.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/

	/*--------------------------------< Public members >-------------------------------------*/
	
	void Application::initialize()
	{
		// We want to use main() as entry point
		SDL_SetMainReady();

		// Initializing SDL_VIDEO explicit
		// Initializing SDL_EVENTS, SDL_FILE_IO and SDL_THREADING implicit
		if (SDL_Init(SDL_INIT_VIDEO))
		{
			throw Initialization(SDL_GetError());
		}
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "SDL initialized!");
	}
	
	void Application::setUpSdl()
	{
		// Create SDL window
		this->mainWindow.reset(
			SDL_CreateWindow("PathTracer", 0, 25, this->renderSettings.getWidth(), this->renderSettings.getHeight(), SDL_WINDOW_SHOWN));
		if (!this->mainWindow)
		{
			throw WindowCreation(SDL_GetError());
		}
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Window created!");
	
		// Create SDL renderer
		this->sdlRenderer.reset(
			SDL_CreateRenderer(this->mainWindow.get(), -1, SDL_RENDERER_ACCELERATED));
		if (!this->sdlRenderer)
		{
			throw RendererCreation(SDL_GetError());
		}
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Renderer created!");
	}

	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> 
		Application::loadTexture(const std::string & filePath)
	{
		// Load bitmap image
		SDL_Surface* bitmapImage = SDL_LoadBMP(filePath.c_str());
		if (!bitmapImage)
		{
			throw BitmapLoad(SDL_GetError());
		}
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Bitmap loaded!");
		
		// Create texture of bitmap
		std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture{
			SDL_CreateTextureFromSurface(this->sdlRenderer.get(), bitmapImage),
			SDL_DestroyTexture};
		SDL_FreeSurface(bitmapImage);
		if (!texture)
		{
			throw TextureCreation(SDL_GetError());
		}
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Texture created!");

		return texture;
	}

	void Application::createScreenTexture()
	{
		// Maybe change static flag to streaming in the future
		this->screenTexture.reset(SDL_CreateTexture(this->sdlRenderer.get(),
			SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, this->renderSettings.getWidth(), this->renderSettings.getHeight()));
		if (!this->screenTexture)
		{
			throw TextureCreation(SDL_GetError());
		}
		SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Screen texture created!");
	}

	void Application::render(SDL_Texture* texture)
	{
		// Draw the texture
		SDL_RenderClear(this->sdlRenderer.get());
		SDL_RenderCopy(this->sdlRenderer.get(), texture, NULL, NULL);
		SDL_RenderPresent(this->sdlRenderer.get());
	}

	void Application::cleanUp()
	{
		SDL_DestroyRenderer(this->sdlRenderer.get());
		SDL_DestroyWindow(this->mainWindow.get());
		SDL_Quit();
	}

	void Application::handleEvents(
		const Uint24* const viewport,
		std::vector<std::thread>& threadPool,
		std::atomic<uint8_t>& threadsTerminated,
		filesystem::path outputDir)
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
							this->mainWindow.get());
						// TODO: Evaluate if threads can safely be terminated instead of joining them
					}
					for (std::thread& threadJoin : threadPool)
					{
						threadJoin.join();
					}
					this->cleanUp();
					break;

				case SDL_WINDOWEVENT:
					switch (sdlEvent.window.event)
					{
					case SDL_WINDOWEVENT_MOVED:
						SDL_SetWindowPosition(this->mainWindow.get(), sdlEvent.window.data1, sdlEvent.window.data2);
						break;
					}
					break;
				}
			}

			if (!doneRendering)
			{
				if (threadsTerminated.load() == static_cast<uint8_t>(threadPool.size()))
				{
					doneRendering = true;
					double renderingTime = raytracing::Timer::getInstance().stop();
					SDL_Log("Elapsed time for rendering scene: %.2f seconds", renderingTime);

					filesystem::path outFile = outputDir / "latest.png";
					if (!writeImage(outFile, viewport))
					{
						SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not write image to: %s", outFile.string().c_str());
					}
					else
					{
						SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Image written to: %s", outFile.string().c_str());
					}
				}
				updateRender(viewport);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	/*--------------------------------< Protected members >----------------------------------*/

	/*--------------------------------< Private members >------------------------------------*/

	void Application::updateRender(const Uint24* pixels)
	{
		int result = SDL_UpdateTexture(this->screenTexture.get(), NULL, pixels, this->renderSettings.getWidth() * sizeof(Uint24));
		if (result)
		{
			throw TextureUpdate(SDL_GetError());
		}
		this->render(this->screenTexture.get());
	}

	int Application::writeImage(filesystem::path outputDir, const void* data)
	{
		return stbi_write_png(
			outputDir.string().c_str(),
			this->renderSettings.getWidth(),
			this->renderSettings.getHeight(),
			sizeof(Uint24),
			data,
			this->renderSettings.getWidth() * sizeof(Uint24));
	}

} // end of namespace raytracing