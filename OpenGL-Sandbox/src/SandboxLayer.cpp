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

    const size_t MaxQuadCount = 1000;
    const size_t MaxVertexCount = MaxQuadCount * 4;
    const size_t MaxIndexCount = MaxQuadCount * 6;

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MaxVertexCount, nullptr, GL_DYNAMIC_DRAW);


    glEnableVertexArrayAttrib(m_QuadVB, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));

    glEnableVertexArrayAttrib(m_QuadVB, 1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Color));

    glEnableVertexArrayAttrib(m_QuadVB, 2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoord));

    glEnableVertexArrayAttrib(m_QuadVB, 3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexID));

//     uint32_t indices[] = {
//         0, 1, 2, 2, 3, 0,
//         4, 5, 6, 6, 7, 4
//     };

    uint32_t indices[MaxIndexCount];
    uint32_t offset = 0;
    for (size_t i = 0; i < MaxIndexCount; i += 6)
    {
        indices[i + 0] = 0 + offset;
        indices[i + 1] = 1 + offset;
        indices[i + 2] = 2 + offset;

        indices[i + 3] = 2 + offset;
        indices[i + 4] = 3 + offset;
        indices[i + 5] = 0 + offset;

        offset += 4;
    }

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

static Vertex* CreateQuad(Vertex* target, float x, float y, float textureID)
{
    float size = 1.0f;
    target->Position[0] = x;
    target->Position[1] = y;
    target->Position[2] = 0.0f;
    target->Color[0] = 0.18f;
    target->Color[1] = 0.6f;
    target->Color[2] = 0.96f;
    target->Color[3] = 1.0f;
    target->TexCoord[0] = 0.0f;
    target->TexCoord[1] = 0.0f;
    target->TexID = textureID;
    target++;

    target->Position[0] = x + size;
    target->Position[1] = y;
    target->Position[2] = 0.0f;
    target->Color[0] = 0.18f;
    target->Color[1] = 0.6f;
    target->Color[2] = 0.96f;
    target->Color[3] = 1.0f;
    target->TexCoord[0] = 1.0f;
    target->TexCoord[1] = 0.0f;
    target->TexID = textureID;
    target++;

    target->Position[0] = x + size;
    target->Position[1] = y + size;
    target->Position[2] = 0.0f;
    target->Color[0] = 0.18f;
    target->Color[1] = 0.6f;
    target->Color[2] = 0.96f;
    target->Color[3] = 1.0f;
    target->TexCoord[0] = 1.0f;
    target->TexCoord[1] = 1.0f;
    target->TexID = textureID;
    target++;

    target->Position[0] = x;
    target->Position[1] = y + size;
    target->Position[2] = 0.0f;
    target->Color[0] = 0.18f;
    target->Color[1] = 0.6f;
    target->Color[2] = 0.96f;
    target->Color[3] = 1.0f;
    target->TexCoord[0] = 0.0f;
    target->TexCoord[1] = 1.0f;
    target->TexID = textureID;
    target++;

    return target;
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

    std::array<Vertex, 1000> vertices;
    uint32_t indexCount = 0;
    Vertex* buffer = vertices.data();
    for (int y = 0; y < 5; ++y)
    {
        for (int x = 0; x < 5; ++x)
        {
            buffer = CreateQuad(buffer, x, y, (x + y) % 2);
            indexCount += 6;
        }
    }
    buffer = CreateQuad(buffer, m_QuadPosition[0], m_QuadPosition[1], 0.0f);
    indexCount += 6;

    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_Shader->GetRendererID());
    glBindTextureUnit(0, m_Cpp);
    glBindTextureUnit(1, m_CppLogo);

    auto& vp = m_CameraController.GetCamera().GetViewProjectionMatrix();
    SetUniformMat4(m_Shader->GetRendererID(), "u_ViewProj", vp);

    glBindVertexArray(m_QuadVA);

    SetUniformMat4(m_Shader->GetRendererID(), "u_Transform", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}

void SandboxLayer::OnImGuiRender()
{
	// ImGui here

    ImGui::Begin("Controls");
    ImGui::DragFloat2("Quad Position", m_QuadPosition, 0.1f);
    ImGui::End();
}
