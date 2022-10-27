//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <iostream>

using namespace dae;

//#define ASYNC
//#define PARALLEL_FOR

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	//camera.CalculateCameraToWorld();

	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const float aspectRatio{ float(m_Width) / float(m_Height) };
	const float fov{ tanf((camera.fovAngle * TO_RADIANS) / 2.f) };
	const Matrix cameraONB{ camera.CalculateCameraToWorld() };
	const float invtLightRayOffset{ 0.0001f };

#if defined(ASYNC)
	//Async execution

#elif defined(PARALLEL_FOR)
	//Parallel-For Execution
#else
	//Synchronous Execution

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			const float pxc{px + 0.5f}, pyc{py + 0.5f}, 
						cx{ ((2 * pxc) / m_Width - 1) * aspectRatio * fov }, 
						cy{ (1 - (2 * pyc) / m_Height) * fov };
			
			Vector3 rayDirection{ cx * camera.right + cy * camera.up + camera.forward };
			rayDirection.Normalize();
			cameraONB.TransformVector(rayDirection);
			const Ray viewRay{ camera.origin, rayDirection};


			ColorRGB finalColor{};
			HitRecord closestHit{};

			pScene->GetClosestHit(viewRay, closestHit);
			
			//if a pixel is hit by viewRay
			if (closestHit.didHit)
			{
				//for each light in the scene
				for (const Light& light : pScene->GetLights())
				{
					Vector3 directionToLight{ LightUtils::GetDirectionToLight(light, closestHit.origin) };

					if (m_ShadowsEnabled)
					{
						//Check if point can see light
						Vector3 closestHitOriginOffset{ closestHit.origin + closestHit.normal * invtLightRayOffset };
						Ray invtLightRay{ closestHitOriginOffset, directionToLight.Normalized(), 0.0001f, directionToLight.Magnitude() };
						if (pScene->DoesHit(invtLightRay))
						{
							continue;
						}
					}
					
					//Calculate the observed area (Lambert's Cosine Law)
					float observerdArea{ Vector3::Dot(directionToLight.Normalized(), closestHit.normal) };

					//Calculate radiance of light
					ColorRGB radiance{ LightUtils::GetRadiance(light, closestHit.origin) };

					//Calculate BRDF
					ColorRGB BRDF{ materials[closestHit.materialIndex]->Shade(closestHit, directionToLight.Normalized(), -rayDirection) };

					switch (m_CurrentLightingMode)
					{
					case LightingMode::ObservedArea:
						//Values below zero point away from light, so not needed
						if (observerdArea >= 0)
						{
							finalColor += {observerdArea, observerdArea, observerdArea};
						}
						break;
					case LightingMode::Radiance:
						finalColor += radiance;
						break;
					case LightingMode::BRDF:
						finalColor += BRDF;
						break;
					case LightingMode::Combined:
						if (observerdArea >= 0)
						{
							finalColor += radiance * BRDF * observerdArea;
						}
						break;
					}
				}
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255)); 
		}
	}
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera,
	const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	const int px = pixelIndex % m_Width;
	const int py = pixelIndex / m_Width;
}

//void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera,
//	const::std::vector<Light>& lights, const::std::vector<Material*>& material) const
//{
	//copy code from for-loop
	//const int px = pixelIndex % m_Width;
	//const int py = pixelIndex / m_Width;
//}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::Update()
{
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	if (pKeyboardState[SDL_SCANCODE_F2])
	{
		ToggleShadows();
	}

	if (pKeyboardState[SDL_SCANCODE_F3])
	{
		CycleLightingMode();
	}
}

void Renderer::CycleLightingMode()
{
	if (m_CurrentLightingMode == LightingMode::Combined)
	{
		m_CurrentLightingMode = LightingMode::ObservedArea;
		std::cout << "ObservedArea\n";
	}
	else if (m_CurrentLightingMode == LightingMode::ObservedArea)
	{
		m_CurrentLightingMode = LightingMode::Radiance;
		std::cout << "Radiance\n";
	}
	else if (m_CurrentLightingMode == LightingMode::Radiance)
	{
		m_CurrentLightingMode = LightingMode::BRDF;
		std::cout << "BRDF\n";
	}
	else if (m_CurrentLightingMode == LightingMode::BRDF)
	{
		m_CurrentLightingMode = LightingMode::Combined;
		std::cout << "Combined\n";
	}
}


