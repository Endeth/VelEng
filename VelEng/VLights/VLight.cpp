
#include "VLight.h"

using namespace std;

namespace Vel 
{
	VLightSource::VLightColor::VLightColor(const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : _ambient(ambient), _diffuse(diffuse), _specular(specular)
	{
	}

	VLightSource::VLightSource(const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : _color(ambient, diffuse, specular)
	{
		
	}

	VLightSource::VLightSource(const VLightColor & color) : _color(color)
	{
	}

	void VLightSource::BindShadowMapForWriting()
	{
		_shadowMap->BindFBOWriting();
	}

	void VLightSource::UnbindShadowMapForWriting()
	{
		_shadowMap->UnbindFBOWriting();
	}

	void VLightSource::BindShadowMapForReading()
	{
		_shadowMap->BindTexturesReading();
	}

	void VLightSource::UnbindShadowMapForReading()
	{
		_shadowMap->UnbindTexturesReading();
	}

	VPointLight::VPointLight(const glm::vec3 & position, const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : VLightSource(ambient, diffuse, specular)
	{
		_shadowMap = std::make_unique<VShadowMapCube>(glm::vec2{ 512,512 });
		SetPosition(position);
		_far = 25.0f;
		_constant = 1.0f;
		_linear = 0.09f;
		_quadratic = 0.032f;
	}

	VPointLight::VPointLight(const glm::vec3 & position, const VLightColor & colors) : VLightSource(colors)
	{
		_shadowMap = std::make_unique<VShadowMapCube>(glm::vec2{ 512,512 });
		SetPosition(position);
		_far = 25.0f;
		_constant = 1.0f;
		_linear = 0.09f;
		_quadratic = 0.032f;
	}

	void VPointLight::SetLightUniforms(GLuint lPassProgram)
	{
		auto id = std::to_string(_id);

		glUniform3fv(glGetUniformLocation(lPassProgram, ("pLights[" + id + "].Position").c_str()), 1, &_position[0]);
		glUniform3fv(glGetUniformLocation(lPassProgram, ("pLights[" + id + "].Color").c_str()), 1, &_color.GetDiffuse()[0]);
		glUniform1f(glGetUniformLocation(lPassProgram, ("pLights[" + id + "].Constant").c_str()), _constant);
		glUniform1f(glGetUniformLocation(lPassProgram, ("pLights[" + id + "].Linear").c_str()), _linear);
		glUniform1f(glGetUniformLocation(lPassProgram, ("pLights[" + id + "].Quadratic").c_str()), _quadratic);
	}

	void VPointLight::SetShadowUniforms()
	{
		for (int i = 0; i < 6; i++)
		{
			auto uni = ("shadowMatrices[" + std::to_string(i) + "]").c_str();
			auto ptr = glm::value_ptr(_shadowTransforms[i]);
			glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID() , uni), 1, GL_FALSE, ptr);
		}
	}

	void VPointLight::SetPosition(const glm::vec3 & pos)
	{
		_position = pos;
		UpdateShadowTransforms();
	}

	void VPointLight::SetLightID(int id)
	{
		_id = id;
	}

	void VPointLight::UpdateShadowTransforms()
	{
		GLfloat aspect = 1;
		GLfloat near = 1.0f;
		glm::mat4 shadowProj = glm::perspective(90.0f, aspect, near, _far);
		auto view = glm::lookAt(_position, _position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)); //make a function ffs
		_shadowTransforms.push_back(shadowProj * view);
		view = glm::lookAt(_position, _position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
		_shadowTransforms.push_back(shadowProj * view);
		view = glm::lookAt(_position, _position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
		_shadowTransforms.push_back(shadowProj * view);
		view = glm::lookAt(_position, _position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
		_shadowTransforms.push_back(shadowProj * view);
		view = glm::lookAt(_position, _position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
		_shadowTransforms.push_back(shadowProj * view);
		view = glm::lookAt(_position, _position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
		_shadowTransforms.push_back(shadowProj * view);
	}

	void VPointLight::BindShadowMapForWriting()
	{
		_shadowMap->BindFBOWriting();
	}

	void VPointLight::UnbindShadowMapForWriting()
	{
		_shadowMap->UnbindFBOWriting();
	}

	void VPointLight::BindShadowMapForReading()
	{
		_shadowMap->BindTexturesReading();
	}

	void VPointLight::UnbindShadowMapForReading()
	{
		_shadowMap->UnbindTexturesReading();
	}

	VDirectionalLight::VDirectionalLight(const glm::vec3 & direction, const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : VLightSource(ambient, diffuse, specular), _direction(direction)
	{
	}

	VDirectionalLight::VDirectionalLight(const glm::vec3 & direction, const VLightColor & colors) : VLightSource(colors), _direction(direction)
	{
	}
	void VSceneLighting::DrawSceneShadows(const std::vector<std::shared_ptr<VModel>>& models)
	{
		for(auto &lightSource : _sceneLights)
		{
			lightSource->BindShadowMapForWriting();
			lightSource->ActivateShader();
			
			lightSource->SetShadowUniforms();
			for (auto &model : models)
			{
				model->SetModelMatrixUniform(lightSource->GetShader());
				model->DrawModelWithImposedShader();
			}
			lightSource->DeactivateShader();
			lightSource->UnbindShadowMapForWriting();
		}
	}
	void VSceneLighting::AddLight(const std::shared_ptr<VLightSource>& lightSource)
	{
		_sceneLights.push_back(lightSource);
	}
	void VSceneLighting::SetLightUniforms(GLuint lPassProgram)
	{
		glUniform3fv(glGetUniformLocation(lPassProgram, ("ambientLight")), 1, &_ambientLight[0]);
		if (_sceneLights.empty())
		{
			return;
		}
		for (auto& lightSource : _sceneLights)
		{
			lightSource->SetLightUniforms(lPassProgram);
		}
	}
}