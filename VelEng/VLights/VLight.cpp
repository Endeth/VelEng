
#include "VLight.h"

using namespace std;

namespace Vel 
{
	VLightSource::VLightColor::VLightColor(const glm::vec3 & diffuse, const glm::vec3 & specular) : _diffuse(diffuse), _specular(specular)
	{
	}

	VLightSource::VLightSource(const glm::vec3 & diffuse, const glm::vec3 & specular) : _color(diffuse, specular)
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



	VPointLight::VPointLight(const glm::vec3 & position, const glm::vec3 & diffuse, const glm::vec3 & specular) : VLightSource(diffuse, specular), _shadowTransforms(6)
	{
		_shadowMap = std::make_unique<VShadowMapCube>(glm::vec2{ 1024,1024 }); //TODO get rid of hard code
		SetPosition(position);
		_constant = 1.0f;
		_linear = 0.09f; //TODO attenuation might come handy in engine
		_quadratic = 0.032f;
	}

	VPointLight::VPointLight(const glm::vec3 & position, const VLightColor & colors) : VLightSource(colors), _shadowTransforms(6)
	{
		_shadowMap = std::make_unique<VShadowMapCube>(glm::vec2{ 1024,1024 });
		SetPosition(position);
		_constant = 1.0f;
		_linear = 0.09f;
		_quadratic = 0.032f;
	}

	void VPointLight::SetLPassLightUniforms(GLuint program, GLuint uniformID)
	{
		auto id = std::to_string(uniformID);

		glUniform3fv(glGetUniformLocation(program, ("pointLights[" + id + "].Position").c_str()), 1, &_position[0]);
		glUniform3fv(glGetUniformLocation(program, ("pointLights[" + id + "].ColorDiff").c_str()), 1, &_color.GetDiffuse()[0]); 
		glUniform3fv(glGetUniformLocation(program, ("pointLights[" + id + "].ColorSpec").c_str()), 1, &_color.GetSpecular()[0]);
		glUniform1f(glGetUniformLocation(program, ("pointLights[" + id + "].Constant").c_str()), _constant);
		glUniform1f(glGetUniformLocation(program, ("pointLights[" + id + "].FarPlane").c_str()), 25.0f); //TODO add far plane
		glUniform1f(glGetUniformLocation(program, ("pointLights[" + id + "].Linear").c_str()), _linear);
		glUniform1f(glGetUniformLocation(program, ("pointLights[" + id + "].Quadratic").c_str()), _quadratic);
		//glUniformMatrix4fv(glGetUniformLocation(program, ("pointLights[" + id + "].lightSpaceMatrix").c_str()), 1, GL_FALSE, glm::value_ptr(_lightSpaceMatrix)); //TODO Don't know if this will be the same with cube shadow
	}

