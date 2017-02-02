
#include "VLight.h"

using namespace std;

namespace Vel 
{
	VLightSource::VLightColor::VLightColor(const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : _ambient(ambient), _diffuse(diffuse), _specular(specular)
	{
	}

	VLightSource::VLightSource(const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : _color(ambient, diffuse, specular)
	{
		_shadowMap = make_unique<VShadowMap>();
	}

	VLightSource::VLightSource(const VLightColor & color) : _color(color)
	{
	}

	void VLightSource::BindShadowMapForReading()
	{
		_shadowMap->BindTexturesReading();
	}

	void VLightSource::UnbindShadowMapForReading()
	{
		_shadowMap->UnbindTexturesReading();
	}

	VPointLight::VPointLight(const glm::vec3 & position, const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : VLightSource(ambient, diffuse, specular), _position(position)
	{
		_constant = 1.0f;
		_linear = 0.09f;
		_quadratic = 0.032f;
	}

	VPointLight::VPointLight(const glm::vec3 & position, const VLightColor & colors) : VLightSource(colors), _position(position)
	{
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

	void VPointLight::SetLightID(int id)
	{
		_id = id;
	}

	VDirectionalLight::VDirectionalLight(const glm::vec3 & direction, const glm::vec3 & ambient, const glm::vec3 & diffuse, const glm::vec3 & specular) : VLightSource(ambient, diffuse, specular), _direction(direction)
	{
	}

	VDirectionalLight::VDirectionalLight(const glm::vec3 & direction, const VLightColor & colors) : VLightSource(colors), _direction(direction)
	{
	}
	void VSceneLighting::AddLight(const std::shared_ptr<VLightSource>& lightSource)
	{
		_sceneLights.push_back(lightSource);
	}
	void VSceneLighting::SetLightUniforms(GLuint lPassProgram)
	{
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