#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"

#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"
#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;
class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		:m_Camera(45.f)
	{
	
		Material& pink = m_Scene.Materials.emplace_back();
		pink.Albedo = { 1.f,0.f,1.f };

		Material& ground = m_Scene.Materials.emplace_back();
		ground.Albedo = { 0.85f,0.75f,0.4f };

		Material& Orange = m_Scene.Materials.emplace_back();
		Orange.Albedo = { 0.8f,0.5f,0.2f };
		Orange.EmssionColor = Orange.Albedo;
		Orange.EmssionPower = 2.f;

		Material& mirror = m_Scene.Materials.emplace_back();
		mirror.Albedo = { 0.4f,0.6f,0.8f };
		mirror.Roughness = 0.f;


		{
			Sphere sphere;
			sphere.Position = { -1.2f,-0.6f,-4.f };
			sphere.Radius = .6f;
			sphere.MaterialIndex = 0;
			m_Scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 8.f,1.8f,-11.5f };
			sphere.Radius = 7.3f;
			sphere.MaterialIndex = 2;
			m_Scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 2.f,-0.7f,-4.f };
			sphere.Radius = 0.5f;
			sphere.MaterialIndex = 3;
			m_Scene.Spheres.push_back(sphere);
		}


		{
			Sphere sphere;
			sphere.Position = { 0.f,-101.0f,-10.0f };
			sphere.Radius = 100.f;
			sphere.MaterialIndex = 1;
			m_Scene.Spheres.push_back(sphere);
		}
		
	};

	void Render() {
		Timer timer;
		m_Renderer.OneResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Camera,m_Scene);
		
		m_LastRenderTime = timer.ElapsedMillis();
	}

	virtual void OnUpdate(float ts)override {
		if (m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();
			
	}
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last RenderTime:%.3fms", m_LastRenderTime);
		if (ImGui::Button("Render")) {
			//一定要先拿到图像的信息才可以，这里保证不会在image之前使用
			Render();
		}

		ImGui::Checkbox("Accmulate", &m_Renderer.GetSetting().Accmulate);
		ImGui::Checkbox("SlowRandom", &m_Renderer.GetSetting().SlowRandom);

		if (ImGui::Button("Reset"))
			m_Renderer.ResetFrameIndex();

		ImGui::End();

		ImGui::Begin("Scene");

		ImGui::ColorEdit3("SkrColor", glm::value_ptr(m_Scene.SkyColor), .1f);
		ImGui::Separator();

		for (size_t i = 0; i < m_Scene.Spheres.size(); i++) {
			ImGui::PushID(i);
			
			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
			ImGui::DragInt("Material", &sphere.MaterialIndex, 1, 0, (int)m_Scene.Materials.size() - 1);
			ImGui::Separator();
			ImGui::PopID();
		}
		for (size_t i = 0; i < m_Scene.Materials.size(); i++) {
			ImGui::PushID(i);

			Material& material = m_Scene.Materials[i];

			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo),.1f);
			ImGui::DragFloat("Metalic", &material.Metalic, 0.05f, 0.f, 1.f);
			ImGui::DragFloat("Roughness", &material.Roughness, 0.05f, 0.f, 1.f);
			ImGui::DragFloat("Emssion Power", &material.EmssionPower, .05f,0.0f,FLT_MAX);
			ImGui::ColorEdit3("Emssion Color", glm::value_ptr(material.EmssionColor), .1f);

			ImGui::Separator();
			ImGui::PopID();
		}


		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if (image) {
		ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(),(float)image->GetHeight() },{0,1},{1,0});
		}
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::ShowDemoWindow();
		Render();
	}
private:
	uint32_t* m_ImageData=nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0;
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Walnut Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}