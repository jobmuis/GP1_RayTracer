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
				else
				{
					hitRecord.didHit = false;
					return false;
				}
			}
			else
			{
				hitRecord.didHit = false;
				return false;
			}
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
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//HitTest_Triangle(const Triangle & triangle, const Ray & ray, HitRecord & hitRecord, bool ignoreHitRecord = false)

			//Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal)
			
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