	void VPointLight::SetShadowUniforms()
	{
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID(), "shadowTransforms[0]"), 1, GL_FALSE, glm::value_ptr(_shadowTransforms[0]));
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID(), "shadowTransforms[1]"), 1, GL_FALSE, glm::value_ptr(_shadowTransforms[1]));
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID(), "shadowTransforms[2]"), 1, GL_FALSE, glm::value_ptr(_shadowTransforms[2]));
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID(), "shadowTransforms[3]"), 1, GL_FALSE, glm::value_ptr(_shadowTransforms[3]));
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID(), "shadowTransforms[4]"), 1, GL_FALSE, glm::value_ptr(_shadowTransforms[4]));
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID(), "shadowTransforms[5]"), 1, GL_FALSE, glm::value_ptr(_shadowTransforms[5]));
			
		glUniform1f(glGetUniformLocation(_depthShader->GetProgramID(), "farPlane"), 25.0f); //TODO magic number
		glUniform3fv(glGetUniformLocation(_depthShader->GetProgramID(), "lightPos"), 1, &_position[0]);

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
		GLfloat near = 1.0f;

		auto shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
		_shadowTransforms[0] = (shadowProj * glm::lookAt(_position, _position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		_shadowTransforms[1] = (shadowProj * glm::lookAt(_position, _position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		_shadowTransforms[2] = (shadowProj * glm::lookAt(_position, _position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		_shadowTransforms[3] = (shadowProj * glm::lookAt(_position, _position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		_shadowTransforms[4] = (shadowProj * glm::lookAt(_position, _position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		_shadowTransforms[5] = (shadowProj * glm::lookAt(_position, _position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
	}

	VDirectionalLight::VDirectionalLight(const glm::vec3 & direction, const glm::vec3 & diffuse, const glm::vec3 & specular) : VLightSource(diffuse, specular)
	{
		_shadowMap = std::make_unique<VShadowMap2D>(glm::vec2{ 2048,2048 }); //TODO get rid of hard code
		SetDirection(direction);
	}

	VDirectionalLight::VDirectionalLight(const glm::vec3 & direction, const VLightColor & colors) : VLightSource(colors)
	{
		_shadowMap = std::make_unique<VShadowMap2D>(glm::vec2{ 2048,2048 }); //TODO get rid of hard code
		SetDirection(direction);
	}
	void VDirectionalLight::SetLPassLightUniforms(GLuint program)
	{
		glUniform3fv(glGetUniformLocation(program, "dirLight.Direction"), 1, &_direction[0]);
		glUniform3fv(glGetUniformLocation(program, "dirLight.ColorDiff"), 1, &_color.GetDiffuse()[0]);
		glUniform3fv(glGetUniformLocation(program, "dirLight.ColorSpec"), 1, &_color.GetSpecular()[0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "dirLight.lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(_lightSpaceMatrix));
	}
	void VDirectionalLight::SetShadowUniforms()
	{
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(_lightSpaceMatrix));
	}
	void VDirectionalLight::SetDirection(const glm::vec3 & dir)
	{
		_direction = dir;
	}
	void VDirectionalLight::SetCameraPosition(const glm::vec3 & pos)
	{
		_camPosition = pos;
		_shadowCastingPosition = _camPosition - _direction * 10.0f;
		UpdateShadowTransforms();
	}
	void VDirectionalLight::UpdateShadowTransforms()
	{
		GLfloat aspect = 1; //TODO get this shit straight
		GLfloat far = 50.0f;
		GLfloat near = 1.0f;

		auto _lightProj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near, far);
		auto _lightView = glm::lookAt(_shadowCastingPosition, _camPosition, glm::vec3(0.0, 1.0, 0.0));

		_lightSpaceMatrix = _lightProj * _lightView;
	}
	void VSceneLighting::DrawSceneShadows(const std::vector<std::shared_ptr<VModel>>& models) //TODO erase repeating code
	{
		glCullFace(GL_FRONT);
		glViewport(0, 0, 2048, 2048); //TODO set to directional light shader resolution
		_directionalLight->BindShadowMapForWriting();
		glClear(GL_DEPTH_BUFFER_BIT);
		_directionalLight->ActivateShader();

		_directionalLight->SetShadowUniforms();
		for (auto &model : models)
		{
			model->SetModelMatrixUniform(_directionalLight->GetShader());
			model->DrawModelWithImposedShader();
		}
		_directionalLight->DeactivateShader();
		_directionalLight->UnbindShadowMapForWriting();

		glViewport(0, 0, 1024, 1024);
		int counter = 0;
		for(auto &lightSource : _sceneLights)
		{
			if (counter < 4)
			{
				//glViewport(0, 0, 1024, 1024); //TODO set to each lights shaders resolution
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
				
				counter++;
				continue;
			}
			break;
		}
		glViewport(0, 0, 1366, 768); //TODO set to main viewport
		glCullFace(GL_BACK);
	}
	void VSceneLighting::AddLight(const std::shared_ptr<VLightSource>& lightSource)
	{
		_sceneLights.push_back(lightSource);
	}
	void VSceneLighting::CreateDirectionalLight(const glm::vec3 & dir, const VLightSource::VLightColor & color)
	{
		_directionalLight = std::make_unique<VDirectionalLight>(dir, color);
	}
	void VSceneLighting::CreateDirectionalLight(std::unique_ptr<VDirectionalLight>&& light)
	{
		_directionalLight = std::move(light);
	}
	void VSceneLighting::ActivateShadowMaps()
	{
		int counter = 0;
		if (_directionalLight != nullptr)
		{
			_directionalLight->SetTextureUnit(GL_TEXTURE8);
			_directionalLight->BindShadowMapForReading();
		}
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
	void VSceneLighting::SetLPassLightUniforms(GLuint lPassProgram)
	{
		glUniform3fv(glGetUniformLocation(lPassProgram, ("ambientLight")), 1, &_ambientLight[0]);
		if (_directionalLight != nullptr)
		{
			_directionalLight->SetLPassLightUniforms(lPassProgram);
		}
		int counter = 0;
		for (auto& lightSource : _sceneLights)
		{
			if(counter < 16)
				lightSource->SetLPassLightUniforms(lPassProgram, counter);
			counter++;
		}
	}
	void VSceneLighting::SetCameraPosition(const glm::vec3 &pos)
	{
		_cameraPosition = pos;
		if(_directionalLight != nullptr)
			_directionalLight->SetCameraPosition(pos);
	}
}