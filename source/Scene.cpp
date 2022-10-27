#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene() :
		m_Materials({ new Material_SolidColor({1,0,0}) })
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for (auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		HitRecord hitRecordTestHit{}, hitRecordClosestHit{};
		hitRecordClosestHit.t = ray.max;

		for (const Sphere& sphere : m_SphereGeometries)
		{
			GeometryUtils::HitTest_Sphere(sphere, ray, hitRecordTestHit);
			if (hitRecordTestHit.t < hitRecordClosestHit.t)
			{
				hitRecordClosestHit = hitRecordTestHit;
			}
		}

		for (const Plane& plane : m_PlaneGeometries)
		{
			GeometryUtils::HitTest_Plane(plane, ray, hitRecordTestHit);
			if (hitRecordTestHit.t < hitRecordClosestHit.t)
			{
				hitRecordClosestHit = hitRecordTestHit;
			}
		}

		for (const TriangleMesh& triangleMesh : m_TriangleMeshGeometries)
		{
			GeometryUtils::HitTest_TriangleMesh(triangleMesh, ray, hitRecordTestHit);
			if (hitRecordTestHit.t < hitRecordClosestHit.t)
			{
				hitRecordClosestHit = hitRecordTestHit;
			}
		}

	/*	for (const Triangle& triangle : m_Triangles)
		{
			GeometryUtils::HitTest_Triangle(triangle, ray, hitRecordTestHit);
			if (hitRecordTestHit.t < hitRecordClosestHit.t)
			{
				hitRecordClosestHit = hitRecordTestHit;
			}
		}*/

		closestHit = hitRecordClosestHit;
		return;
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		for (const Sphere& sphere : m_SphereGeometries)
		{
			if (GeometryUtils::HitTest_Sphere(sphere, ray))
			{
				return true;
			}
		}

		for (const Plane& plane : m_PlaneGeometries)
		{
			if (GeometryUtils::HitTest_Plane(plane, ray))
			{
				return true;
			}
		}

		for (const TriangleMesh& triangleMesh : m_TriangleMeshGeometries)
		{
			if (GeometryUtils::HitTest_TriangleMesh(triangleMesh, ray))
			{
				return true;
			}
		}

		//for (const Triangle& triangle : m_Triangles)
		//{
		//	if (GeometryUtils::HitTest_Triangle(triangle, ray))
		//	{
		//		return true;
		//	}
		//}

		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}
#pragma endregion


#pragma region SCENE W2
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });

		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Plane
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matId_Solid_Green);
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matId_Solid_Green);
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matId_Solid_Yellow);
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matId_Solid_Yellow);
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matId_Solid_Magenta);

		//Spheres
		AddSphere(Vector3{ -1.75f, 1.f, 0.f }, 0.75f, matId_Solid_Red);
		AddSphere(Vector3{ 0.f, 1.f, 0.f }, 0.75f, matId_Solid_Blue);
		AddSphere(Vector3{ 1.75f, 1.f, 0.f }, 0.75f, matId_Solid_Red);
		AddSphere(Vector3{ -1.75f, 3.f, 0.f }, 0.75f, matId_Solid_Blue);
		AddSphere(Vector3{ 0.f, 3.f, 0.f }, 0.75f, matId_Solid_Red);
		AddSphere(Vector3{ 1.75f, 3.f, 0.f }, 0.75f, matId_Solid_Blue);

		//Light
		AddPointLight(Vector3{ 0.f, 5.f, -5.f }, 70.f, colors::White);
	}
#pragma endregion

#pragma region SCENE W3_TestScene
	void Scene_W3_TestScene::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		//default: Material id0 >> SolidColor Material (RED)
		/*constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });
		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });*/

		//Materials
		const auto matLambert_Red = AddMaterial(new Material_Lambert(colors::Red, 1.f));
		const auto matLambert_Blue = AddMaterial(new Material_Lambert(colors::Blue, 1.f));
		const auto matLambert_Yellow = AddMaterial(new Material_Lambert(colors::Yellow, 1.f));
		const auto matLambertPhong_Blue = AddMaterial(new Material_LambertPhong(colors::Blue, 1.f, 1.f, 60.f));
		//const auto matLambertPhong_Black = AddMaterial(new Material_LambertPhong(colors::Black, 1.f, 1.f, 60.f));
		//const auto matLambertPhong_Pink = AddMaterial(new Material_LambertPhong(colors::Pink, 1.f, 1.f, 60.f));
		//const auto matLambert_GreyBlue = AddMaterial(new Material_Lambert(colors::GreyBlue, 1.f));



		//Spheres
		AddSphere(Vector3{ -.75f, 1.f, .0f }, 1.f, matLambert_Red);
		AddSphere(Vector3{ .75f, 1.f, .0f }, 1.f, matLambertPhong_Blue);
		
		//Plane
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_Yellow);
		//AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GreyBlue);

		//Light
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 25.f, colors::White);
		AddPointLight(Vector3{ 0.f, 2.5f, -5.f }, 25.f, colors::White);
	}
