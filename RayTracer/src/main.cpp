#include <iostream>
#include <memory>


//#include "SDL.h"

#include "main.hpp"
#include "Application.hpp"



int main(int argc, char* argv[])
{
	raytracer::Application app;

	app.initialize();

	app.setUpApplication();

	std::string imagePath("F:/Dokumente/GitHub/ray-tracer/RayTracer/res/texture.bmp");
	std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture(app.loadTexture(imagePath));

	app.render(texture.get());

	std::cout << "Press Enter to exit SDL..." << std::endl;
	std::cin.get();

	app.cleanUp();

	std::cout << "Press Enter to quit..." << std::endl;
	std::cin.get();

	return 0;
}