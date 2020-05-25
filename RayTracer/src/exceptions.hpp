/*
 * exceptions.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <exception>

#include "assimp\types.h"

namespace raytracer
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	struct SdlException : public std::exception
	{
	public:

		SdlException(const char* message = "")
		{
			this->sdlError = message;
		}

		const char* what() const throw()
		{
			return "Unknown SDL exception.";
		}

		const char* getSdlError()
		{
			return this->sdlError;
		}
		
	private:
		
		const char* sdlError;
	};

	struct Initialization : public SdlException
	{
	public:

		Initialization(const char* message=""):
			SdlException(message)
		{

		}

		const char* what() const throw ()
		{
			return "Initialization of SDL failed!";
		}
	};

	struct BitmapLoad : public SdlException
	{
	public:

		BitmapLoad(const char* message = "") :
			SdlException(message)
		{

		}

		const char* what() const throw ()
		{
			return "Error on bitmap load!";
		}
	};

	struct TextureCreation : public SdlException
	{
	public:

		TextureCreation(const char* message = "") :
			SdlException(message)
		{

		}

		const char* what() const throw ()
		{
			return "Error on texture creation!";
		}
	};

	struct WindowCreation : public SdlException
	{
	public:

		WindowCreation(const char* message = "") :
			SdlException(message)
		{

		}

		const char* what() const throw ()
		{
			return "Error on window creation!";
		}
	};

	struct RendererCreation : public SdlException
	{
	public:

		RendererCreation(const char* message = "") :
			SdlException(message)
		{

		}

		const char* what() const throw ()
		{
			return "Error on renderer creation!";
		}
	};

	struct TextureUpdate : public SdlException
	{
	public:

		TextureUpdate(const char* message = "") :
			SdlException(message)
		{

		}

		const char* what() const throw ()
		{
			return "Error on texture update!";
		}
	};

// Force packing of struct to avoid padding bytes
#pragma pack(push, 1)
	struct Uint24
	{
		Uint24():
			color(0x000000)
		{}

		Uint24(int value) :
			color(value)
		{}

		Uint24(aiColor3D value)
		{
			uint8_t red = static_cast<uint8_t>(value.r * 255);
			uint8_t green = static_cast<uint8_t>(value.g * 255);
			uint8_t blue = static_cast<uint8_t>(value.b * 255);
			this->color = (red << 16) | (green << 8) | (blue << 0);
		}

		unsigned int color : 24;
	};
#pragma pack(pop)

	/*--------------------------------< Constants >-----------------------------------------*/

	/*--------------------------------< Public methods >------------------------------------*/
	
	/*--------------------------------< Protected methods >---------------------------------*/
	
	/*--------------------------------< Private methods >-----------------------------------*/
	
	/*--------------------------------< Public members >------------------------------------*/

	/*--------------------------------< Protected members >---------------------------------*/

	/*--------------------------------< Private members >-----------------------------------*/
	
} // end of namespace raytracer