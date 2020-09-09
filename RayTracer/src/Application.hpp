/*
 * Application.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <iostream>
#include <string>
#include <memory>
#include <atomic>

#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "exceptions.hpp"
#include "raytracing.hpp"
#include "Types\RenderJob.hpp"
#include "Types\SynchronizedQueue.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/
	
	class Application
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:
	
		Application();
	
		void initialize();
	
		void setUpSdl();

		inline const uint16_t getRenderWidth()
		{
			return this->WINDOW_DIMENSION_X;
		}

		inline const uint16_t getRenderHeight()
		{
			return this->WINDOW_DIMENSION_Y;
		}

		SDL_Window* getWindow()
		{
			return this->mainWindow.get();
		}

		std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> 
			loadTexture(const std::string& filePath);

		void createScreenTexture();

		void render(SDL_Texture* texture);

		void cleanUp();

		void updateRender(Uint24* pixels);

		void handleEvents(Uint24* viewport, std::vector<std::thread>& threadPool, std::atomic<uint8_t>& threadsTerminated);

	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:

	/*--------------------------------< Public members >------------------------------------*/
	public:
	
	/*--------------------------------< Protected members >---------------------------------*/
	protected:
	
	/*--------------------------------< Private members >-----------------------------------*/
	private:
		std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> mainWindow;
	
		std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> sdlRenderer;

		std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> screenTexture;

		static const uint16_t WINDOW_DIMENSION_X = 512;

		static const uint16_t WINDOW_DIMENSION_Y = 512;
	
	};
	
} // end of namespace raytracing