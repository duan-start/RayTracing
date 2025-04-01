#pragma once

#include "Walnut/Image.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

class Camera
{
public:
	Camera(float verticalFOV , float nearClip = 0.1, float farClip = 100.f);

	//与帧率无关的速度
	bool OnUpdate(float ts);
	void OnResize(uint32_t width, uint32_t height);

	float GetRotationSpeed();

	void SetProjection(const glm::vec4& screen);
	void SetRotation(const glm::vec4& rotation);
	void SetPosition(const glm::vec3& position);

	const glm::vec3& GetPosition()const { return m_Position; }
	const glm::vec4& GetRotation()const { return m_Rotation; }

	//真就是光追特有
	const std::vector<glm::vec3>& GetRayDirection()const { return m_RayDirection; }

	const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
	const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
	const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
	const glm::mat4& GetInverseView() const { return m_InverseView; }
private:
	void RecalculateProjection();
	void RecalculateRayDirection();
	void RecalculateViewMatrix();
private:
	//主要是这个后续经常要取出来，所以就这样写
	glm::mat4 m_ProjectionMatrix{ 1.0f };
	glm::mat4 m_InverseProjection{ 1.0f };
	glm::mat4 m_ViewMatrix{ 1.0f };
	glm::mat4 m_InverseView{ 1.0f };
	glm::mat4 m_ViewProjectionMatrix{ 1.0f };

	float m_VerticalFov = 45.f;
	float m_NearClip = 0.1f;
	float m_FarClip = 100.f;

	//初始话默认值直接扔这里不就好了，除去可读性，这样更好
	glm::vec3 m_Position{ 0.f, 0.f, 3.f };
	glm::vec3 m_ForwardDirection{ 0.f,0.f,-1.f };
	glm::vec4 m_Rotation{0.f, 0.f, 1.0f, 0.f};

	std::vector<glm::vec3> m_RayDirection;

	glm::vec2 m_LastMousePosition{ 0.f,0.f };
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

};





