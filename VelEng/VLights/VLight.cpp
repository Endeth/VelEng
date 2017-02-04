
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
		_shadowMap = std::make_unique<VShadowMap2D>(glm::vec2{ 1024,1024 });
		SetPosition(position);
		_far = 25.0f;
		_constant = 1.0f;
		_linear = 0.09f;
		_quadratic = 0.032f;
	}

	VPointLight::VPointLight(const glm::vec3 & position, const VLightColor & colors) : VLightSource(colors)
	{
		_shadowMap = std::make_unique<VShadowMap2D>(glm::vec2{ 1024,1024 });
		SetPosition(position);
		_far = 7.5f;
		_constant = 1.0f;
		_linear = 0.03f;
		_quadratic = 0.02f;
	}

	void VPointLight::SetLightUniforms(GLuint lPassProgram)
	{
		auto id = std::to_string(_id);

		glUniform3fv(glGetUniformLocation(lPassProgram, ("pLights[" + id + "].Position").c_str()), 1, &_position[0]);
		glUniform3fv(glGetUniformLocation(lPassProgram, ("pLights[" + id + "].Color").c_str()), 1, &_color.GetDiffuse()[0]);
		glUniform1f(glGetUniformLocation(lPassProgram, ("pLights[" + id + "].Constant").c_str()), _constant);
		glUniform1f(glGetUniformLocation(lPassProgram, ("pLights[" + id + "].Linear").c_str()), _linear);
		glUniform1f(glGetUniformLocation(lPassProgram, ("pLights[" + id + "].Quadratic").c_str()), _quadratic);
		glUniformMatrix4fv(glGetUniformLocation(lPassProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	}

	void VPointLight::SetShadowUniforms()
	{
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID() , "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

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
		//auto shadowProj = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near, _far);
		auto lightView = glm::lookAt(_position, glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = shadowProj * lightView;
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
			glViewport(0, 0, 1024, 1024);
			lightSource->BindShadowMapForWriting();
			glClear(GL_DEPTH_BUFFER_BIT);
			lightSource->ActivateShader();
			
			lightSource->SetShadowUniforms();
			for (auto &model : models)
			{
				model->SetModelMatrixUniform(lightSource->GetShader());
				model->DrawModelWithImposedShader();
			}
			lightSource->DeactivateShader();
			lightSource->UnbindShadowMapForWriting();
			glViewport(0, 0, 1366, 768);
		}
	}
	void VSceneLighting::AddLight(const std::shared_ptr<VLightSource>& lightSource)
	{
		_sceneLights.push_back(lightSource);
	}
	void VSceneLighting::ActivateShadowMap()
	{
		_sceneLights.front()->BindShadowMapForReading();
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