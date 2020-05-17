/*
 * exceptions.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <exception>

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