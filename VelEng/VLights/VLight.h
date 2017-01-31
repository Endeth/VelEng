#pragma once
#include "glew.h"
#include "glm/glm.hpp"
#include "../../VShaders/VGLSLShader.h"
#include "../../VRendering/VFramebuffer.h"
#include <memory>


namespace Vel
{

	class VLightSource
	{
	public:
		class VLightColor
		{
		public:
			VLightColor(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
			const glm::vec3& GetAmbient() const { return _ambient; }
			const glm::vec3& GetDiffuse() const { return _diffuse; }
			const glm::vec3& GetSpecular() const { return _specular; }
		private:
			glm::vec3 _ambient;
			glm::vec3 _diffuse;
			glm::vec3 _specular;
		};

		VLightSource(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
		VLightSource(const VLightColor& colors);
		const VLightColor& GetColors() const { return _colors; }

	protected:
		VLightColor _colors;
	};

	class VDynamicLight : public VLightSource
	{
	protected:
		using ShaderPtr = std::shared_ptr<VGLSLShader>;
		using ShadowMapPtr = std::unique_ptr<VShadowMap>;
	public:
		VDynamicLight(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
		VDynamicLight(const VDynamicLight& colors);
		void SetShader(const ShaderPtr& shader); //TODO change to static shaders?
	protected:
		void CreateShadowMap();

		ShaderPtr _lightShader;
		ShadowMapPtr _shadowMap;
	};

	class VPointLight : public VLightSource
	{
	public:
		VPointLight(const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
		VPointLight(const glm::vec3& position, const VLightColor& colors);
		const glm::vec3& GetPosition() const { return _position; }
		
	protected:
		glm::vec3 _position;
	};

	class VDirectionalLight : public VLightSource
	{
	public:
		VDirectionalLight(const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
		VDirectionalLight(const glm::vec3& direction, const VLightColor& colors);
		const glm::vec3& GetDirection() const { return _direction; }
	protected:
		glm::vec3 _direction;
	};

	class VSpotLight : public VPointLight, public VDirectionalLight
	{

	};

	class VSceneLighting
	{

	};
}