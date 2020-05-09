/*
 * ErrorHandler.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <string>
#include <exception>

#include "exceptions.hpp"

namespace raytracer
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class ErrorHandler
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:
		static ErrorHandler& getInstance();

		void reportCritical(const char* message, const char* additional = "");
		
		void reportCritical(const char* message, SdlException& exception);

	
	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:
		ErrorHandler();

		ErrorHandler(ErrorHandler const&) = delete;

		void operator=(ErrorHandler const&) = delete;
	
	/*--------------------------------< Public members >------------------------------------*/
	public:
	
	/*--------------------------------< Protected members >---------------------------------*/
	protected:
	
	/*--------------------------------< Private members >-----------------------------------*/
	public:

	};
	
} // end of namespace raytracer