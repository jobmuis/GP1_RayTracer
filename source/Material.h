#pragma once
#include "Math.h"
#include "DataTypes.h"
#include "BRDFs.h"

namespace dae
{
#pragma region Material BASE
	class Material
	{
	public:
		Material() = default;
		virtual ~Material() = default;

		Material(const Material&) = delete;
		Material(Material&&) noexcept = delete;
		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&) noexcept = delete;

		/**
		 * \brief Function used to calculate the correct color for the specific material and its parameters
		 * \param hitRecord current hitrecord
		 * \param l light direction
		 * \param v view direction
		 * \return color
		 */
		virtual ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) = 0;
	};
#pragma endregion

#pragma region Material SOLID COLOR
	//SOLID COLOR
	//===========
	class Material_SolidColor final : public Material
	{
	public:
		Material_SolidColor(const ColorRGB& color): m_Color(color)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord, const Vector3& l, const Vector3& v) override
		{
			return m_Color;
		}

	private:
		ColorRGB m_Color{colors::White};
	};
#pragma endregion

#pragma region Material LAMBERT
	//LAMBERT
	//=======
	class Material_Lambert final : public Material
	{
	public:
		Material_Lambert(const ColorRGB& diffuseColor, float diffuseReflectance) :
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(diffuseReflectance){}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			return BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor);
		}

	private:
		ColorRGB m_DiffuseColor{colors::White};
		float m_DiffuseReflectance{1.f}; //kd
	};
#pragma endregion

#pragma region Material LAMBERT PHONG
	//LAMBERT-PHONG
	//=============
	class Material_LambertPhong final : public Material
	{
	public:
		Material_LambertPhong(const ColorRGB& diffuseColor, float kd, float ks, float phongExponent):
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(kd), m_SpecularReflectance(ks),
			m_PhongExponent(phongExponent)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			return BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor) 
				+ BRDF::Phong(m_SpecularReflectance, m_PhongExponent, -l, v, hitRecord.normal);
		}

	private:
		ColorRGB m_DiffuseColor{colors::White};
		float m_DiffuseReflectance{0.5f}; //kd
		float m_SpecularReflectance{0.5f}; //ks
		float m_PhongExponent{1.f}; //Phong Exponent
	};
#pragma endregion

#pragma region Material COOK TORRENCE
	//COOK TORRENCE
	class Material_CookTorrence final : public Material
	{
	public:
		Material_CookTorrence(const ColorRGB& albedo, float metalness, float roughness):
			m_Albedo(albedo), m_Metalness(metalness), m_Roughness(roughness)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			ColorRGB baseReflectivityF0{};
			if (m_Metalness == 0)
			{
				baseReflectivityF0 = ColorRGB(0.04f, 0.04f, 0.04f);
			}
			else
			{
				baseReflectivityF0 = m_Albedo;
			}

			Vector3 viewAndLightAdded{ l + v };
			Vector3 halfVector{ viewAndLightAdded / viewAndLightAdded.Magnitude()};
			//ColorRGB ks{ BRDF::FresnelFunction_Schlick(halfVector, v, baseReflectivityF0) };

			// F - Fresnel function
			//return BRDF::FresnelFunction_Schlick(halfVector.Normalized(), v, baseReflectivityF0);
			ColorRGB Fresnel{ BRDF::FresnelFunction_Schlick(halfVector.Normalized(), v, baseReflectivityF0) };

			// D - Normal distribution
			//return ColorRGB{ BRDF::NormalDistribution_GGX(hitRecord.normal, halfVector.Normalized(), m_Roughness),
			//				 BRDF::NormalDistribution_GGX(hitRecord.normal, halfVector.Normalized(), m_Roughness),
			//				 BRDF::NormalDistribution_GGX(hitRecord.normal, halfVector.Normalized(), m_Roughness) };
			float NormalDistribution{ BRDF::NormalDistribution_GGX(hitRecord.normal, halfVector.Normalized(), m_Roughness) };

			// G - Geometry function
			//return ColorRGB{ BRDF::GeometryFunction_Smith(hitRecord.normal, v, l, m_Roughness),
			//				 BRDF::GeometryFunction_Smith(hitRecord.normal, v, l, m_Roughness),
			//				 BRDF::GeometryFunction_Smith(hitRecord.normal, v, l, m_Roughness) };
			float Geometry{ BRDF::GeometryFunction_Smith(hitRecord.normal, v, l, m_Roughness) };

			ColorRGB specularCookTorrance;
			specularCookTorrance = (Fresnel * NormalDistribution * Geometry)
				/ (4.f * Vector3::Dot(v, hitRecord.normal) * Vector3::Dot(l, hitRecord.normal));
			
			ColorRGB kd{};
			if (m_Metalness == 0)
			{
				kd = ColorRGB{1.f, 1.f, 1.f} - Fresnel;
			}
			else
			{
				kd = ColorRGB{ 0, 0, 0 };
			}
			ColorRGB diffuseLambert{ BRDF::Lambert(kd, m_Albedo) };
			//return diffuseLambert;

			return ColorRGB{ diffuseLambert + specularCookTorrance };

		}

	private:
		ColorRGB m_Albedo{0.955f, 0.637f, 0.538f}; //Copper
		float m_Metalness{1.0f};
		float m_Roughness{0.1f}; // [1.0 > 0.0] >> [ROUGH > SMOOTH]
	};
#pragma endregion
}