#pragma endregion

#pragma region SCENE W3
	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));

		//Plane
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		//Temporary Lambert-Phong Spheres & Materials
		//const auto matLambertPhong1 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 3.f));
		//const auto matLambertPhong2 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 15.f));
		//const auto matLambertPhong3 = AddMaterial(new Material_LambertPhong(colors::Blue, 0.5f, 0.5f, 50.f));

		//AddSphere(Vector3{ -1.75f, 1.f, 0.f }, .75f, matLambertPhong1);
		//AddSphere(Vector3{ 0.f, 1.f, 0.f }, .75f, matLambertPhong2);
		//AddSphere(Vector3{ 1.75f, 1.f, 0.f }, .75f, matLambertPhong3);
		
		//Spheres
		AddSphere(Vector3{ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere(Vector3{ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere(Vector3{ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere(Vector3{ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere(Vector3{ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		//Light
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //BackLight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f }); 
	}

#pragma endregion

#pragma region SCENE W4_TestScene
	void Scene_W4_TestScene::Initialize()
	{
		m_Camera.origin = { 0.f, 1.f, -5.f };
		m_Camera.fovAngle = 45.f;

		//Materials
		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({.49f, 0.57f, 0.57f}, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//Planes
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		//Triangle (temp)
		//auto triangle = Triangle{ {-.75f, .5f, .0f}, {-.75f, 2.f, 0.f}, {.75f, .5f, 0.f} };
		//triangle.cullMode = TriangleCullMode::NoCulling;
		//triangle.materialIndex = matLambert_White;

		//m_Triangles.emplace_back(triangle);

		//Triangle Mesh
		//const auto triangleMesh = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		/*
		pMesh = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		pMesh->positions = {
			{ -.75f, -1.f, .0f}, 
			{-.75f, 1.f, .0f}, 
			{.75f, 1.f, 1.f}, 
			{.75f, -1.f, 0.f} };

		pMesh->indices = {
			0, 1, 2, //Triangle 1
			0, 2, 3  //Triangle 2
		};

		pMesh->CalculateNormals();

		pMesh->Translate({ 0.f, 1.5f, 0.f });
		//pMesh->RotateY(45.f);

		pMesh->UpdateTransforms();
		*/
		//pMesh->RotateY(90.f);
		//pMesh->UpdateTransforms();

		//Cube
		pMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		Utils::ParseOBJ("Resources/simple_cube.obj",
			pMesh->positions,
			pMesh->normals,
			pMesh->indices);

		pMesh->UpdateTransforms();

		pMesh->Scale({ .7f, .7f, .7f });
		pMesh->Translate({ .0f, 1.f, 0.f });


		//Lights
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //BackLight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });

	}

	void Scene_W4_TestScene::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);

		float rotVal{ 90.f };
		pMesh->RotateY(90 * pTimer->GetTotal());
		pMesh->UpdateTransforms();
	}

#pragma endregion

#pragma region SCENE W4_ReferenceScene
	void Scene_W4_ReferenceScene::Initialize()
	{
		sceneName = "Reference scene";
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;

		const auto matCT_GrayRoughMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, 1.f));
		const auto matCT_GrayMediumMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .6f));
		const auto matCT_GraySmoothMetal = AddMaterial(new Material_CookTorrence({ .972f, .960f, .915f }, 1.f, .1f));
		const auto matCT_GrayRoughPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, 1.f));
		const auto matCT_GrayMediumPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .6f));
		const auto matCT_GraySmoothPlastic = AddMaterial(new Material_CookTorrence({ .75f, .75f, .75f }, .0f, .1f));

		const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
		const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

		//Plane
		AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
		AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
		AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
		AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
		AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

		//Spheres
		AddSphere(Vector3{ -1.75f, 1.f, 0.f }, .75f, matCT_GrayRoughMetal);
		AddSphere(Vector3{ 0.f, 1.f, 0.f }, .75f, matCT_GrayMediumMetal);
		AddSphere(Vector3{ 1.75f, 1.f, 0.f }, .75f, matCT_GraySmoothMetal);
		AddSphere(Vector3{ -1.75f, 3.f, 0.f }, .75f, matCT_GrayRoughPlastic);
		AddSphere(Vector3{ 0.f, 3.f, 0.f }, .75f, matCT_GrayMediumPlastic);
		AddSphere(Vector3{ 1.75f, 3.f, 0.f }, .75f, matCT_GraySmoothPlastic);

		//CW Winding Order!
		const Triangle baseTriangle = { Vector3(-.75f, 1.5f, 0.f), Vector3(.75f, 0.f, 0.f), Vector3(-.75f, 0.f, 0.f) };

		m_Meshes[0] = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
		m_Meshes[0]->AppendTriangle(baseTriangle, true);
		m_Meshes[0]->Translate({ -1.75f, 4.5f, 0.f });
		m_Meshes[0]->UpdateTransforms();

		m_Meshes[1] = AddTriangleMesh(TriangleCullMode::FrontFaceCulling, matLambert_White);
		m_Meshes[1]->AppendTriangle(baseTriangle, true);
		m_Meshes[1]->Translate({ 0.f, 4.5f, 0.f });
		m_Meshes[1]->UpdateTransforms();

		m_Meshes[2] = AddTriangleMesh(TriangleCullMode::NoCulling, matLambert_White);
		m_Meshes[2]->AppendTriangle(baseTriangle, true);
		m_Meshes[2]->Translate({ 1.75f, 4.5f, 0.f });
		m_Meshes[2]->UpdateTransforms();
		
		//Lights
		AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //BackLight
		AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
		AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	}

	void Scene_W4_ReferenceScene::Update(Timer* pTimer)
	{
		Scene::Update(pTimer);
		//eerst in radians, dan coverten naar degrees
		const auto yawAngle = ((cos(pTimer->GetTotal()) + 1.f) / 2.f * PI_2) * TO_DEGREES;
		for (const auto m : m_Meshes)
		{
			m->RotateY(yawAngle);
			m->UpdateTransforms();
		}
	}

