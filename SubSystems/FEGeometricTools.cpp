#include "FEGeometricTools.h"
using namespace FocalEngine;

FEAABB::FEAABB()
{

}

FEAABB::FEAABB(glm::vec3 Min, glm::vec3 Max)
{
	min = Min;
	max = Max;

	size = abs(max.x - min.x);
	if (abs(max.y - min.y) > size)
		size = abs(max.y - min.y);

	if (abs(max.z - min.z) > size)
		size = abs(max.z - min.z);
}

FEAABB::FEAABB(std::vector<glm::vec3>& VertexPositions)
{
	min.x = VertexPositions[0].x;
	min.y = VertexPositions[0].y;
	min.z = VertexPositions[0].z;

	max.x = VertexPositions[0].x;
	max.y = VertexPositions[0].y;
	max.z = VertexPositions[0].z;

	for (size_t i = 1; i < VertexPositions.size(); i++)
	{
		if (min.x > VertexPositions[i].x)
			min.x = VertexPositions[i].x;

		if (min.y > VertexPositions[i].y)
			min.y = VertexPositions[i].y;

		if (min.z > VertexPositions[i].z)
			min.z = VertexPositions[i].z;

		if (max.x < VertexPositions[i].x)
			max.x = VertexPositions[i].x;

		if (max.y < VertexPositions[i].y)
			max.y = VertexPositions[i].y;

		if (max.z < VertexPositions[i].z)
			max.z = VertexPositions[i].z;
	}

	size = abs(max.x - min.x);
	if (abs(max.y - min.y) > size)
		size = abs(max.y - min.y);

	if (abs(max.z - min.z) > size)
		size = abs(max.z - min.z);
}

FEAABB::FEAABB(std::vector<float>& VertexPositions)
{
	min.x = VertexPositions[0];
	min.y = VertexPositions[1];
	min.z = VertexPositions[2];

	max.x = VertexPositions[0];
	max.y = VertexPositions[1];
	max.z = VertexPositions[2];

	for (size_t i = 3; i < VertexPositions.size(); i += 3)
	{
		if (min.x > VertexPositions[i])
			min.x = VertexPositions[i];

		if (min.y > VertexPositions[i + 1])
			min.y = VertexPositions[i + 1];

		if (min.z > VertexPositions[i + 2])
			min.z = VertexPositions[i + 2];

		if (max.x < VertexPositions[i])
			max.x = VertexPositions[i];

		if (max.y < VertexPositions[i + 1])
			max.y = VertexPositions[i + 1];

		if (max.z < VertexPositions[i + 2])
			max.z = VertexPositions[i + 2];
	}

	size = abs(max.x - min.x);
	if (abs(max.y - min.y) > size)
		size = abs(max.y - min.y);

	if (abs(max.z - min.z) > size)
		size = abs(max.z - min.z);
}

FEAABB::FEAABB(float* VertexPositions, int VertexCount)
{
	min.x = VertexPositions[0];
	min.y = VertexPositions[1];
	min.z = VertexPositions[2];

	max.x = VertexPositions[0];
	max.y = VertexPositions[1];
	max.z = VertexPositions[2];

	for (size_t i = 3; i < size_t(VertexCount); i += 3)
	{
		if (min.x > VertexPositions[i])
			min.x = VertexPositions[i];

		if (min.y > VertexPositions[i + 1])
			min.y = VertexPositions[i + 1];

		if (min.z > VertexPositions[i + 2])
			min.z = VertexPositions[i + 2];

		if (max.x < VertexPositions[i])
			max.x = VertexPositions[i];

		if (max.y < VertexPositions[i + 1])
			max.y = VertexPositions[i + 1];

		if (max.z < VertexPositions[i + 2])
			max.z = VertexPositions[i + 2];
	}

	size = abs(max.x - min.x);
	if (abs(max.y - min.y) > size)
		size = abs(max.y - min.y);

	if (abs(max.z - min.z) > size)
		size = abs(max.z - min.z);
}

FEAABB::~FEAABB()
{
}

glm::vec3 FEAABB::getMin()
{
	return min;
}

glm::vec3 FEAABB::getMax()
{
	return max;
}

