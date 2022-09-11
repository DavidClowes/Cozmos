#include "Cozmos.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public Cozmos::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f)
	{
		m_VertexArray.reset(Cozmos::VertexArray::Create());

		float vertices[3 * 7] =
		{
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};

		Cozmos::Ref<Cozmos::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Cozmos::VertexBuffer::Create(vertices, sizeof(vertices)));
		Cozmos::BufferLayout layout =
		{
			{ Cozmos::ShaderDataType::Float3, "a_Position" },
			{ Cozmos::ShaderDataType::Float4, "a_Color" }
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0, 1, 2 };
		Cozmos::Ref<Cozmos::IndexBuffer> indexBuffer;
		indexBuffer.reset(Cozmos::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		// Square //

		m_SquareVA.reset(Cozmos::VertexArray::Create());

		float squareVertices[5 * 4] =
		{
		    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Cozmos::Ref<Cozmos::VertexBuffer> squareVB;
		squareVB.reset(Cozmos::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ Cozmos::ShaderDataType::Float3, "a_Position" },
			{ Cozmos::ShaderDataType::Float2, "a_TexCoord" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Cozmos::Ref<Cozmos::IndexBuffer> squareIB;
		squareIB.reset(Cozmos::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 330 core
		
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec3 v_Position;
			out vec4 v_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_ModelTransform;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_ModelTransform * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core
		
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

		m_Shader.reset(Cozmos::Shader::Create(vertexSrc, fragmentSrc));

		std::string flatColorShaderVertexSrc = R"(
			#version 330 core
		
			layout(location = 0) in vec3 a_Position;

			out vec3 v_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_ModelTransform;


			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_ModelTransform * vec4(a_Position, 1.0);
			}
		)";

		std::string flatColorShaderFragmentSrc = R"(
			#version 330 core
		
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

		m_FlatColorShader.reset(Cozmos::Shader::Create(flatColorShaderVertexSrc, flatColorShaderFragmentSrc));

		m_TextureShader.reset(Cozmos::Shader::Create("Assets/Shaders/Texture.glsl"));

		m_Texture = Cozmos::Texture2D::Create("Assets/Textures/Checkerboard.png");
		m_AlphaTexture = Cozmos::Texture2D::Create("Assets/Textures/XD.png");

		std::dynamic_pointer_cast<Cozmos::OpenGLShader>(m_TextureShader)->Bind();
		std::dynamic_pointer_cast<Cozmos::OpenGLShader>(m_TextureShader)->UploadUniformInt("u_Texture", 0);

	}

	void OnUpdate(Cozmos::Timestep timestep) override
	{
		if (Cozmos::Input::IsKeyPressed(COZ_KEY_LEFT))
			m_CameraPosition.x -= m_CameraSpeed * timestep;
		if (Cozmos::Input::IsKeyPressed(COZ_KEY_RIGHT))
			m_CameraPosition.x += m_CameraSpeed * timestep;

		if (Cozmos::Input::IsKeyPressed(COZ_KEY_DOWN))
			m_CameraPosition.y -= m_CameraSpeed * timestep;
		if (Cozmos::Input::IsKeyPressed(COZ_KEY_UP))
			m_CameraPosition.y += m_CameraSpeed * timestep;

		if (Cozmos::Input::IsKeyPressed(COZ_KEY_A))
			m_CameraRotation -= m_CameraRotationSpeed * timestep;
		if (Cozmos::Input::IsKeyPressed(COZ_KEY_D))
			m_CameraRotation += m_CameraRotationSpeed * timestep;

		Cozmos::RenderCommand::SetClearColor({ 0.1, 0.1, 0.1, 1 });
		Cozmos::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);


		Cozmos::Renderer::BeginScene(m_Camera);
		{
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

			glm::vec4 redColor(0.8f, 0.2f, 0.3f, 1.0f);
			glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);

			std::dynamic_pointer_cast<Cozmos::OpenGLShader>(m_FlatColorShader)->Bind();
			std::dynamic_pointer_cast<Cozmos::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);

			for (int y = 0; y < 20; y++)
			{
				for (int x = 0; x < 5; x++)
				{
					glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
					glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
					Cozmos::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
				}
			}

			m_Texture->Bind();
			Cozmos::Renderer::Submit(m_TextureShader, m_SquareVA,
				glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
			m_AlphaTexture->Bind();
			Cozmos::Renderer::Submit(m_TextureShader, m_SquareVA,
				glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

			// 
			// Triangle
			// Cozmos::Renderer::Submit(m_Shader, m_VertexArray);
		}
		Cozmos::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		{
			ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		}
		ImGui::End();
	}

	void OnEvent(Cozmos::Event& event) override
	{
	}

private:

	Cozmos::Ref<Cozmos::Shader> m_Shader;
	Cozmos::Ref<Cozmos::VertexArray> m_VertexArray;

	Cozmos::Ref<Cozmos::Shader> m_FlatColorShader, m_TextureShader;
	Cozmos::Ref<Cozmos::VertexArray> m_SquareVA;

	Cozmos::Ref<Cozmos::Texture2D> m_Texture, m_AlphaTexture;

	Cozmos::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraSpeed = 0.1f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 2.0f;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class Sandbox : public Cozmos::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}
};

Cozmos::Application* Cozmos::CreateApplication()
{
	return new Sandbox();
}