/*
 * ErrorHandler.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <iostream>


#include "ErrorHandler.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	ErrorHandler& ErrorHandler::getInstance()
	{
		static ErrorHandler instance;
		return instance;
	}

	void ErrorHandler::reportError(const char* message, const char* additional)
	{
		SDL_Log("%s. %s", message, additional);
		std::cin.get();
	}

	void ErrorHandler::reportError(SdlException& exception)
	{
		SDL_Log("%s. %s", exception.what(), exception.getSdlError());
		std::cin.get();
	}
		
	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

	ErrorHandler::ErrorHandler()
	{

	}
	
} // end of namespace raytracing