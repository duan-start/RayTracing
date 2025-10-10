#pragma once
#include <glm/glm.hpp>
#include <vector>


struct Material {
	//Todo:: PBR
	glm::vec3 Albedo{1.f,1.f,1.f};
	float Roughness=.1f;
	float Metalic=0.f;
	glm::vec3 EmssionColor{ 0.f,0.f,0.f };
	float EmssionPower = 0.f;

	const glm::vec3 GetEmssion()const { return EmssionPower * EmssionColor; }
};

struct Sphere {
	glm::vec3 Position{ 0.f,0.f,0.f };
	float Radius{ 0.5f };
	int MaterialIndex = 0;
};

struct Scene {
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;
	glm::vec3 SkyColor{ 0.5f,0.6f,0.8f };
};