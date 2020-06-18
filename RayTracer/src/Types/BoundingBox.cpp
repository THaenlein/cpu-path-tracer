/*
 * BoundingBox.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include "BoundingBox.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/
		
	bool BoundingBox::intersects(aiRay& ray)
	{
		aiVector3D tmin;
		aiVector3D tmax;
		// Compute the inverse in case of ray.dir = -0
		aiVector3D one (1, 1, 1);
		aiVector3D inverse = one / ray.dir;

		if (ray.dir < aiVector3D(0.f, 0.f, 0.f))
		{
			tmin = inverse.SymMul(this->max - ray.pos);
			tmax = inverse.SymMul(this->min - ray.pos);
		}
		else
		{
			tmin = inverse.SymMul(this->min - ray.pos);
			tmax = inverse.SymMul(this->max - ray.pos);
		}

		aiVector3D tminimum = std::min(tmin, tmax);
		aiVector3D tmaximum = std::max(tmin, tmax);
		tminimum = std::max(tminimum, std::min(tmin, tmax));
		tmaximum = std::min(tmaximum, std::max(tmin, tmax));

		return  tminimum < tmaximum;
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

} // end of namespace raytracer