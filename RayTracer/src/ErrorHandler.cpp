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

	void ErrorHandler::reportError(const char* message, const char* additional)
	{
		BOOST_LOG_TRIVIAL(error) << message << " " << additional;
		std::cin.get();
	}

	void ErrorHandler::reportError(SdlException& exception)
	{
		BOOST_LOG_TRIVIAL(error) << exception.what() << " " << exception.getSdlError();
		std::cin.get();
	}
		
	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

	ErrorHandler::ErrorHandler()
	{

	}
	
} // end of namespace raytracer