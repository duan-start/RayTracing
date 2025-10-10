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

//关键的tick函数，放在组件里面会更灵活
bool Camera::OnUpdate(float ts)
{
	//鼠标信息
	glm::vec2 mousePos = Input::GetMousePosition();
	//设置鼠标的灵敏度
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

	//这个if-else 代表的是优先级，先判断
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

	//使用四元数计算鼠标带来的旋转
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
	//使用欧拉角来计算鼠标带来的旋转（万向锁）
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
			//将屏幕空间的数据转化成标准裁剪空间里面的
			//然后根据标准的裁剪空间的数据去发出对应的光线
			glm::vec2 coord = { (float)x / (float)m_ViewportWidth,(float)y / (float)m_ViewportHeight };
			coord = coord * 2.f - 1.0f;//-1->1

			//返回世界坐标的空间位置
			//针对裁剪空间的一个特定的平面发出的光线（一般选择远的，这一不会出错），将这个点的位置转为摄像机坐标的齐次表达
			//四维的齐次空间
			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1.f, 1.f);

			//转为摄像机坐标的标准表达，然后转成世界坐标的标准表达
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0));
			//世界坐标的每一根光线的具体表达
			m_RayDirection[x + y * m_ViewportWidth] = rayDirection;
		}
	}
}

void Camera::RecalculateViewMatrix()
{
	////正面计算,由于这边是设置摄像机的位置，后面要做逆，所以是先旋转后平移，从右自左
	//glm::mat4 transform = glm::translate(glm::mat4(1.0), m_Position) * glm::rotate(glm::mat4(1.0), glm::radians(m_Rotation.w), glm::vec3(m_Rotation.x, m_Rotation.y, m_Rotation.z));

	//m_ViewMatrix = glm::inverse(transform);
	//m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;

	//正向的效果
	m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
	//求逆
	m_InverseView = glm::inverse(m_ViewMatrix);
}