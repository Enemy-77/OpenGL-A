#include "SandboxLayer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

using namespace GLCore;
using namespace GLCore::Utils;

struct Vertex
{
    float Position[3];
    float Color[4];
    float TexCoord[2];
    float TexID;
};

SandboxLayer::SandboxLayer()
    : m_CameraController(16.0f / 9.0f)
{
}

SandboxLayer::~SandboxLayer()
{
}

static GLuint LoadTexture(const std::string& path)
{
    int w, h, bits;

    stbi_set_flip_vertically_on_load(1);
    auto* pixels = stbi_load(path.c_str(), &w, &h, &bits, STBI_rgb);
    GLuint textureID;
    glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    stbi_image_free(pixels);
    
    return textureID;
}

void SandboxLayer::OnAttach()
{
	EnableGLDebugging();

	// Init here
    m_Shader = std::unique_ptr<GLCore::Utils::Shader>(Shader::FromGLSLTextFiles(
        "assets/shaders/test.glsl.vert",
        "assets/shaders/test.glsl.frag"
    ));

    glUseProgram(m_Shader->GetRendererID());
    auto loc = glGetUniformLocation(m_Shader->GetRendererID(), "u_Textures");
    int samplers[2] = { 0, 1 };
    glUniform1iv(loc, 2, samplers);


    glClearColor(0.1f, 0.1f, 0.1f, 0.1f);

    /*
    float vertices[] = {
        -1.5f, -0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f, 1.0f, 1.0f, 0.0f,
        -1.5f,  0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f, 0.0f, 1.0f, 0.0f,

         0.5f, -0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f, 0.0f, 0.0f, 1.0f,
         1.5f, -0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f, 1.0f, 0.0f, 1.0f,
         1.5f,  0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f, 0.0f, 1.0f, 1.0f
    };*/

    glCreateVertexArrays(1, &m_QuadVA);
    glBindVertexArray(m_QuadVA);

    glCreateBuffers(1, &m_QuadVB);
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 1000, nullptr, GL_DYNAMIC_DRAW);


    glEnableVertexArrayAttrib(m_QuadVB, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));

    glEnableVertexArrayAttrib(m_QuadVB, 1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));

    glEnableVertexArrayAttrib(m_QuadVB, 2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoord));

    glEnableVertexArrayAttrib(m_QuadVB, 3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexID));


    uint32_t indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4
    };

    glCreateBuffers(1, &m_QuadIB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    m_Cpp = LoadTexture("assets/cpp.jpg");
    m_CppLogo = LoadTexture("assets/cpplogo.png");
}

void SandboxLayer::OnDetach()
{
	// Shutdown here
}

void SandboxLayer::OnEvent(Event& event)
{
	// Events here
    m_CameraController.OnEvent(event);
}

static void SetUniformMat4(uint32_t shader, const char* name, const glm::mat4& matrix)
{
    int loc = glGetUniformLocation(shader, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
}

static void SetUniformVec4(uint32_t shader, const char* name, const glm::vec4& vec4)
{
    int loc = glGetUniformLocation(shader, name);
    glUniform4fv(loc, 1, glm::value_ptr(vec4));
}

static std::array<Vertex, 4> CreateQuad(float x, float y, float textureID)
{
    float size = 1.0f;
    Vertex v0;
    v0.Position[0] = x;
    v0.Position[1] = y;
    v0.Position[2] = 0.0f;
    v0.Color[0] = 0.18f;
    v0.Color[1] = 0.6f;
    v0.Color[2] = 0.96f;
    v0.Color[3] = 1.0f;
    v0.TexCoord[0] = 0.0f;
    v0.TexCoord[1] = 0.0f;
    v0.TexID = textureID;

    Vertex v1;
    v1.Position[0] = x + size;
    v1.Position[1] = y;
    v1.Position[2] = 0.0f;
    v1.Color[0] = 0.18f;
    v1.Color[1] = 0.6f;
    v1.Color[2] = 0.96f;
    v1.Color[3] = 1.0f;
    v1.TexCoord[0] = 1.0f;
    v1.TexCoord[1] = 0.0f;
    v1.TexID = textureID;

    Vertex v2;
    v2.Position[0] = x + size;
    v2.Position[1] = y + size;
    v2.Position[2] = 0.0f;
    v2.Color[0] = 0.18f;
    v2.Color[1] = 0.6f;
    v2.Color[2] = 0.96f;
    v2.Color[3] = 1.0f;
    v2.TexCoord[0] = 1.0f;
    v2.TexCoord[1] = 1.0f;
    v2.TexID = textureID;

    Vertex v3;
    v3.Position[0] = x;
    v3.Position[1] = y + size;
    v3.Position[2] = 0.0f;
    v3.Color[0] = 0.18f;
    v3.Color[1] = 0.6f;
    v3.Color[2] = 0.96f;
    v3.Color[3] = 1.0f;
    v3.TexCoord[0] = 0.0f;
    v3.TexCoord[1] = 1.0f;
    v3.TexID = textureID;

    return { v0, v1, v2, v3 };
}


void SandboxLayer::OnUpdate(Timestep ts)
{
	// Render here
    m_CameraController.OnUpdate(ts);

    //Set Dynamic vertex buffer
    /*float vertices[] = {
        -1.5f, -0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f, 1.0f, 1.0f, 0.0f,
        -1.5f,  0.5f, 0.0f, 0.18f, 0.6f, 0.96f, 1.0f, 0.0f, 1.0f, 0.0f,

         0.5f, -0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f, 0.0f, 0.0f, 1.0f,
         1.5f, -0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f, 1.0f, 0.0f, 1.0f,
         1.5f,  0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 0.93f, 0.24f, 1.0f, 0.0f, 1.0f, 1.0f
    };*/

    auto q0 = CreateQuad(m_QuadPosition[0], m_QuadPosition[1], 0.0f);
    auto q1 = CreateQuad(0.5f, -0.5f, 1.0f);
    Vertex vertices[8];
    memcpy(vertices, q0.data(), q0.size() * sizeof(Vertex));
    memcpy(vertices + q0.size(), q1.data(), q1.size() * sizeof(Vertex));


    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_Shader->GetRendererID());
    glBindTextureUnit(0, m_Cpp);
    glBindTextureUnit(1, m_CppLogo);

    auto& vp = m_CameraController.GetCamera().GetViewProjectionMatrix();
    SetUniformMat4(m_Shader->GetRendererID(), "u_ViewProj", vp);

    glBindVertexArray(m_QuadVA);

    SetUniformMat4(m_Shader->GetRendererID(), "u_Transform", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, nullptr);
}

void SandboxLayer::OnImGuiRender()
{
	// ImGui here

    ImGui::Begin("Controls");
    ImGui::DragFloat2("Quad Position", m_QuadPosition, 0.1f);
    ImGui::End();
}
