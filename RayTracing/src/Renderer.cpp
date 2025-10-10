#include "Renderer.h"
#include "Walnut/Random.h"

#include <execution>
#include <iostream>

namespace WorkUtils {
	static uint32_t utils(const glm::vec4& color) {
		glm::vec3 col=glm::pow((glm::vec3)color, glm::vec3(1.0f / 2.2f));
		uint8_t r = 255*col.r;
		uint8_t g = 255*col.g;
		uint8_t b = 255*col.b;
		uint8_t a = 255*color.a;
		return 0x00000000 | (a << 24) | (b << 16) | (g << 8) | (r);
	}

	//伪随机数的生成，加速
	static uint32_t PCG_Hash(uint32_t input) {
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	//由于并行实现的参数化
	static float RandomFloat(uint32_t& seed) {
		seed = PCG_Hash(seed);
		return (float)seed / (float)std::numeric_limits<uint32_t>::max();
	}

	static glm::vec3 InUnitSphere(uint32_t& seed)
	{
		return glm::vec3(RandomFloat(seed)*2.0f-1.0f,
			RandomFloat(seed) * 2.0f - 1.0f,
			RandomFloat(seed) * 2.0f - 1.0f);
	}
}

void Renderer::Render(const Camera& camera, const Scene& scene)
{
	m_ActiveCamera = &camera;
	m_ActiveScene = &scene;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

	//遍历所有的像素

	//调用多线程
#define MT 1

#if  MT
	std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(), [this](uint32_t y) {

#if 0	//为每一个像素分配一个线程
		std::for_each(std::execution::par, m_ImageHorizonalIter.begin(), m_ImageHorizonalIter.end(), [this, y](uint32_t x) {

			glm::vec4 color = Perpixel(x, y);

			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;
			glm::vec4 accumulateColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			//做一个平均
			accumulateColor /= (float)m_FrameIndex;

			accumulateColor = glm::clamp(accumulateColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = WorkUtils::utils(accumulateColor);

			});

	});
#else  //为每一行分配一个线程，硬件的性能都达到了顶峰，编译器会自动控制，不用管具体的
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
			//四个字节，三十二位，从右往左是rgba 的格式
			//现在做了一个更新，范围是在0->1
			glm::vec4 color = Perpixel(x, y);

			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;
			glm::vec4 accumulateColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			//做一个平均
			accumulateColor /= (float)m_FrameIndex;


			accumulateColor = glm::clamp(accumulateColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = WorkUtils::utils(accumulateColor);
			}
	});

#endif 


#else

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++) {
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
			//四个字节，三十二位，从右往左是rgba 的格式
			//现在做了一个更新，范围是在0->1
			glm::vec4 color = Perpixel(x, y);

			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;
			glm::vec4 accumulateColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			//做一个平均
			accumulateColor /=(float) m_FrameIndex;

			 accumulateColor = glm::clamp(accumulateColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] =WorkUtils::utils(accumulateColor);

			//glm::vec4 testColor = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			//m_ImageData[x + y * m_FinalImage->GetWidth()] = WorkUtils::utils(testColor);
		}
	}
#endif

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
	
	//会自动判断空指针的情况，不会报错
	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];
	
	m_ImageHorizonalIter.resize(width);
	m_ImageVerticalIter.resize(height);
	for (int i = 0; i < width; i++)
		m_ImageHorizonalIter[i] = i;
	for (int i = 0; i < height; i++)
		m_ImageVerticalIter[i] = i;
}


