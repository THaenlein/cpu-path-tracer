/*
 * Application.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <iostream>
#include <string>
#include <memory>
#include <atomic>
#include <filesystem>

#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "exceptions.hpp"
#include "raytracing.hpp"
#include "Types/RenderJob.hpp"
#include "Types/SynchronizedQueue.hpp"
#include "settings.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	namespace filesystem = std::filesystem;

	/*--------------------------------< Constants >-----------------------------------------*/
	
	class Application
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:
	
		Application(Settings options) :
			mainWindow(nullptr, SDL_DestroyWindow),
			sdlRenderer(nullptr, SDL_DestroyRenderer),
			screenTexture(nullptr, SDL_DestroyTexture),
			renderSettings(options)
		{};
	
		void initialize();
	
		void setUpSdl();

		SDL_Window* getWindow()
		{
			return this->mainWindow.get();
		}

		std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> 
			loadTexture(const std::string& filePath);

		void createScreenTexture();

		void render(SDL_Texture* texture);

		void cleanUp();

		void handleEvents(
			const Uint24* viewport,
			std::vector<std::thread>& threadPool,
			std::atomic<uint8_t>& threadsTerminated,
			filesystem::path outputDir);

	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:

		void updateRender(const Uint24* pixels);

		int writeImage(filesystem::path outputDir, const void* data);

	/*--------------------------------< Public members >------------------------------------*/
	public:
	
	/*--------------------------------< Protected members >---------------------------------*/
	protected:
	
	/*--------------------------------< Private members >-----------------------------------*/
	private:
		std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> mainWindow;
	
		std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> sdlRenderer;

		std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> screenTexture;

		const Settings renderSettings;
	
	};
	
} // end of namespace raytracing