#include <iostream>
#include <memory>
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include "main.hpp"

static const int WINDOW_DIMENSION_X = 1024;
static const int WINDOW_DIMENSION_Y = 1024;

int main(int argc, char* argv[])
{
	// We want to use main() as entry point
	SDL_SetMainReady();

	// Initializing SDL_VIDEO, SDL_EVENTS, SDL_FILE_IO and SDL_THREADING
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		std::cout << "Initialization of SDL failed! " << SDL_GetError() << std::endl;
		return 1;
	}
	else
	{
		std::cout << "SDL initialized!\n" << std::endl;
	}

	// Create SDL window
	std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> mainWindow{
		SDL_CreateWindow("RayTracer", 0, 25, WINDOW_DIMENSION_X, WINDOW_DIMENSION_Y, SDL_WINDOW_SHOWN),
		SDL_DestroyWindow};
	if (!mainWindow)
	{
		std::cout << "Creation of window failed! " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	else
	{
		std::cout << "Window created!\n" << std::endl;
	}

	// Create SDL renderer
	std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer{
		SDL_CreateRenderer(mainWindow.get(), -1, SDL_RENDERER_ACCELERATED),
		SDL_DestroyRenderer};
	if (!renderer)
	{
		SDL_DestroyWindow(mainWindow.get());
		std::cout << "Creation of renderer failed! " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	else
	{
		std::cout << "Renderer created!\n" << std::endl;
	}

	// Load bitmap image
	std::string imagePath("F:/Dokumente/GitHub/ray-tracer/RayTracer/res/texture.bmp"); // TODO: get proper ressource path
	SDL_Surface* bitmap = SDL_LoadBMP(imagePath.c_str());
	if (!bitmap)
	{
		SDL_DestroyRenderer(renderer.get());
		SDL_DestroyWindow(mainWindow.get());
		std::cout << "Error on bitmap load! " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	else
	{
		std::cout << "Bitmap laoded!\n" << std::endl;
	}

	// Create texture from bitmap
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture{
		SDL_CreateTextureFromSurface(renderer.get(), bitmap),
		SDL_DestroyTexture };
	SDL_FreeSurface(bitmap); // TODO: Check if smart-pointer was deleted
	if (!texture)
	{
		SDL_DestroyRenderer(renderer.get());
		SDL_DestroyWindow(mainWindow.get());
		std::cout << "Error on bitmap load! " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	else
	{
		std::cout << "Texture created!\n" << std::endl;
	}

	// Draw the texture
	SDL_RenderClear(renderer.get());
	SDL_RenderCopy(renderer.get(), texture.get(), NULL, NULL);
	SDL_RenderPresent(renderer.get());


	std::cout << "Press Enter to exit SDL..." << std::endl;
	std::cin.get();

	SDL_DestroyTexture(texture.get());
	SDL_DestroyRenderer(renderer.get());
	SDL_DestroyWindow(mainWindow.get());
	SDL_Quit();
	std::cout << "Quitting SDL!\n" << std::endl;

	std::cout << "Press Enter to quit..." << std::endl;
	std::cin.get();

	return 0;
}