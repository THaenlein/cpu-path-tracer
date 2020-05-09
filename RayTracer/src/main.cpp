#include <iostream>
#include <memory>
#include <exception>
//#include <boost/log/trivial.hpp>

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
	}
	catch(raytracer::SdlException& exception)
	{
		raytracer::ErrorHandler::getInstance().reportError(exception);
		app.cleanUp();
		return 0;
	}

	SDL_Event sdlEvent;
	bool continueRendering{ true };

	while (continueRendering)
	{
		while (SDL_PollEvent(&sdlEvent))
		{
			// Do something about this
			//BOOST_LOG_TRIVIAL(debug) << "Event received:" << sdlEvent.type;

			switch (sdlEvent.type)
			{
			case SDL_QUIT:
				continueRendering = false;
				break;
			
			case SDL_WINDOWEVENT:
				switch (sdlEvent.window.event)
				{
				case SDL_WINDOWEVENT_MOVED:
					SDL_SetWindowPosition(app.mainWindow.get(), sdlEvent.window.data1, sdlEvent.window.data2);
					break;
				}
				break;
			}
		}

		app.render(texture.get());
	}

	raytracer::ErrorHandler::getInstance().reportInfo("Success. Press Enter to exit SDL...");
	std::cin.get();

	app.cleanUp();

	raytracer::ErrorHandler::getInstance().reportInfo("Press Enter to quit...");
	std::cin.get();

	return 0;
}