bool FEAABB::rayIntersect(glm::vec3 RayOrigin, glm::vec3 RayDirection, float& distance)
{
	// ChatGPT 4 version:
	const float EPSILON = 0.000001f;

	glm::vec3 t1 = (min - RayOrigin) / RayDirection;
	glm::vec3 t2 = (max - RayOrigin) / RayDirection;

	// Check if the ray direction is parallel to the axis
	for (int i = 0; i < 3; ++i) {
		if (fabs(RayDirection[i]) < EPSILON) {
			t1[i] = -INFINITY;
			t2[i] = INFINITY;
		}
	}

	glm::vec3 tmin = glm::min(t1, t2);
	glm::vec3 tmax = glm::max(t1, t2);

	float tNear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
	float tFar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

	if (tNear < 0 || tNear > tFar) {
		return false;
	}
	else {
		distance = tNear;
		return true;
	}

	/*float tmin = (min.x - RayOrigin.x) / RayDirection.x;
	float tmax = (max.x - RayOrigin.x) / RayDirection.x;

	if (tmin > tmax) std::swap(tmin, tmax);

	float tymin = (min.y - RayOrigin.y) / RayDirection.y;
	float tymax = (max.y - RayOrigin.y) / RayDirection.y;

	if (tymin > tymax) std::swap(tymin, tymax);

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	float tzmin = (min.z - RayOrigin.z) / RayDirection.z;
	float tzmax = (max.z - RayOrigin.z) / RayDirection.z;

	if (tzmin > tzmax) std::swap(tzmin, tzmax);

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	if (tzmax < tmax)
		tmax = tzmax;

	distance = std::fmax(std::fmax(std::fmin(tmin, tmax), std::fmin(tymin, tymax)), std::fmin(tzmin, tzmax));
	return true;*/
}

// only for uniform sized AABB
FEAABB::FEAABB(glm::vec3 center, float size)
{
	float halfSize = size / 2.0f;
	min[0] = center[0] - halfSize;
	min[1] = center[1] - halfSize;
	min[2] = center[2] - halfSize;

	max[0] = center[0] + halfSize;
	max[1] = center[1] + halfSize;
	max[2] = center[2] + halfSize;

	this->size = abs(max.x - min.x);
	if (abs(max.y - min.y) > this->size)
		this->size = abs(max.y - min.y);

	if (abs(max.z - min.z) > this->size)
		this->size = abs(max.z - min.z);
}

FEAABB::FEAABB(FEAABB other, glm::mat4 transformMatrix)
{
	// firstly we generate 8 points that represent AABBCube.
	// bottom 4 points
	glm::vec4 bottomLeftFront = glm::vec4(other.min.x, other.min.y, other.max.z, 1.0f);
	glm::vec4 bottomRightFront = glm::vec4(other.max.x, other.min.y, other.max.z, 1.0f);
	glm::vec4 bottomRightBack = glm::vec4(other.max.x, other.min.y, other.min.z, 1.0f);
	glm::vec4 bottomLeftBack = glm::vec4(other.min.x, other.min.y, other.min.z, 1.0f);
	// top 4 points
	glm::vec4 topLeftFront = glm::vec4(other.min.x, other.max.y, other.max.z, 1.0f);
	glm::vec4 topRightFront = glm::vec4(other.max.x, other.max.y, other.max.z, 1.0f);
	glm::vec4 topRightBack = glm::vec4(other.max.x, other.max.y, other.min.z, 1.0f);
	glm::vec4 topLeftBack = glm::vec4(other.min.x, other.max.y, other.min.z, 1.0f);

	// transform each point of this cube
	bottomLeftFront = transformMatrix * bottomLeftFront;
	bottomRightFront = transformMatrix * bottomRightFront;
	bottomRightBack = transformMatrix * bottomRightBack;
	bottomLeftBack = transformMatrix * bottomLeftBack;

	topLeftFront = transformMatrix * topLeftFront;
	topRightFront = transformMatrix * topRightFront;
	topRightBack = transformMatrix * topRightBack;
	topLeftBack = transformMatrix * topLeftBack;

	// for more convenient searching
	std::vector<glm::vec4> allPoints;
	allPoints.push_back(bottomLeftFront);
	allPoints.push_back(bottomRightFront);
	allPoints.push_back(bottomRightBack);
	allPoints.push_back(bottomLeftBack);

	allPoints.push_back(topLeftFront);
	allPoints.push_back(topRightFront);
	allPoints.push_back(topRightBack);
	allPoints.push_back(topLeftBack);

	min = glm::vec3(FLT_MAX);
	max = glm::vec3(-FLT_MAX);
	for (auto point : allPoints)
	{
		if (point.x < min.x)
			min.x = point.x;

		if (point.x > max.x)
			max.x = point.x;

		if (point.y < min.y)
			min.y = point.y;

		if (point.y > max.y)
			max.y = point.y;

		if (point.z < min.z)
			min.z = point.z;

		if (point.z > max.z)
			max.z = point.z;
	}

	size = abs(max.x - min.x);
	if (abs(max.y - min.y) > size)
		size = abs(max.y - min.y);

	if (abs(max.z - min.z) > size)
		size = abs(max.z - min.z);
}

