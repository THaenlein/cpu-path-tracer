/*
 * Application.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include "Application.hpp"

#include "ErrorHandler.hpp"


namespace raytracer
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/

	/*--------------------------------< Public members >-------------------------------------*/
	Application::Application():
		mainWindow(nullptr, SDL_DestroyWindow),
		sdlRenderer(nullptr, SDL_DestroyRenderer),
		screenTexture(nullptr, SDL_DestroyTexture)
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
			SDL_CreateWindow("RayTracer", 0, 25, this->WINDOW_DIMENSION_X, this->WINDOW_DIMENSION_Y, SDL_WINDOW_SHOWN));
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

	void Application::createScreenTexture()
	{
		// Maybe change static flag to streaming in the future
		this->screenTexture.reset(SDL_CreateTexture(this->sdlRenderer.get(),
			SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, this->WINDOW_DIMENSION_X, this->WINDOW_DIMENSION_Y));
		if (!this->screenTexture)
		{
			throw TextureCreation(SDL_GetError());
		}
		ErrorHandler::getInstance().reportInfo("Screen texture created!");
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

	void Application::presentRender(Uint24* pixels)
	{
		int result = SDL_UpdateTexture(this->screenTexture.get(), NULL, pixels, this->WINDOW_DIMENSION_X * sizeof(Uint24));
		if (result)
		{
			throw TextureUpdate(SDL_GetError());
		}
		this->render(this->screenTexture.get());
	}

	/*--------------------------------< Protected members >----------------------------------*/

	/*--------------------------------< Private members >------------------------------------*/

} // end of namespace raytracer