//光线追踪
glm::vec4 Renderer::Perpixel( uint32_t x, uint32_t y)
{

	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	//为什么这里没有也不会变形了，这里是像素着色，
	// 透视投影的矩阵考虑的宽高比
	//保证光线射出的数量是一致的,横纵坐标
	//x *= aspectradio;
	ray.Direction = m_ActiveCamera->GetRayDirection()[x + y * m_FinalImage->GetWidth()];

	//创建种子
	uint32_t seed = x + y * m_FinalImage->GetWidth();
	seed *= m_FrameIndex;

	int bounce = 5;
	glm::vec3 contribution{ 1.0f };
	glm::vec3 light(0.f);
	for (int i = 0; i < bounce; i++) {
		seed += i;

        const Renderer::HitPayload& payload = TraceRay(ray);
		if (payload.HitDistance < 0) {
		//	glm::vec3 skyColor={ 0.3f,0.3f,0.45f };//天空的颜色
			light += m_ActiveScene->SkyColor * contribution;//后续可以加上ao之类的
			
			break;
		}
			
		const Sphere& sphere = m_ActiveScene->Spheres[payload.HitObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];	

		//每个像素的着色（击中球体）
		// 定向光源
	//	glm::vec3 lightDir = glm::normalize(glm::vec3(-1.f, -1.f, -1.f));
	//	float lightIntensity = std::max(0.f, glm::dot(payload.WorldNormal, -lightDir));
	//	glm::vec3 sphereColor = lightIntensity * (material.Albedo);
	//	color += sphereColor * multiplier;

		//light += material.Albedo*contribution;
		//contribution *= 0.5f;

		
		//光线经过一次bounce剩下的能量和发光物体加重
		contribution *= material.Albedo;//diffuse下的颜色
		light += material.GetEmssion();//这个是发出的光线
		
		//ToDo:PBR
		//由于既有折射又有反射的部分，不可能一根光线充当两个的作用，所有这里根据权重和概率随机选择
		float diffuseWeight = 1.0f - material.Metalic; // 非金属更多漫反射
		float specularWeight = material.Metalic;        // 金属优先镜面
		float sumWeight = diffuseWeight + specularWeight;


		ray.Origin = payload.Position + payload.WorldNormal * (float)1e-5;
		//由于粗糙度的原因，我需要进行法线的偏移，微表面的原因
		// 
	//	ray.Direction = glm::reflect(ray.Direction,payload.WorldNormal+ material.Roughness*Walnut::Random::Vec3(-0.5f,0.5f));

		float p = Walnut::Random::Float();
		
		if (p < diffuseWeight) {
			//这个是随机的反射，可以理解为漫反射的要求
			glm::vec3 inDir = ray.Direction;
			if (m_Setting.SlowRandom) {
				ray.Direction = glm::normalize(Walnut::Random::InUnitSphere() + payload.WorldNormal);
			}
			else {
				ray.Direction = glm::normalize(WorkUtils::InUnitSphere(seed) + payload.WorldNormal);
			}
			// 2. 计算 pdf
			//float pdf = glm::dot(ray.Direction, payload.WorldNormal) / 3.14;

			// 3. 贡献
			//glm::vec3 F0 = glm::mix(glm::vec3(0.04f), material.Albedo, material.Metalic);
			//glm::vec3 diffuseBRDF = material.Albedo / (glm::vec3) 3.14 * (1.0f - F0) * (1.0f - material.Metalic);
			//contribution *= diffuseBRDF * glm::dot(ray.Direction, payload.WorldNormal) / pdf;
		}
		else {
			// 1. 入射方向（指向表面）
			//glm::vec3 V = -ray.Direction;

			// 2. 理想镜面方向
			glm::vec3 idealReflect = glm::reflect(ray.Direction, payload.WorldNormal);

			// 3. 粗糙扰动（近似微表面）
			glm::vec3 roughReflect = glm::normalize(idealReflect + material.Roughness * WorkUtils::InUnitSphere(seed));

			// 4. 半程向量
			//glm::vec3 H = glm::normalize(V + roughReflect);

			// 5. Cook-Torrance BRDF 贡献
			//float NdotL = std::max(glm::dot(payload.WorldNormal, roughReflect), 0.0f);
			//??
			//contribution *= BRDF(V, roughReflect, payload.WorldNormal, material) ;

			// 6. 更新 ray 方向
			ray.Direction = roughReflect;



		}
		

	}

	//glm::vec3 ambient(0.2f);
	
	return { light, 1.0f };

}

