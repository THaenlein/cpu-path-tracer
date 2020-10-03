/*
 * ArgParser.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <vector>
#include <string>

namespace utility
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	// https://stackoverflow.com/a/868894/14259791
	class ArgParser
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		ArgParser(int &argc, char **argv) 
		{
			for (int i = 1; i < argc; ++i)
			{
				this->tokens.push_back(std::string(argv[i]));
			}
		}

		const std::string& getCmdOption(const std::string &option) const;

		bool cmdOptionExists(const std::string &option) const;
	
	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:
	
	/*--------------------------------< Public members >------------------------------------*/
	public:
	
	/*--------------------------------< Protected members >---------------------------------*/
	protected:
	
	/*--------------------------------< Private members >-----------------------------------*/
	private:

		std::vector<std::string> tokens;

	};
	
} // end of namespace raytracer