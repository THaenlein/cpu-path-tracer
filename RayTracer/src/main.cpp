#include <iostream>
#include <memory>
#include <exception>


//#include "SDL.h"

#include "main.hpp"
#include "Application.hpp"
#include "exceptions.hpp"
#include "ErrorHandler.hpp"

int main(int argc, char* argv[])
{
	raytracer::Application app;

	std::string imagePath("F:/Dokumente/GitHub/ray-tracer/RayTracer/res/texture.bmp");
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture(nullptr, SDL_DestroyTexture);

	try
	{
		app.initialize();
		app.setUpSdl();

		texture = app.loadTexture(imagePath);
		
		app.render(texture.get());
	}
	catch(raytracer::SdlException& exception)
	{
		raytracer::ErrorHandler::getInstance().reportCritical("Some error message.", exception);
		app.cleanUp();
		return 0;
	}

	std::cout << "Success. Press Enter to exit SDL..." << std::endl;
	std::cin.get();

	app.cleanUp();

	std::cout << "Press Enter to quit..." << std::endl;
	std::cin.get();

	return 0;
}