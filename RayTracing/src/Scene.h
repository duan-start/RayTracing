#pragma once
#include <glm/glm.hpp>
#include <vector>


struct Material {
	glm::vec3 Albedo{1.f,1.f,1.f};
	float Roughness=.1f;
	float Metalic=0.f;

};

struct Sphere {
	glm::vec3 Position{ 0.f,0.f,0.f };
	float Radius{ 0.5f };
	int MaterialIndex = 0;
};

struct Scene {
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;
};