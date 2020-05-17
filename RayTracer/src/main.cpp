#include <iostream>
#include <memory>
#include <exception>
//#include <boost/log/trivial.hpp>

#include "assimp/Importer.hpp"

#include "main.hpp"
#include "Application.hpp"
#include "exceptions.hpp"
#include "ErrorHandler.hpp"
#include "RayTracer.hpp"

int main(int argc, char* argv[])
{
	raytracer::Application app;
	raytracer::RayTracer rayTracer;

	try
	{
		app.initialize();
		app.setUpSdl();
		app.createScreenTexture();
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
			switch (sdlEvent.type)
			{
			case SDL_QUIT:
				continueRendering = false;
				break;
			
			case SDL_WINDOWEVENT:
				switch (sdlEvent.window.event)
				{
				case SDL_WINDOWEVENT_MOVED:
					SDL_SetWindowPosition(app.getWindow(), sdlEvent.window.data1, sdlEvent.window.data2);
					break;
				}
				break;
			}
		}

		try
		{
			rayTracer.render(app);
		}
		catch (raytracer::SdlException& exception)
		{
			raytracer::ErrorHandler::getInstance().reportError(exception);
			app.cleanUp();
			return 0;
		}
	}

	raytracer::ErrorHandler::getInstance().reportInfo("Success. Press Enter to exit SDL...");
	std::cin.get();

	app.cleanUp();

	raytracer::ErrorHandler::getInstance().reportInfo("Press Enter to quit...");
	std::cin.get();

	return 0;
}