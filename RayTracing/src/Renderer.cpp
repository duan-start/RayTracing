#include "Renderer.h"
#include "Walnut/Random.h"

#include <iostream>

namespace WorkUtils {
	static uint32_t utils(const glm::vec4& color) {
		uint8_t r = 255*color.r;
		uint8_t g = 255*color.g;
		uint8_t b = 255*color.b;
		uint8_t a = 255*color.a;
		return 0x00000000 | (a << 24) | (b << 16) | (g << 8) | (r);
	}
}

void Renderer::Render(const Camera& camera, const Scene& scene)
{
	m_ActiveCamera = &camera;
	m_ActiveScene = &scene;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

	//�������е�����
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
			//�ĸ��ֽڣ���ʮ��λ������������rgba �ĸ�ʽ
			//��������һ�����£���Χ����0->1
			glm::vec4 color = Perpixel(x, y);

			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;
			glm::vec4 accumulateColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			//��һ��ƽ��
			accumulateColor /=(float) m_FrameIndex;

			 accumulateColor = glm::clamp(accumulateColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] =WorkUtils::utils(accumulateColor);


			//glm::vec4 testColor = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			//m_ImageData[x + y * m_FinalImage->GetWidth()] = WorkUtils::utils(testColor);
		}
	}
	m_FinalImage->SetData(m_ImageData);
	if (m_Setting.Accmulate)
		m_FrameIndex++;
	else
	{
		m_FrameIndex = 1;
	}


}

void Renderer::OneResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage) {
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);
	}
	else {
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	
	//���Զ��жϿ�ָ�����������ᱨ��
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];
	
}


//����׷��
glm::vec4 Renderer::Perpixel( uint32_t x, uint32_t y)
{

	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	//Ϊʲô����û��Ҳ��������ˣ�������������ɫ��
	// ͸��ͶӰ�ľ����ǵĿ�߱�
	//��֤���������������һ�µ�,��������
	//x *= aspectradio;
	ray.Direction = m_ActiveCamera->GetRayDirection()[x + y * m_FinalImage->GetWidth()];

	int bounce = 5;
	float multiplier = 1.0f;
	glm::vec3 color(0.f);
	for (int i = 0; i < bounce; i++) {
		Renderer::HitPayload& payload = TraceRay(ray);
		if (payload.HitDistance < 0) {
			glm::vec3 skyColor={ 0.6f,0.7f,0.9f };//��յ���ɫ
			color += skyColor * multiplier;//�������Լ���ao֮���
			break;
		}
			
		
		//ÿ�����ص���ɫ���������壩
		glm::vec3 lightDir = glm::normalize(glm::vec3(-1.f, -1.f, -1.f));
		float lightIntensity = std::max(0.f, glm::dot(payload.WorldNormal, -lightDir));

		const Sphere& sphere = m_ActiveScene->Spheres[payload.HitObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

		glm::vec3 sphereColor = lightIntensity * (material.Albedo);
		
		color += sphereColor * multiplier;
		multiplier *= 0.5f;
	
		ray.Origin = payload.Position + payload.WorldNormal* (float)1e-5 ;
		//���ڴֲڶȵ�ԭ������Ҫ���з��ߵ�ƫ�ƣ�΢�����ԭ��
		ray.Direction = glm::reflect(ray.Direction,payload.WorldNormal+ material.Roughness*Walnut::Random::Vec3(-0.5f,0.5f));
	}

	//glm::vec3 ambient(0.2f);
	return{ color ,1.f };

}

//����ǹؼ��Ĵ����ˣ��������з����Ĺ��ߣ��ֱ�������е����壬������̵ľ�������������
  Renderer::HitPayload Renderer::TraceRay(const Ray & ray)
{

	float hitdistance = std::numeric_limits<float>::max();
	int colosetSphere = -1;

	for (int i = 0; i < m_ActiveScene->Spheres.size();i++) {
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		float radius = sphere.Radius;
		glm::vec3 origin = ray.Origin - sphere.Position;
		//���Ĳ�������,���η���
		//first=bx^2+by^2+bz^2
		//seconf=2(ax*bx+ayby+)
		//third= ax^2+ay^2+a^2-r^2
		//a-ray origin
		//b-ray dir
		//c-r
		//t-distance ,what we need to get
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2 * (glm::dot(origin, ray.Direction));
		float c = glm::dot(origin,origin) - radius * radius;
		//b^2-4ac
		float disc = b * b - 4. * a * c;

		if (disc<0)
			continue;

		//-b+sprt(disc);
		float closestT = (-b - sqrt(disc)) /( 2.f * a);
		//float to=(-b+sqrt(disc))/2*a;
		//����͵���Ĺ�ϵ�һ�û����
		if (closestT>0.0f && closestT < hitdistance) {
			hitdistance = closestT;
			colosetSphere = (int)i;
			}
		}
	
		if (colosetSphere < 0)
			return Miss(ray);

		return ClosestHit(ray, hitdistance, colosetSphere);

	}

  Renderer::HitPayload Renderer::Miss(const Ray& ray)
  {
	 // HitPayload m_miss_payload;
	  return HitPayload();
  }

  Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitdistance, int objectId)
  {
	  const Sphere& sphere = m_ActiveScene->Spheres[objectId];
	  HitPayload payload;
	  payload.HitDistance = hitdistance;
	  payload.HitObjectIndex = objectId;
  ;
	  glm::vec3 origin = ray.Origin - sphere.Position;
	  payload.Position = origin + ray.Direction * hitdistance;
	  payload.WorldNormal = glm::normalize(payload.Position);
	  payload.Position = payload.Position + sphere.Position;
	  //glm::vec3 lightdir = m_LightPosition;
	  //lightdir = glm::normalize(lightdir);
	
	  return payload;
  }

	