//这个是关键的代码了，对于所有发出的光线，分别遍历所有的球体，更新最短的距离和最近的球体
  Renderer::HitPayload Renderer::TraceRay(const Ray & ray)
{

	float hitdistance = std::numeric_limits<float>::max();
	int colosetSphere = -1;

	for (int i = 0; i < m_ActiveScene->Spheres.size();i++) {
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		float radius = sphere.Radius;
		glm::vec3 origin = ray.Origin - sphere.Position;
		//求解的参数方程,二次方程
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
		//这里和弹射的关系我还没看懂
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

  glm::vec3 Renderer::BRDF(const glm::vec3& inDir, const glm::vec3& outDir, const glm::vec3& worldNormol, const Material& material)
  {
	  glm::vec3 V = outDir;       // 出射方向，指向观察者
	  glm::vec3 L = inDir;        // 入射方向，指向光源或采样方向
	  glm::vec3 H = glm::normalize(V + L);

	  float NdotL = std::max(glm::dot(worldNormol, L), 0.0f);
	  float NdotV = std::max(glm::dot(worldNormol, V), 0.0f);
	  float NdotH = std::max(glm::dot(worldNormol, H), 0.0f);
	  float VdotH = std::max(glm::dot(V, H), 0.0f);

	  // --- NDF: GGX ---
	  float alpha = std::max(material.Roughness, 0.001f);
	  float alpha2 = alpha * alpha;
	  float denom = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;
	  float D = alpha2 / (3.14* denom * denom);

	  // --- Fresnel ---
	  glm::vec3 F0 = glm::mix(glm::vec3(0.04f), material.Albedo, material.Metalic);
	  glm::vec3 F = F0 + (glm::vec3(1.0f) - F0) * pow(1.0f - VdotH, 5.0f);

	  // --- Geometry: Smith ---
	  auto G1 = [](float NdotX, float alpha) {
		  return 2.0f * NdotX / (NdotX + sqrt(alpha * alpha + (1.0f - alpha * alpha) * NdotX * NdotX));
		  };
	  float G = G1(NdotL, alpha) * G1(NdotV, alpha);

	  // --- Cook-Torrance specular ---
	  float denominator = 4.0f * NdotL * NdotV + 1e-5f;
	  glm::vec3 specular = (D * F * G) / denominator;

	  return specular;
	  
  }

  float Renderer::Distri(const glm::vec3& half, const glm::vec3& normal, float roughness)
  {
	  float alpha = roughness * roughness;
	  float NdotH = std::max(glm::dot(normal, half), 0.0f);
	  float denom = (NdotH * NdotH * (alpha * alpha - 1.0f) + 1.0f);
	  return (alpha * alpha) / (3.14 * denom * denom);
  }

  // ----------------------------
  // 2. F - Schlick Fresnel
  // ----------------------------
  glm::vec3 Renderer::Fer(const glm::vec3& inDir, const glm::vec3& half, const glm::vec3& Albedo)
  {
	  float VoH = std::max(glm::dot(inDir, half), 0.0f);
	  return Albedo + (glm::vec3(1.0f) - Albedo) * pow(1.0f - VoH, 5.0f);
  }

  // ----------------------------
  // 3. G - Geometry / Smith
  // ----------------------------
  float Renderer::G1(float NdotV, float roughness)
  {
	  float alpha = roughness * roughness;
	  return 2.0f * NdotV / (NdotV + sqrt(alpha + (1.0f - alpha) * NdotV * NdotV));
  }

  float Renderer::Gxx(const glm::vec3& inDir, const glm::vec3& outDir, const glm::vec3& normal, float roughness)
  {
	  float NdotL = std::max(glm::dot(normal, inDir), 0.0f);
	  float NdotV = std::max(glm::dot(normal, outDir), 0.0f);
	  return G1(NdotL, roughness) * G1(NdotV, roughness);
  }

	




