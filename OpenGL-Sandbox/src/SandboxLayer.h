#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

class SandboxLayer : public GLCore::Layer
{
public:
	SandboxLayer();
	virtual ~SandboxLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
private:
    std::unique_ptr<GLCore::Utils::Shader> m_Shader;
    GLCore::Utils::OrthographicCameraController m_CameraController;
    GLuint m_QuadVA, m_QuadVB, m_QuadIB;
	GLuint m_Cpp, m_CppLogo;
	float m_QuadPosition[2] = { -1.5f, -0.5f };

};