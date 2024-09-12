/*
 * ErrorHandler.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <string>
#include <exception>

#include <sdl2/SDL_log.h>

#include "exceptions.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class ErrorHandler
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:
		static ErrorHandler& getInstance();

		inline void reportTrace(const char* message, const char* additional = "")
		{
			SDL_Log("%s. %s", message, additional);
		}

		inline void reportDebug(const char* message, const char* additional = "")
		{
			SDL_Log("%s. %s", message, additional);
		}

		inline void reportInfo(const char* message, const char* additional = "")
		{
			SDL_Log("%s. %s", message, additional);
		}

		inline void reportWarning(const char* message, const char* additional = "")
		{
			SDL_Log("%s. %s", message, additional);
		}

		void reportError(const char* message, const char* additional = "");
		
		void reportError(SdlException& exception);

	
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
	
} // end of namespace raytracing