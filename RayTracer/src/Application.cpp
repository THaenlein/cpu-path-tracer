/*
 * Application.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include "Application.hpp"


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
		// We want to use main() as entry point
		SDL_SetMainReady();
	}
	
	void Application::initialize()
	{
		// Initializing SDL_VIDEO, SDL_EVENTS, SDL_FILE_IO and SDL_THREADING
		if (SDL_Init(SDL_INIT_VIDEO))
		{
			std::string errorMessage("Initialization of SDL failed! ");
			this->reportFailure(errorMessage);
			//std::cout << "Initialization of SDL failed! " << SDL_GetError() << std::endl;
			//return 1;
		}
		std::cout << "SDL initialized!\n" << std::endl;
	}
	
	void Application::setUpApplication()
	{
		// Create SDL window
		/*std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> mainWindow{
			SDL_CreateWindow("RayTracer", 0, 25, WINDOW_DIMENSION_X, WINDOW_DIMENSION_Y, SDL_WINDOW_SHOWN),
			SDL_DestroyWindow };*/
		this->mainWindow.reset(
			SDL_CreateWindow("RayTracer", 0, 25, WINDOW_DIMENSION_X, WINDOW_DIMENSION_Y, SDL_WINDOW_SHOWN));
		if (!this->mainWindow)
		{
			std::string errorMessage("Creation of window failed! ");
			this->reportFailure(errorMessage);
			//std::cout << "Creation of window failed! " << SDL_GetError() << std::endl;
			SDL_Quit();
			//return 1;
		}
		std::cout << "Window created!\n" << std::endl;
	
		// Create SDL renderer
		/*std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer{
			SDL_CreateRenderer(mainWindow.get(), -1, SDL_RENDERER_ACCELERATED),
			SDL_DestroyRenderer };*/
		this->sdlRenderer.reset(
			SDL_CreateRenderer(this->mainWindow.get(), -1, SDL_RENDERER_ACCELERATED));
		if (!this->sdlRenderer)
		{
			SDL_DestroyWindow(this->mainWindow.get());
			std::string errorMessage("Creation of renderer failed! ");
			this->reportFailure(errorMessage);
			//std::cout << "Creation of renderer failed! " << SDL_GetError() << std::endl;
			SDL_Quit();
			//return 1;
		}
		std::cout << "Renderer created!\n" << std::endl;
	}

	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> 
		Application::loadTexture(const std::string & filePath)
	{
		// Load bitmap image
		//std::string imagePath("F:/Dokumente/GitHub/ray-tracer/RayTracer/res/texture.bmp"); // TODO: get proper ressource path
		SDL_Surface* bitmapImage = SDL_LoadBMP(filePath.c_str());
		if (!bitmapImage)
		{
			SDL_DestroyRenderer(this->sdlRenderer.get());
			SDL_DestroyWindow(this->mainWindow.get());
			std::string errorMessage("Error on bitmap load! ");
			this->reportFailure(errorMessage);
			//std::cout << "Error on bitmap load! " << SDL_GetError() << std::endl;
			SDL_Quit();
			//return 1;
		}
		std::cout << "Bitmap laoded!\n" << std::endl;
		
		std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture{
			SDL_CreateTextureFromSurface(this->sdlRenderer.get(), bitmapImage),
			SDL_DestroyTexture};
		SDL_FreeSurface(bitmapImage);
		if (!texture)
		{
			SDL_DestroyRenderer(this->sdlRenderer.get());
			SDL_DestroyWindow(this->mainWindow.get());
			std::string errorMessage("Texture creation failed! ");
			this->reportFailure(errorMessage);
			//std::cout << "Texture creation failed! " << SDL_GetError() << std::endl;
			SDL_Quit();
			//return 1;
		}
		std::cout << "Texture created!\n" << std::endl;

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

	void Application::reportFailure(std::string& message)
	{
		// TODO: CleanUp?

		std::cout << message << SDL_GetError() << std::endl; // TODO: Use boost for logging

		SDL_Quit();

		while (true);
	}

} // end of namespace raytracer