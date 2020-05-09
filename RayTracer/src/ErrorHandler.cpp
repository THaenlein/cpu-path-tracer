/*
 * ErrorHandler.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <iostream>

#include "ErrorHandler.hpp"


namespace raytracer
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

	void ErrorHandler::reportCritical(const char* message, const char* additional)
	{
		// TODO: Use boost/log
		std::cout << message << " " << additional << std::endl;
		std::cin.get();
	}

	void ErrorHandler::reportCritical(const char* message, SdlException& exception)
	{
		// TODO: Use boost/log
		std::cout << exception.what() << " " << exception.getSdlError() << std::endl;
		std::cin.get();
	}
		
	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

	ErrorHandler::ErrorHandler()
	{

	}
	
} // end of namespace raytracer