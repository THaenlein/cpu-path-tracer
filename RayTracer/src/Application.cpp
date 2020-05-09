/*
 * Application.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include "Application.hpp"
#include "exceptions.hpp"
#include "ErrorHandler.hpp"


namespace raytracer
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
	
	static const int WINDOW_DIMENSION_X = 1024;

	static const int WINDOW_DIMENSION_Y = 1024;

	/*--------------------------------< Public members >-------------------------------------*/
	Application::Application():
		mainWindow(nullptr, SDL_DestroyWindow),
		sdlRenderer(nullptr, SDL_DestroyRenderer)
	{
		
	}
	
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
		ErrorHandler::getInstance().reportInfo("SDL initialized!");
	}
	
	void Application::setUpSdl()
	{
		// Create SDL window
		this->mainWindow.reset(
			SDL_CreateWindow("RayTracer", 0, 25, WINDOW_DIMENSION_X, WINDOW_DIMENSION_Y, SDL_WINDOW_SHOWN));
		if (!this->mainWindow)
		{
			throw WindowCreation(SDL_GetError());
		}
		ErrorHandler::getInstance().reportInfo("Window created!");
	
		// Create SDL renderer
		this->sdlRenderer.reset(
			SDL_CreateRenderer(this->mainWindow.get(), -1, SDL_RENDERER_ACCELERATED));
		if (!this->sdlRenderer)
		{
			throw RendererCreation(SDL_GetError());
		}
		ErrorHandler::getInstance().reportInfo("Renderer created!");
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
		ErrorHandler::getInstance().reportInfo("Bitmap loaded!");
		
		// Create texture of bitmap
		std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture{
			SDL_CreateTextureFromSurface(this->sdlRenderer.get(), bitmapImage),
			SDL_DestroyTexture};
		SDL_FreeSurface(bitmapImage);
		if (!texture)
		{
			throw TextureCreation(SDL_GetError());
		}
		ErrorHandler::getInstance().reportInfo("Texture created!");

		return texture;
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

	/*--------------------------------< Protected members >----------------------------------*/

	/*--------------------------------< Private members >------------------------------------*/

} // end of namespace raytracer