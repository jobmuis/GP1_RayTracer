#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include <iostream>

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			Vector3 worldUp{ Vector3::UnitY };
			right = Vector3::Cross(worldUp, forward);
			right.Normalize();
			up = Vector3::Cross(forward, right);
			up.Normalize();
			//reverse camera
			//forward.z = -1.f;

			cameraToWorld = { right, up, forward, origin };
			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			const float moveSpeed{ 5.f };

			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * moveSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * moveSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * moveSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * moveSpeed * deltaTime;
			}


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			const float rotationSpeed{ 5.f };

			if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				totalPitch = mouseY * rotationSpeed * deltaTime;
				totalYaw = mouseX * rotationSpeed * deltaTime;
			}

			//std::cout << totalYaw << '\n';

			Matrix finalRotation{ Matrix::CreateRotationY(totalYaw) * Matrix::CreateRotationX(totalPitch) };

			forward = finalRotation.TransformVector(forward);
			forward.Normalize();
		}
	};
}