#pragma endregion

#pragma region SCENE W4_Bunny
	void Scene_W4_Bunny::Initialize()
	{
		sceneName = "Bunny scene";
		m_Camera.origin = { 0.f, 3.f, -9.f };
		m_Camera.fovAngle = 45.f;
	

	const auto matLambert_GrayBlue = AddMaterial(new Material_Lambert({ .49f, 0.57f, 0.57f }, 1.f));
	const auto matLambert_White = AddMaterial(new Material_Lambert(colors::White, 1.f));

	//Plane
	AddPlane(Vector3{ 0.f, 0.f, 10.f }, Vector3{ 0.f, 0.f, -1.f }, matLambert_GrayBlue); //BACK
	AddPlane(Vector3{ 0.f, 0.f, 0.f }, Vector3{ 0.f, 1.f, 0.f }, matLambert_GrayBlue); //BOTTOM
	AddPlane(Vector3{ 0.f, 10.f, 0.f }, Vector3{ 0.f, -1.f, 0.f }, matLambert_GrayBlue); //TOP
	AddPlane(Vector3{ 5.f, 0.f, 0.f }, Vector3{ -1.f, 0.f, 0.f }, matLambert_GrayBlue); //RIGHT
	AddPlane(Vector3{ -5.f, 0.f, 0.f }, Vector3{ 1.f, 0.f, 0.f }, matLambert_GrayBlue); //LEFT

	m_BunnyMesh = AddTriangleMesh(TriangleCullMode::BackFaceCulling, matLambert_White);
	Utils::ParseOBJ("Resources/lowpoly_bunny.obj",
		m_BunnyMesh->positions,
		m_BunnyMesh->normals,
		m_BunnyMesh->indices);

	m_BunnyMesh->UpdateTransforms();

	m_BunnyMesh->Scale({ 2.f, 2.f, 2.f });
	m_BunnyMesh->RotateY(180.f);

	m_BunnyMesh->UpdateTransforms();

	//Lights
	AddPointLight(Vector3{ 0.f, 5.f, 5.f }, 50.f, ColorRGB{ 1.f, .61f, .45f }); //BackLight
	AddPointLight(Vector3{ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f, .8f, .45f }); //Front Light Left
	AddPointLight(Vector3{ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f, .47f, .68f });
	
	}

#pragma endregion
}