/*
 * mathUtility.cpp
 */

/*--------------------------------< Includes >-------------------------------------------*/
#include <algorithm>

#include "mathUtility.hpp"


namespace utility
{
	/*--------------------------------< Defines >--------------------------------------------*/

	/*--------------------------------< Typedefs >-------------------------------------------*/

	/*--------------------------------< Constants >------------------------------------------*/
		
	/*--------------------------------< Public members >-------------------------------------*/

	float mathUtility::getRandomFloat(float lowerBound, float upperBound)
	{
		static std::default_random_engine randomEngine;
		static std::uniform_real_distribution<float> uniformDistribution(lowerBound, upperBound);
		return uniformDistribution(randomEngine);
	}

	aiVector3D mathUtility::uniformSampleHemisphere(const float r1, const float r2)
	{
		// cos(theta) = r1 = y
		// cos^2(theta) + sin^2(theta) = 1 -> sin(theta) = srtf(1 - cos^2(theta))

		float sinTheta = sqrtf(1.f - r1 * r1);
		float phi = 2.f * PI * r2;
		float x = sinTheta * cosf(phi);
		float z = sinTheta * sinf(phi);

		return aiVector3D(x, r1, z);
	}

	aiVector3D mathUtility::cosineSampleHemisphere(const float r1, const float r2)
	{
		const float r = sqrtf(r1);
		const float theta = 2.f * PI * r2;

		const float x = r * cosf(theta);
		const float z = r * sinf(theta);

		return aiVector3D(x, sqrt(1.f - r1), z);
	}

	void mathUtility::createCoordinateSystem(const aiVector3D& N, aiVector3D& Nt, aiVector3D& Nb)
	{
		if (std::fabs(N.x) > std::fabs(N.y))
		{
			Nt = aiVector3D(N.z, 0, -N.x) / sqrtf(N.x * N.x + N.z * N.z);
		}
		else
		{
			Nt = aiVector3D(0, -N.z, N.y) / sqrtf(N.y * N.y + N.z * N.z);
		}

		Nb = N ^ Nt;
	}

	float mathUtility::fresnel(
		const aiVector3D& incidenceVector,
		const aiVector3D& incidenceNormal,
		const float ior)
	{
		float cosIncidence = std::clamp(incidenceVector * incidenceNormal, -1.f, 1.f);
		float etaIncidence = 1.f;
		float etaTransmission = ior;
		if (cosIncidence > 0.f) { std::swap(etaIncidence, etaTransmission); }
		// Snell's law
		float sinTransmission = etaIncidence / etaTransmission * sqrtf(std::max(0.f, 1.f - cosIncidence * cosIncidence));

		if (sinTransmission >= 1.f)
		{
			// Total internal reflection
			return 1.f;
		}
		else
		{
			float cosTransmission = sqrtf(std::max(0.f, 1.f - sinTransmission * sinTransmission));
			cosIncidence = fabsf(cosIncidence);
			float Rs = ((etaTransmission * cosIncidence) - (etaIncidence * cosTransmission)) / ((etaTransmission * cosIncidence) + (etaIncidence * cosTransmission));
			float Rp = ((etaIncidence * cosIncidence) - (etaTransmission * cosTransmission)) / ((etaIncidence * cosIncidence) + (etaTransmission * cosTransmission));
			float reflectedLight = (Rs * Rs + Rp * Rp) / 2.f;
			return reflectedLight;
		}
	}

	aiVector3D mathUtility::calculateRefractionDirection(
		const aiVector3D& incidenceVector,
		const aiVector3D& incidenceNormal,
		const float ior)
	{
		float cosIncidence = std::clamp(incidenceVector * incidenceNormal, -1.0f, 1.0f);
		float etaIncidence = 1.f;
		float etaTransmission = ior;
		aiVector3D nRefraction(incidenceNormal);
		if (cosIncidence < 0.f)
		{
			// Inside medium
			cosIncidence = -cosIncidence;
		}
		else
		{
			// Outside medium
			std::swap(etaIncidence, etaTransmission);
			nRefraction = -incidenceNormal;
		}
		float eta = etaIncidence / etaTransmission;
		float criticalAngle = 1.f - eta * eta * (1.f - cosIncidence * cosIncidence);
		if (criticalAngle < 0.f)
		{
			return { 0.f, 0.f, 0.f };
		}
		else
		{
			return { eta * incidenceVector + (eta * cosIncidence - sqrtf(criticalAngle)) * nRefraction };
		}
	}

