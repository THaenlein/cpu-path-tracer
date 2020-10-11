/*
 * Timer.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <chrono>
#include <ctime>

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	using time = std::chrono::time_point<std::chrono::system_clock>;

	using duration = std::chrono::duration<double>;

	/*--------------------------------< Constants >-----------------------------------------*/

	class Timer
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		static Timer& getInstance();

		void start();

		double stop();

		void reset();

	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:

		Timer();

		Timer(Timer const&) = delete;

		void operator=(Timer const&) = delete;
	
	/*--------------------------------< Public members >------------------------------------*/
	public:
	
	/*--------------------------------< Protected members >---------------------------------*/
	protected:
	
	/*--------------------------------< Private members >-----------------------------------*/
	private:

		time lastTime;

		duration lastDuration;

	};
	
} // end of namespace raytracer