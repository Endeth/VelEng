
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

	void VLightSource::SetShadowUniforms()
	{
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(_lightSpaceMatrix));
	}

	void VLightSource::SetTextureUnit(GLuint textureUnit)
	{
		_shadowMap->SetTextureUnit(textureUnit);
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
		_shadowMap = std::make_unique<VShadowMap2D>(glm::vec2{ 1024,1024 }); //TODO get rid of hard code
		SetPosition(position);
		_constant = 1.0f;
		_linear = 0.09f;
		_quadratic = 0.032f;
	}

	VPointLight::VPointLight(const glm::vec3 & position, const VLightColor & colors) : VLightSource(colors)
	{
		_shadowMap = std::make_unique<VShadowMap2D>(glm::vec2{ 1024,1024 });
		SetPosition(position);
		_constant = 1.0f;
		_linear = 0.03f;
		_quadratic = 0.02f;
	}

	void VPointLight::SetLightUniforms(GLuint program, GLuint uniformID)
	{
		auto id = std::to_string(uniformID);

		glUniform3fv(glGetUniformLocation(program, ("pLights[" + id + "].Position").c_str()), 1, &_position[0]);
		glUniform3fv(glGetUniformLocation(program, ("pLights[" + id + "].Color").c_str()), 1, &_color.GetDiffuse()[0]);
		glUniform1f(glGetUniformLocation(program, ("pLights[" + id + "].Constant").c_str()), _constant);
		glUniform1f(glGetUniformLocation(program, ("pLights[" + id + "].Linear").c_str()), _linear);
		glUniform1f(glGetUniformLocation(program, ("pLights[" + id + "].Quadratic").c_str()), _quadratic);
		glUniformMatrix4fv(glGetUniformLocation(program, ("pLights[" + id + "].lightSpaceMatrix").c_str()), 1, GL_FALSE, glm::value_ptr(_lightSpaceMatrix));
	}

	void VPointLight::SetShadowUniforms()
	{
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID() , "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(_lightSpaceMatrix)); //TODO Don't know if this will be the same with cube shadow

	}

	void VPointLight::SetPosition(const glm::vec3 & pos)
	{
		_position = pos;
		UpdateShadowTransforms();
	}


	void VPointLight::UpdateShadowTransforms()
	{
		GLfloat aspect = 1; //TODO get this shit straight
		GLfloat far = 25.0f;
		GLfloat near = 0.5f;
		//glm::mat4 shadowProj = glm::perspective(90.0f, aspect, near, _far); //TODO change to this somewhere soon
		auto _lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near, far);
		auto _lightView = glm::lookAt(_position, glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0)); //TODO when creating shadow cube this will look different

		_lightSpaceMatrix = _lightProj * _lightView;
	}

	VDirectionalLight::VDirectionalLight(const glm::vec3 & direction, const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : VLightSource(ambient, diffuse, specular), _direction(direction)
	{
	}

	VDirectionalLight::VDirectionalLight(const glm::vec3 & direction, const VLightColor & colors) : VLightSource(colors), _direction(direction)
	{
	}
	void VSceneLighting::DrawSceneShadows(const std::vector<std::shared_ptr<VModel>>& models)
	{
		int counter = 0;
		for(auto &lightSource : _sceneLights)
		{
			if (counter < 4)
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
				counter++;
				continue;
			}
			break;
		}
	}
	void VSceneLighting::AddLight(const std::shared_ptr<VLightSource>& lightSource)
	{
		_sceneLights.push_back(lightSource);
	}
	void VSceneLighting::ActivateShadowMap()
	{
		int counter = 0;
		for (auto ite = _sceneLights.begin(); ite != _sceneLights.end(); ite++)
		{
			if (counter < 4)
			{
				(*ite)->SetTextureUnit(GL_TEXTURE4 + counter);
				(*ite)->BindShadowMapForReading();
				counter++;
				continue;
			}
			break;
		}
	}
	void VSceneLighting::SetLightUniforms(GLuint lPassProgram)
	{
		glUniform3fv(glGetUniformLocation(lPassProgram, ("ambientLight")), 1, &_ambientLight[0]);

		int counter = 0;
		for (auto& lightSource : _sceneLights)
		{
			if(counter < 16)
				lightSource->SetLightUniforms(lPassProgram, counter);
			counter++;
		}
	}
}