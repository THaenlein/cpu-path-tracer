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

		std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> 
			loadTexture(const std::string& filePath);

		void render(SDL_Texture* texture);

		void cleanUp();
	
	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:

	/*--------------------------------< Public members >------------------------------------*/
	public:
	
	/*--------------------------------< Protected members >---------------------------------*/
	protected:
	
	/*--------------------------------< Private members >-----------------------------------*/
	public:
		std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> mainWindow;
	
		std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> sdlRenderer;
	
	};
	
} // end of namespace raytracer