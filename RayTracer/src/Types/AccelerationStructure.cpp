/*
 * <template>.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include "AccelerationStructure.hpp"


namespace raytracing
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/
		
	/*--------------------------------< Protected members >----------------------------------*/

	bool AccelerationStructure::rayTriangleIntersection(const aiRay& ray, std::vector<aiVector3D*> vecTriangle, aiVector3D* outIntersectionPoint, aiVector2D* outUV)
	{
		const float EPSILON = 1e-6f;
		// Invariant: A face always consists of 3 vertices
		// See: aiProcess_Triangulate flag in scene import
		aiVector3D* vertex0 = vecTriangle[0];
		aiVector3D* vertex1 = vecTriangle[1];
		aiVector3D* vertex2 = vecTriangle[2];
		aiVector3D edge1, edge2, pVec, tVec, qVec;
		float determinant, invDet, u, v;
		edge1 = *vertex1 - *vertex0;
		edge2 = *vertex2 - *vertex0;
		pVec = ray.dir ^ edge2;
		determinant = edge1 * pVec;
		if (determinant > -EPSILON && determinant < EPSILON)
		{
			return false; // This ray is parallel to this triangle.
		}
		invDet = 1.f / determinant;
		tVec = ray.pos - *vertex0;
		u = invDet * tVec * pVec;
		if (u < 0.0 || u > 1.0)
		{
			return false;
		}
		qVec = tVec ^ edge1;
		v = invDet * ray.dir * qVec;
		if (v < 0.0 || u + v > 1.0)
		{
			return false;
		}
		// At this stage we can compute t to find out where the intersection point is on the line.
		float t = invDet * edge2 * qVec;
		if (t > EPSILON) // ray intersection
		{
			*outIntersectionPoint = ray.pos + ray.dir * t;
			outUV->x = u;
			outUV->y = v;
			return true;
		}
		else // This means that there is a line intersection but not a ray intersection.
		{
			return false;
		}
	}
		
	/*--------------------------------< Private members >------------------------------------*/
	
} // end of namespace raytracer