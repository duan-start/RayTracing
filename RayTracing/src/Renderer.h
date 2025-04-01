#pragma once
#include "Walnut/Image.h"
#include <memory>
#include <glm/glm.hpp>
#include "Camera.h"
#include "Ray.h"

#include "Scene.h"

class Renderer {
	public:
		struct Settings {
			bool Accmulate = false;
		};

	public:
		Renderer() = default;

		void Render(const Camera& camera,const Scene& scene);

		void OneResize(uint32_t width,uint32_t height);

		void ResetFrameIndex() { m_FrameIndex = 1; }

		const std::shared_ptr<Walnut::Image>& GetFinalImage()const {
			return m_FinalImage	;
		}

		Settings& GetSetting() { return m_Setting; }

	private:
		struct HitPayload {
			float HitDistance=-1;
			glm::vec3 Position{0.f,0.f,0.f};
			glm::vec3 WorldNormal{0.f,0.f,0.f};

			int HitObjectIndex;
		
		};
	private:
		glm::vec4 Perpixel(uint32_t x,uint32_t y);//RayGen
		HitPayload TraceRay(const Ray& ray);
		HitPayload Miss(const Ray& ray);
		HitPayload ClosestHit(const Ray& ray,float hitDistance,int objectId);

	private:
		std::shared_ptr<Walnut::Image> m_FinalImage;
		uint32_t* m_ImageData = nullptr;
		glm::vec4* m_AccumulationData = nullptr;

		int m_FrameIndex = 1;

		const Camera* m_ActiveCamera;
		const Scene* m_ActiveScene;
		Settings m_Setting;
};