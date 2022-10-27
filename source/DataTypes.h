#pragma once
#include <cassert>

#include "Math.h"
#include "vector"
#include <iostream>

namespace dae
{
#pragma region GEOMETRY
	struct Sphere
	{
		Vector3 origin{};
		float radius{};

		unsigned char materialIndex{ 0 };
	};

	struct Plane
	{
		Vector3 origin{};
		Vector3 normal{};

		unsigned char materialIndex{ 0 };
	};

	enum class TriangleCullMode
	{
		FrontFaceCulling,
		BackFaceCulling,
		NoCulling
	};

	struct Triangle
	{
		Triangle() = default;
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal):
			v0{_v0}, v1{_v1}, v2{_v2}, normal{_normal.Normalized()}{}

		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }
		{
			const Vector3 edgeV0V1 = v1 - v0;
			const Vector3 edgeV0V2 = v2 - v0;
			normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
		}

		Vector3 v0{};
		Vector3 v1{};
		Vector3 v2{};

		Vector3 normal{};

		TriangleCullMode cullMode{};
		unsigned char materialIndex{};
	};

	struct TriangleMesh
	{
		TriangleMesh() = default;
		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, TriangleCullMode _cullMode):
		positions(_positions), indices(_indices), cullMode(_cullMode)
		{
			//Calculate Normals
			CalculateNormals();

			//Update Transforms
			UpdateTransforms();
		}

		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, const std::vector<Vector3>& _normals, TriangleCullMode _cullMode) :
			positions(_positions), indices(_indices), normals(_normals), cullMode(_cullMode)
		{
			UpdateTransforms();
		}

		std::vector<Vector3> positions{};
		std::vector<Vector3> normals{};
		std::vector<int> indices{};
		unsigned char materialIndex{};

		TriangleCullMode cullMode{TriangleCullMode::BackFaceCulling};

		Matrix rotationTransform{};
		Matrix translationTransform{};
		Matrix scaleTransform{};

		std::vector<Vector3> transformedPositions{};
		std::vector<Vector3> transformedNormals{};

		void Translate(const Vector3& translation)
		{
			translationTransform = Matrix::CreateTranslation(translation);
		}

		void RotateY(float yaw)
		{
			rotationTransform = Matrix::CreateRotationY(yaw);
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Matrix::CreateScale(scale);
		}

		void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
		{
			int startIndex = static_cast<int>(positions.size());

			positions.push_back(triangle.v0);
			positions.push_back(triangle.v1);
			positions.push_back(triangle.v2);

			indices.push_back(startIndex);
			indices.push_back(++startIndex);
			indices.push_back(++startIndex);

			normals.push_back(triangle.normal);

			//Not ideal, but making sure all vertices are updated
			if(!ignoreTransformUpdate)
				UpdateTransforms();
		}

		void CalculateNormals()
		{
			int triangleAmount{ int(indices.size()) / 3 };
			//std::cout << triangleAmount << '\n';

			for (int i{}; i < triangleAmount; ++i)
			{
				int index1{ i * 3 }, index2{ i * 3 + 1 }, index3{ i * 3 + 2 };
				
				int v0{ indices[index1] };
				int v1{ indices[index2] };
				int v2{ indices[index3]};

				//Calculate two sides of triangle
				Vector3 edgeA{ positions[v1] - positions[v0] }, edgeB{ positions[v2] - positions[v0] };
				Vector3 normal{ Vector3::Cross(edgeA, edgeB) };
				normal.Normalize();
				normals.push_back(normal);
			}
		}

		void UpdateTransforms()
		{
			//Calculate Final Transform 
			//const auto finalTransform = ...
			const auto finalTransform = scaleTransform * rotationTransform * translationTransform;
			transformedPositions.clear();
			transformedNormals.clear();

			//Transform Positions (positions > transformedPositions)
			transformedPositions.reserve(positions.size());
			for (const Vector3& point : positions)
			{
				//std::cout << "point " << point.x << ' ' << point.y << ' ' << point.z << '\n';
				Vector3 transformedPosition{ finalTransform.TransformPoint(point)};
				//std::cout << "TransformedPoint " << transformedPosition.x << ' ' << transformedPosition.y << ' ' << transformedPosition.z << '\n';
				transformedPositions.emplace_back(transformedPosition);
			}

			//Transform Normals (normals > transformedNormals)
			transformedNormals.reserve(normals.size());
			for (const Vector3& normal : normals)
			{
				Vector3 transformedNormal{ finalTransform.TransformVector(normal) };
				transformedNormals.emplace_back(transformedNormal);
			}

			//transformedPositions = positions;
			//transformedNormals = normals;
		}
	};
#pragma endregion
#pragma region LIGHT
	enum class LightType
	{
		Point,
		Directional
	};

	struct Light
	{
		Vector3 origin{};
		Vector3 direction{};
		ColorRGB color{};
		float intensity{};

		LightType type{};
	};
#pragma endregion
#pragma region MISC
	struct Ray
	{
		Vector3 origin{};
		Vector3 direction{};

		float min{ 0.0001f };
		float max{ FLT_MAX };
	};

	struct HitRecord
	{
		Vector3 origin{};
		Vector3 normal{};
		float t = FLT_MAX;

		bool didHit{ false };
		unsigned char materialIndex{ 0 };
	};
#pragma endregion
}