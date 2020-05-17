/*
 * Application.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <iostream>
#include <string>
#include <memory>

#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "exceptions.hpp"

namespace raytracer
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

		uint16_t getRenderWidth()
		{
			return this->WINDOW_DIMENSION_X;
		}

		uint16_t getRenderHeight()
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

		void presentRender(Uint24* pixels);
	
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

		static const uint16_t WINDOW_DIMENSION_X = 1024;

		static const uint16_t WINDOW_DIMENSION_Y = 1024;
	
	};
	
} // end of namespace raytracer