
#include "VRenderer.h"

Vel::VDefferedRenderer::VDefferedRenderer(const glm::ivec2& resolution, const ShaderPtr &gPass, const ShaderPtr &lPass) : _gBuffer(resolution), _gPassShader(gPass), _lPassShader(lPass)
{
	_quad.SetShader(lPass);
}

//g and l passes
void Vel::VDefferedRenderer::Render()
{
	GeometryPass();
	LightingPass();
}

void Vel::VDefferedRenderer::SetLPassShader(const ShaderPtr & shader)
{
	_lPassShader = shader;
	_quad.SetShader(shader);
	_quad.SetVAO();
}

void Vel::VDefferedRenderer::BindGBufferForWriting()
{
	_gBuffer.BindFBOWriting();
}

void Vel::VDefferedRenderer::UnbindGBufferForWriting()
{
	_gBuffer.UnbindFBOWriting();
}

//bids gbuffer and draws with renderers gpass shader
void Vel::VDefferedRenderer::GeometryPass()
{
	BindGBufferForWriting();

	_scene->DrawSceneWithImposedShader(_gPassShader);

	UnbindGBufferForWriting();
}

//additionaly clears default framebuffer
void Vel::VDefferedRenderer::LightingPass()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	_lPassShader->Activate();
	_gBuffer.BindTexturesReading();
	_quad.DrawWithImposedShader();

	_gBuffer.UnbindTexturesReading();
	_lPassShader->Deactivate();
}

Vel::VDefferedRenderer::LightingPassQuad::LightingPassQuad()
{
	_primitive = GL_TRIANGLES;
	_vertices.reserve(4);
	_vertices.push_back(Vertex(glm::vec3{ -1.0f,  1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec2{ 0.0f, 1.0f }));
	_vertices.push_back(Vertex(glm::vec3{ -1.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec2{ 0.0f, 0.0f }));
	_vertices.push_back(Vertex(glm::vec3{  1.0f,  1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec2{ 1.0f, 1.0f }));
	_vertices.push_back(Vertex(glm::vec3{  1.0f, -1.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec2{ 1.0f, 0.0f }));

	_indices.reserve(6);
	_indices.push_back(0); _indices.push_back(1); _indices.push_back(2);
	_indices.push_back(1); _indices.push_back(3); _indices.push_back(2);

	LoadIntoGPU();
}

void Vel::VDefferedRenderer::LightingPassQuad::SetVAO()
{
	auto stride = sizeof(Vertex);

	glBindVertexArray(_vaoID);
	_vboVertices.BindBuffer();

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offsetof(Vertex, UV));

	_vboIndices.BindBuffer();

	glBindVertexArray(0);
	_vboIndices.UnbindBuffer();
	_vboVertices.UnbindBuffer();
}

void Vel::VDefferedRenderer::LightingPassQuad::LoadIntoGPU()
{
	_vboVertices.FillBuffer(_vertices.size(), &_vertices[0]);
	_vboIndices.FillBuffer(_indices.size(), &_indices[0]);
}
