
#include "VLight.h"

using namespace std;

namespace Vel 
{
	LightSource::LightColor::LightColor(const glm::vec3 & diffuse, const glm::vec3 & specular) : _diffuse(diffuse), _specular(specular)
	{
	}

	LightSource::LightSource(const glm::vec3 & diffuse, const glm::vec3 & specular) : _color(diffuse, specular)
	{
		
	}

	LightSource::LightSource(const LightColor & color) : _color(color)
	{
	}

	void LightSource::SetShadowUniforms()
	{
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(_lightSpaceMatrix));
	}

	void LightSource::SetTextureUnit(GLuint textureUnit)
	{
		_shadowMap->SetTextureUnit(textureUnit);
	}

	void LightSource::BindShadowMapForWriting()
	{
		_shadowMap->BindFBOWriting();
	}

	void LightSource::UnbindShadowMapForWriting()
	{
		_shadowMap->UnbindFBOWriting();
	}

	void LightSource::BindShadowMapForReading()
	{
		_shadowMap->BindTexturesReading();
	}

	void LightSource::UnbindShadowMapForReading()
	{
		_shadowMap->UnbindTexturesReading();
	}



	PointLight::PointLight(const glm::vec3 & position, const glm::vec3 & diffuse, const glm::vec3 & specular) : LightSource(diffuse, specular), _shadowTransforms(6)
	{
		_shadowResolution = glm::vec2{ 1024,1024 };
		_shadowMap = std::make_unique<ShadowMapCube>(_shadowResolution);
		SetPosition(position);
		_constant = 1.0f;
		_linear = 0.09f; //TODO attenuation might come handy in engine
		_quadratic = 0.032f;
	}

	PointLight::PointLight(const glm::vec3 & position, const LightColor & colors) : LightSource(colors), _shadowTransforms(6)
	{
		_shadowResolution = glm::vec2{ 1024,1024 };
		_shadowMap = std::make_unique<ShadowMapCube>(_shadowResolution);
		SetPosition(position);
		_constant = 1.0f;
		_linear = 0.09f;
		_quadratic = 0.032f;
	}

	void PointLight::SetLPassLightUniforms(GLuint program, GLuint uniformID)
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

	void PointLight::SetShadowUniforms()
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

	void PointLight::SetPosition(const glm::vec3 & pos)
	{
		_position = pos;
		UpdateShadowTransforms();
	}


	void PointLight::UpdateShadowTransforms()
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

	DirectionalLight::DirectionalLight(const glm::vec3 & direction, const glm::vec3 & diffuse, const glm::vec3 & specular) : LightSource(diffuse, specular)
	{
		_shadowResolution = glm::vec2{ 2048,2048 };
		_shadowMap = std::make_unique<ShadowMap2D>(_shadowResolution);
		SetDirection(direction);
	}

	DirectionalLight::DirectionalLight(const glm::vec3 & direction, const LightColor & colors) : LightSource(colors)
	{
		_shadowResolution = glm::vec2{ 2048,2048 };
		_shadowMap = std::make_unique<ShadowMap2D>(_shadowResolution);
		SetDirection(direction);
	}
	void DirectionalLight::SetLPassLightUniforms(GLuint program)
	{
		glUniform3fv(glGetUniformLocation(program, "dirLight.Direction"), 1, &_direction[0]);
		glUniform3fv(glGetUniformLocation(program, "dirLight.ColorDiff"), 1, &_color.GetDiffuse()[0]);
		glUniform3fv(glGetUniformLocation(program, "dirLight.ColorSpec"), 1, &_color.GetSpecular()[0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "dirLight.lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(_lightSpaceMatrix));
	}
	void DirectionalLight::SetShadowUniforms()
	{
		glUniformMatrix4fv(glGetUniformLocation(_depthShader->GetProgramID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(_lightSpaceMatrix));
	}
	void DirectionalLight::SetDirection(const glm::vec3 & dir)
	{
		_direction = dir;
	}
	void DirectionalLight::SetCameraPosition(const glm::vec3 & pos)
	{
		_camPosition = pos;
		_shadowCastingPosition = _camPosition - _direction * 10.0f;
		UpdateShadowTransforms();
	}
	void DirectionalLight::UpdateShadowTransforms()
	{
		GLfloat aspect = 1; //TODO get this shit straight
		GLfloat far = 50.0f;
		GLfloat near = 1.0f;

		auto _lightProj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near, far);
		auto _lightView = glm::lookAt(_shadowCastingPosition, _camPosition, glm::vec3(0.0, 1.0, 0.0));

		_lightSpaceMatrix = _lightProj * _lightView;
	}
	void SceneLighting::DrawSceneShadows(const std::vector<std::shared_ptr<Model>>& models) //TODO erase repeating code
	{
		glCullFace(GL_FRONT);
		auto dirRes = _directionalLight->GetShadowResolution();
		glViewport(0, 0, dirRes.x, dirRes.y); //TODO set to directional light shader resolution
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

		
		int counter = 0;
		for(auto &lightSource : _sceneLights)
		{
			if (counter < 4)
			{
				auto res = lightSource->GetShadowResolution();
				glViewport(0, 0, res.x, res.y);
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
	void SceneLighting::AddLight(const std::shared_ptr<LightSource>& lightSource)
	{
		_sceneLights.push_back(lightSource);
	}
	void SceneLighting::CreateDirectionalLight(const glm::vec3 & dir, const LightSource::LightColor & color)
	{
		_directionalLight = std::make_unique<DirectionalLight>(dir, color);
	}
	void SceneLighting::CreateDirectionalLight(std::unique_ptr<DirectionalLight>&& light)
	{
		_directionalLight = std::move(light);
	}
	void SceneLighting::ActivateShadowMaps()
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
	void SceneLighting::SetLPassLightUniforms(GLuint lPassProgram)
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
	void SceneLighting::SetCameraPosition(const glm::vec3 &pos)
	{
		_cameraPosition = pos;
		if(_directionalLight != nullptr)
			_directionalLight->SetCameraPosition(pos);
	}
}