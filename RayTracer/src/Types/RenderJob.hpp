/*
 * RenderJob.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <cstdint>

#include "../exceptions.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	/*--------------------------------< Constants >-----------------------------------------*/

	class RenderJob
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		RenderJob(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY) :
			startCoordinateX(startX),
			startCoordinateY(startY),
			endCoordinateX(endX),
			endCoordinateY(endY)
		{

		}

		RenderJob() :
			startCoordinateX(0),
			startCoordinateY(0),
			endCoordinateX(0),
			endCoordinateY(0)
		{

		}

		inline uint16_t getTileStartX()
		{
			return this->startCoordinateX;
		}
		
		inline uint16_t getTileStartY()
		{
			return this->startCoordinateY;
		}

		inline uint16_t getTileEndX()
		{
			return this->endCoordinateX;
		}

		inline uint16_t getTileEndY()
		{
			return this->endCoordinateY;
		}

		inline uint16_t getTileWidth()
		{
			return this->endCoordinateX - this->startCoordinateX;
		}

		inline uint16_t getTileHeight()
		{
			return this->endCoordinateY - this->startCoordinateY;
		}

		uint32_t getTileSize()
		{
			return this->getTileWidth() * this->getTileHeight();
		}

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

		uint16_t startCoordinateX;
		
		uint16_t endCoordinateX;
		
		uint16_t startCoordinateY;
		
		uint16_t endCoordinateY;

	};
	
} // end of namespace raytracing