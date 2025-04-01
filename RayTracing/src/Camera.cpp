#include "Camera.h"
#include "Walnut/Input/Input.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp>

using namespace Walnut;

Camera::Camera(float verticalFOV, float nearClip , float farClip ):
	m_VerticalFov(verticalFOV),m_NearClip(nearClip),m_FarClip(farClip)
{
}

//�ؼ���tick����������������������
bool Camera::OnUpdate(float ts)
{
	//�����Ϣ
	glm::vec2 mousePos = Input::GetMousePosition();
	//��������������
	glm::vec2 delta = (mousePos - m_LastMousePosition) * 0.002f;
	m_LastMousePosition = mousePos;

	if(!Input::IsMouseButtonDown(MouseButton::Right)) {
		Input::SetCursorMode(CursorMode::Normal);
		return false;
	}
	Input::SetCursorMode(CursorMode::Locked);
	bool moved = false;
	constexpr glm::vec3 upDirection(0.f, 1.f, 0.f);
	glm::vec3 rightDirection = glm::cross(m_ForwardDirection, upDirection);
	float speed = 4.f;

	//���if-else ����������ȼ������ж�
	if (Input::IsKeyDown(KeyCode::W)) {
		m_Position += m_ForwardDirection * ts * speed;
		moved = true;
	}
	else if (Input::IsKeyDown(KeyCode::S)) {
		m_Position -= m_ForwardDirection * ts * speed;
		moved = true;
	}

	if (Input::IsKeyDown(KeyCode::D)) {
		m_Position += rightDirection * ts * speed;
		moved = true;
	}
	else if (Input::IsKeyDown(KeyCode::A)) {
		m_Position -= rightDirection * ts * speed;
		moved = true;
	}
	
	if (Input::IsKeyDown(KeyCode::E)) {
		m_Position += upDirection * ts * speed;
		moved = true;
	}
	else if (Input::IsKeyDown(KeyCode::Q)) {
		m_Position -= upDirection * ts * speed;
		moved = true;
	}

	//ʹ����Ԫ����������������ת
	if (delta.x != 0.f || delta.y != 0.f) {
		float pitchDelta = delta.y * GetRotationSpeed();
		float yawDelta = delta.x * GetRotationSpeed();

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
			glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.f))));
		m_ForwardDirection = glm::rotate(q, m_ForwardDirection);
		moved = true;
	}
	if (moved) {
		RecalculateViewMatrix();
		RecalculateRayDirection();
	}
	return moved;
	//ʹ��ŷ��������������������ת����������
}

void Camera::OnResize(uint32_t width, uint32_t height)
{
	if (width == m_ViewportWidth && height == m_ViewportHeight)
		return;
	m_ViewportWidth = width;
	m_ViewportHeight = height;

	RecalculateProjection();
	RecalculateRayDirection();
}

float Camera::GetRotationSpeed()
{
	return 0.3f;
}


void Camera::SetProjection(const glm::vec4& screen)
{
	//m_ProjectionMatrix = (glm::perspective(screen.x, screen.y, screen.z, screen.w));
	//RecalculateViewMatrix();
}
void Camera::SetRotation(const glm::vec4& rotation)
{
	//m_Rotation = rotation;
	//RecalculateViewMatrix();
}
void Camera::SetPosition(const glm::vec3& position)
{
	//m_Position = position;
	//RecalculateViewMatrix();
}


void Camera::RecalculateProjection()
{
	m_ProjectionMatrix = glm::perspectiveFov(glm::radians(m_VerticalFov),(float) m_ViewportWidth,(float) m_ViewportHeight, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_ProjectionMatrix);
}

void Camera::RecalculateRayDirection()
{
	m_RayDirection.resize(m_ViewportWidth * m_ViewportHeight);
	for (uint32_t y = 0; y < m_ViewportHeight; y++) {
		for (uint32_t x = 0; x < m_ViewportWidth; x++) {
			//����Ļ�ռ������ת���ɱ�׼�ü��ռ������
			//Ȼ����ݱ�׼�Ĳü��ռ������ȥ������Ӧ�Ĺ���
			glm::vec2 coord = { (float)x / (float)m_ViewportWidth,(float)y / (float)m_ViewportHeight };
			coord = coord * 2.f - 1.0f;//-1->1

			//������������Ŀռ�λ��
			//��Բü��ռ��һ���ض���ƽ�淢���Ĺ��ߣ�һ��ѡ��Զ�ģ���һ�����������������λ��תΪ������������α��
			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1.f, 1.f);

			//תΪ���������ı�׼��Ȼ��ת����������ı�׼���
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0));
			//���������ÿһ�����ߵľ�����
			m_RayDirection[x + y * m_ViewportWidth] = rayDirection;
		}
	}
}

void Camera::RecalculateViewMatrix()
{
	////�������,��������������������λ�ã�����Ҫ���棬����������ת��ƽ�ƣ���������
	//glm::mat4 transform = glm::translate(glm::mat4(1.0), m_Position) * glm::rotate(glm::mat4(1.0), glm::radians(m_Rotation.w), glm::vec3(m_Rotation.x, m_Rotation.y, m_Rotation.z));

	//m_ViewMatrix = glm::inverse(transform);
	//m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

	//�����Ч��
	m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
	//����
	m_InverseView = glm::inverse(m_ViewMatrix);
}