#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//Analytic solution
			float A{ Vector3::Dot(ray.direction, ray.direction) };
			float B{ Vector3::Dot(2 * ray.direction, (ray.origin - sphere.origin)) };
			float C{ Vector3::Dot(ray.origin - sphere.origin, ray.origin - sphere.origin) - powf(sphere.radius, 2) };
			float discriminant{ powf(B, 2) - 4 * A * C };

			if (discriminant > 0)
			{
				float t{ (-B - sqrtf(discriminant)) / (2 * A) };

				if (t < ray.min)
				{
					t = (-B + sqrtf(discriminant)) / (2 * A);
				}
				
				if (t > ray.min && t < ray.max)
				{
					hitRecord.didHit = true;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.origin = ray.origin + t * ray.direction;
					hitRecord.t = t;
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
					return true;
				}
			}
			hitRecord.didHit = false;
			return false;
			

			//Geometric solution
			//We have sphere and viewray
			//Intersection with sphere if distance between point on ray and center of sphere equals radius of sphere

			//Calculate hitpoints on sphere
			/*
			Vector3 L = sphere.origin - ray.origin;
			float tca = Vector3::Dot(L, ray.direction);
			float odSquared = powf(Vector3::Reject(L, ray.direction).Magnitude(), 2.f);
			float thc = sqrtf(powf(sphere.radius, 2.f) - odSquared);
			float t0 = tca - thc;
			float t1 = tca + thc;
			//Calculate distance between hitpoint on ray and center of sphere
			Vector3 p = ray.origin + t0 * ray.direction;
			//Vector3 pointToCenter = sphere.origin - p;
			//float distPointAndSphereCenter = pointToCenter.Magnitude();

			//if more than 1 hitpoint
			if (t0 != t1)
			{
				//if sphere is in front of camera
				if (t0 > 0 && t1 > 0)
				{
					//if hitpoint is on ray interval
					if (t0 > ray.min && t0 < ray.max)
					{
						//if (distPointAndSphereCenter == sphere.radius)
						//{
							hitRecord.didHit = true;
							hitRecord.materialIndex = sphere.materialIndex;
							hitRecord.origin = p;
							hitRecord.t = t0;
							hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
							return true;
						//}
					}
				}
			}
			hitRecord.didHit = false;
			return false;
			*/
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{

			float t{ (Vector3::Dot(plane.origin - ray.origin, plane.normal) / Vector3::Dot(ray.direction, plane.normal)) };

			if (t > ray.min && t < ray.max)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.origin = ray.origin + t * ray.direction;
				hitRecord.t = t;
				hitRecord.normal = plane.normal;
				return true;
			}
			else
			{
				hitRecord.didHit = false;
				return false;
			}
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//Calculate two sides of triangle
			//Vector3 a{ triangle.v1 - triangle.v0 }, b{triangle.v2 - triangle.v0};
			//Get normal of triangle
			//Vector3 normal{ Vector3::Cross(a, b) };
			//normal.Normalize();

			/*
			//Check if view ray is perpendicular with triangle. If perpendicular, it isn't viewable
			if (Vector3::Dot(triangle.normal, ray.direction) == 0) return false;

			//Check culling mode
			if (!ignoreHitRecord)
			{
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling)
				{
					if (Vector3::Dot(triangle.normal, ray.direction) > 0) return false;
				}
				else if (triangle.cullMode == TriangleCullMode::FrontFaceCulling)
				{
					if (Vector3::Dot(triangle.normal, ray.direction) < 0) return false;
				}
			}
			//Cull mode is inverted for shadows
			else
			{
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling)
				{
					if (Vector3::Dot(triangle.normal, ray.direction) < 0) return false;
				}
				else if (triangle.cullMode == TriangleCullMode::FrontFaceCulling)
				{
					if (Vector3::Dot(triangle.normal, ray.direction) > 0) return false;
				}
			}

			//Calculate t-value
			Vector3 center{ (triangle.v0 + triangle.v1 + triangle.v2) / 3.f };
			Vector3 L{ center - ray.origin };
			float t{ Vector3::Dot(L, triangle.normal) / Vector3::Dot(ray.direction, triangle.normal) };

			//Check if triangle is in range of ray
			if (t < ray.min || t > ray.max) return false;

			//Calculate intersection point
			Vector3 intersectPoint{ ray.origin + t * ray.direction };

			//Check if intersection point is on inside of triangle (aka to the right of every side)
			Vector3 edgeA{ triangle.v1 - triangle.v0 };
			Vector3 pointToSide{ intersectPoint - triangle.v0 };
			if (Vector3::Dot(triangle.normal, Vector3::Cross(edgeA, pointToSide)) < 0) return false;

			Vector3 edgeB{ triangle.v2 - triangle.v1 };
			pointToSide = intersectPoint - triangle.v1;
			if (Vector3::Dot(triangle.normal, Vector3::Cross(edgeB, pointToSide)) < 0) return false;

			Vector3 edgeC{ triangle.v0 - triangle.v2 };
			pointToSide = intersectPoint - triangle.v2;
			if (Vector3::Dot(triangle.normal, Vector3::Cross(edgeC, pointToSide)) < 0) return false;

			hitRecord.didHit = true;
			hitRecord.materialIndex = triangle.materialIndex;
			hitRecord.origin = intersectPoint;
			hitRecord.t = t;
			hitRecord.normal = triangle.normal;
			return true;
			*/

			//M?ller?Trumbore intersection algorithm
			//Check culling mode
			if (!ignoreHitRecord)
			{
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling)
				{
					if (Vector3::Dot(triangle.normal, ray.direction) > 0) return false;
				}
				else if (triangle.cullMode == TriangleCullMode::FrontFaceCulling)
				{
					if (Vector3::Dot(triangle.normal, ray.direction) < 0) return false;
				}
			}
			//Cull mode is inverted for shadows
			else
			{
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling)
				{
					if (Vector3::Dot(triangle.normal, ray.direction) < 0) return false;
				}
				else if (triangle.cullMode == TriangleCullMode::FrontFaceCulling)
				{
					if (Vector3::Dot(triangle.normal, ray.direction) > 0) return false;
				}
			}

			const float EPSILON = 0.0000001f;
			Vector3 vertex0 = triangle.v0;
			Vector3 vertex1 = triangle.v1;
			Vector3 vertex2 = triangle.v2;
			Vector3 edge1, edge2, h, s, q;
			float a, f, u, v;
			edge1 = vertex1 - vertex0;
			edge2 = vertex2 - vertex0;
			//right order?
			h = Vector3::Cross(ray.direction, edge2);
			a = Vector3::Dot(edge1, h);
			if (a > -EPSILON && a < EPSILON) return false; //ray is parallel with triangle
			f = 1.f / a;
			s = ray.origin - vertex0;
			u = f * Vector3::Dot(s, h);
			if (u < 0 || u > 1) return false;
			// right order?
			q = Vector3::Cross(s, edge1);
			v = f * Vector3::Dot(ray.direction, q);
			if (v < 0 || (u + v) > 1) return false;

			float t = f * Vector3::Dot(edge2, q);
			if (t < ray.min || t > ray.max) return false;
			if (t > EPSILON)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = ray.origin + ray.direction * t;
				hitRecord.t = t;
				hitRecord.normal = triangle.normal;
				return true;
			}
			else
			{
				return false;
			}

		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}
		
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// slabtest
			if (!SlabTest_TriangleMesh(mesh, ray))
			{
				return false;
			}

			HitRecord hitRecordTestHit{};
			//hitRecordClosestHit.t = ray.max;
			int triangleAmount{ int(mesh.indices.size()) / 3 };

			for (int i{}; i < triangleAmount; ++i)
			{
				int index1{ i * 3 }, index2{ i * 3 + 1 }, index3{ i * 3 + 2 };
				
				int v0{ mesh.indices[index1] };
				int v1{ mesh.indices[index2] };
				int v2{ mesh.indices[index3] };

				Triangle triangle(mesh.transformedPositions[v0], mesh.transformedPositions[v1], mesh.transformedPositions[v2], mesh.transformedNormals[i]);
				triangle.materialIndex = mesh.materialIndex;
				triangle.cullMode = mesh.cullMode;
				HitTest_Triangle(triangle, ray, hitRecordTestHit);
				if (hitRecordTestHit.t < hitRecord.t)
				{
					hitRecord = hitRecordTestHit;
				}
			}
			if (hitRecordTestHit.didHit)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			Vector3 DirectionToLight{light.origin - origin};
			return DirectionToLight;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			Vector3 vectorFromTargetToLight{ light.origin - target };
			float radius{ vectorFromTargetToLight.SqrMagnitude() };

			return light.color * (light.intensity / radius);
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}