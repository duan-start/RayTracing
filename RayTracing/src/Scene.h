#pragma once
#include <glm/glm.hpp>
#include <vector>


struct Material {
    // ========== 基础颜色 ==========
    glm::vec3 albedo{ 1.0f };              // 也常叫 baseColor
    // glm::vec3 baseColor{1.0f};        // 更现代的命名（推荐）

    // ========== 金属-粗糙度工作流（目前最主流）==========
    float metallic = 0.0f;              // 0=绝缘体(dielectric)，1=金属
    float roughness = 0.5f;              // 0=镜面，1=完全漫反射

    // ========== 自发光 ==========
    glm::vec3 emission{ 0.0f };            // 直接存 HDR 强度（单位：尼特或 cd/m²）
    // 不再需要 EmissionPower！直接 emission * exposure 就行
    float exposure;

    // ========== 高级/扩展参数（99% 的 PBR 引擎都有）==========
    float ao = 1.0f;                     // Ambient Occlusion 强度（常来自 AO 贴图）
    float alpha = 1.0f;                  // 不透明度（用于透明/半透明材质）
    float alphaCutoff = 0.0f;            // Alpha Test 阈值（用于树叶、栅栏等）

    float ior = 1.5f;                    // 折射率 Index of Refraction（用于玻璃、水）
    float transmission = 0.0f;          // 透射率 0~1（0=不透明，1=完全透射）
    float transmissionRoughness = 0.0f;  // 透射粗糙度（毛玻璃）

    // 可选：Sheen（布料）
    float sheen = 0.0f;
    float sheenRoughness = 0.0f;

    // ========== 纹理指针（真实项目必备）==========
    //Texture* albedoMap = nullptr;
    //Texture* normalMap = nullptr;
    //Texture* metallicMap = nullptr;
    //Texture* roughnessMap = nullptr;
    //Texture* aoMap = nullptr;
    //Texture* emissionMap = nullptr;
    //Texture* ormMap = nullptr;    // 常用打包：R=AO, G=Roughness, B=Metallic

    // ========== 工具函数 ==========
    glm::vec3 getEmission() const { return emission; }

    // 判断是否发光（加速结构优化用）
    bool isEmissive() const { return glm::length(emission) > 0.001f; }

    // 判断是否透明/半透明
    bool isTransparent() const { return alpha < 0.99f || transmission > 0.01f; }
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