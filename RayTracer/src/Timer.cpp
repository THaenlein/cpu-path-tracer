/*
 * Timer.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include "Timer.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/
		
	Timer& Timer::getInstance()
	{
		static Timer instance;
		return instance;
	}

	void Timer::start()
	{
		this->lastTime = std::chrono::system_clock::now();
	}

	double Timer::stop()
	{
		auto end = std::chrono::system_clock::now();
		duration timeSpan = end - this->lastTime;
		this->lastTime = end;
		this->lastDuration = timeSpan;
		return timeSpan.count();
	}

	void Timer::reset()
	{
		this->lastTime = {};
		this->lastDuration = {};
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

	Timer::Timer()
	{
		this->reset();
	}
	
} // end of namespace raytracer
