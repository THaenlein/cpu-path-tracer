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
		
	void BoundingBox::split(BoundingBox& left, BoundingBox& right)
	{
		// Evaluate longest axis
		aiVector3D length = this->max - this->min;
		uint8_t indexOfAxisToSplitAlong = length.x >= length.y ? (length.x >= length.z ? 0 : 2) : (length.y >= length.z ? 1 : 2);
		float splitAt = length[indexOfAxisToSplitAlong] / 2.f;

		// Left box
		aiVector3D leftMin(this->min);
		aiVector3D leftMax(this->max);
		leftMax[indexOfAxisToSplitAlong] = splitAt;
		left.min = leftMin;
		left.max = leftMax;
		
		// Right box
		aiVector3D rightMin(this->min);
		rightMin[indexOfAxisToSplitAlong] = splitAt;
		aiVector3D rightMax(this->max);
		right.min = rightMin;
		right.max = rightMax;
	}

	bool BoundingBox::intersects(aiRay& ray)
	{
		aiVector3D tMin((this->min - ray.pos) / ray.dir);
		aiVector3D tMax((this->max - ray.pos) / ray.dir);
		float tMinimum = std::min(tMin[0], tMax[0]);
		float tMaximum = std::max(tMin[0], tMax[0]);

		tMinimum = std::max(tMinimum, std::min(tMin[1], tMax[1]));
		tMaximum = std::min(tMaximum, std::max(tMin[1], tMax[1]));
		tMinimum = std::max(tMinimum, std::min(tMin[2], tMax[2]));
		tMaximum = std::min(tMaximum, std::max(tMin[2], tMax[2]));

		return tMaximum >= tMinimum;
	}

	bool BoundingBox::operator==(const BoundingBox & other) const
	{
		return this->min.Equal(other.min) && this->max.Equal(other.max);
	}

	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

} // end of namespace raytracer