	aiVector3D mathUtility::calculateReflectionDirection(
		const aiVector3D& incidenceVector,
		const aiVector3D& incidenceNormal)
	{
		return { incidenceVector - 2 * (incidenceVector * incidenceNormal) * incidenceNormal };
	}

	bool mathUtility::russianRoulette(const float probability)
	{
		float random = getRandomFloat(0.f, 1.f);
		return (random < (probability * 0.9f)) ? false : true;
	}

	void mathUtility::calculateDepthOfFieldRay(aiRay* cameraRay, const float aperature, const float focalDistance)
	{
		// Uniform random point on the aperture
		float angle = getRandomFloat(0.f, 1.f) * 2.0f * PI;
		float radius = sqrt(getRandomFloat(0.f, 1.f));
		aiVector2D offset = aiVector2D(cos(angle), sin(angle)) * radius * aperature;

		// Find to intersection point with the focal plane
		aiVector3D focalPoint(cameraRay->pos + focalDistance * cameraRay->dir);
		aiVector3D focalRayDir(focalPoint - (cameraRay->pos + (offset.x + offset.y)));
		focalRayDir.Normalize();

		// Calculate position of focal ray
		aiVector3D focalRayPos(cameraRay->pos);
		focalRayPos = focalRayPos + (offset.x + offset.y);

		cameraRay->pos = focalRayPos;
		cameraRay->dir = focalRayDir;
	}

	bool mathUtility::rayTriangleIntersection(
		const aiRay& ray,
		std::vector<aiVector3D*> vecTriangle,
		aiVector3D* outIntersectionPoint,
		aiVector2D* outUV)
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

	aiVector3D mathUtility::calculateSmoothNormal(aiVector2D& uv, std::vector<aiVector3D*>& vertexNormals)
	{
		aiVector3D smoothNormal = 
			(1 - uv.x - uv.y) *
			*(vertexNormals[0]) + uv.x *
			*(vertexNormals[1]) + uv.y *
			*(vertexNormals[2]);
		return smoothNormal.Normalize();
	}

	void mathUtility::gammaCorrectSrgb(aiColor3D* color)
	{
		color->r = sRgb(color->r);
		color->g = sRgb(color->g);
		color->b = sRgb(color->b);
	}

	void mathUtility::gammaCorrectAdobeRgb(aiColor3D* color)
	{
		color->r = adobeRgb(color->r);
		color->g = adobeRgb(color->g);
		color->b = adobeRgb(color->b);
	}
		
	/*--------------------------------< Protected members >----------------------------------*/
		
	/*--------------------------------< Private members >------------------------------------*/

	float mathUtility::sRgb(float value)
	{
		static const float GAMMA = 1.f / 2.4f;
		static const float A = 1.055f;
		static const float B = -0.055f;
		static const float C = 12.92f;
		static const float D = 0.0031308f;

		if (value < 0.f)
		{
			return -sRgb(-value);
		}
		else if ((0.f <= value) && (value < D))
		{
			return C * value;
		}
		else if (value >= D)
		{
			return A * std::pow(value, GAMMA) + B;
		}
		else
		{
			// This case can not occur. This return suppresses warnings
			return value;
		}
	}

	float mathUtility::adobeRgb(float value)
	{
		static const float GAMMA = 1.f / 2.19921875f;

		if (value >= 0.f)
		{
			return std::pow(value, GAMMA);
		}
		else
		{
			return -std::pow(-value, GAMMA);
		}
	}
	
} // end of namespace raytracer