float FEAABB::getSize()
{
	return size;
}

glm::vec3 FEAABB::getCenter()
{
	return min + abs(min - max) / 2.0f;
}

// Check if a point is inside a triangle
bool pointInTriangle(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ap = p - a;

	float d1 = glm::dot(ab, ap);
	float d2 = glm::dot(ac, ap);
	if (d1 <= 0.0f && d2 <= 0.0f)
		return true;

	float d3 = glm::dot(ab, ac);
	float d4 = glm::dot(ab, ab);
	float d5 = glm::dot(ac, ac);
	float v = d3 * d2 - d5 * d1;
	if (v < 0.0f || v >(d3 * d3 - d4 * d5))
		return false;

	float w = d3 * d1 - d4 * d2;
	if (w < 0.0f || (v + w) >(d3 * d3 - d4 * d5))
		return false;

	return true;
}

bool FEAABB::IntersectsTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
	glm::vec3 edges[3] = { b - a, c - b, a - c };

	for (int i = 0; i < 3; ++i)
	{
		glm::vec3 origin = a + edges[i];
		glm::vec3 direction = glm::normalize(edges[i]);
		float distance;

		if (rayIntersect(origin, -direction, distance))
		{
			if (distance <= glm::length(edges[i]) || distance <= size)
			{
				return true;
			}
		}

		if (rayIntersect(origin, direction, distance))
		{
			if (distance <= glm::length(edges[i]) || distance <= size)
			{
				return true;
			}
		}
	}

	return false;
}

//bool FEAABB::IntersectsTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
//{
//	glm::vec3 triangleNormal = cross(b - a, c - a);
//
//	float minVal, maxVal;
//
//	// Check if any vertex of the triangle is inside the AABB
//	if ((a.x >= min.x && a.x <= max.x && a.y >= min.y && a.y <= max.y && a.z >= min.z && a.z <= max.z) ||
//		(b.x >= min.x && b.x <= max.x && b.y >= min.y && b.y <= max.y && b.z >= min.z && b.z <= max.z) ||
//		(c.x >= min.x && c.x <= max.x && c.y >= min.y && c.y <= max.y && c.z >= min.z && c.z <= max.z))
//	{
//		return true;
//	}
//
//	// Check if any of the AABB's edges intersect the triangle
//	std::vector<glm::vec3> vertices = { min, glm::vec3(max.x, min.y, min.z), glm::vec3(min.x, max.y, min.z), glm::vec3(min.x, min.y, max.z),
//										glm::vec3(max.x, max.y, min.z), glm::vec3(max.x, min.y, max.z), glm::vec3(min.x, max.y, max.z), max };
//
//	// Plane equation: Ax + By + Cz + D = 0
//	float A = triangleNormal.x;
//	float B = triangleNormal.y;
//	float C = triangleNormal.z;
//	float D = -(A * a.x + B * a.y + C * a.z);
//
//	// Intersection point of each AABB edge with the plane
//	for (int i = 0; i < 8; i++)
//	{
//		for (int j = i + 1; j < 8; j++)
//		{
//			glm::vec3 point1 = vertices[i];
//			glm::vec3 point2 = vertices[j];
//
//			if ((A * point1.x + B * point1.y + C * point1.z + D) * (A * point2.x + B * point2.y + C * point2.z + D) <= 0)
//			{
//				// The edge intersects the plane, calculate the intersection point
//				glm::vec3 direction = point2 - point1;
//				float t = -(A * point1.x + B * point1.y + C * point1.z + D) / (A * direction.x + B * direction.y + C * direction.z);
//				glm::vec3 intersectionPoint = point1 + t * direction;
//
//				// Check if the intersection point is inside the triangle
//				if (pointInTriangle(intersectionPoint, a, b, c))
//					return true;
//			}
//		}
//	}
//	/*for (int i = 0; i < 8; i++)
//	{
//		for (int j = i + 1; j < 8; j++)
//		{
//			glm::vec3 origin = vertices[i];
//			glm::vec3 direction = glm::normalize(vertices[j] - vertices[i]);
//
//			float distance;
//			if (rayIntersect(origin, direction, distance))
//			{
//				glm::vec3 intersectionPoint = origin + distance * direction;
//				if (pointInTriangle(intersectionPoint, a, b, c))
//					return true;
//			}
//		}
//	}*/
//
//	// If no intersection has been found so far, there is no intersection
//	return false;
//}