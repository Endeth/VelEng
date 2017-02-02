#pragma once
#include "../VOpenGL/glew.h"
#include "../VOpenGL/glm/glm.hpp"
#include "../VShaders/VGLSLShader.h"
#include "../VRendering/VFramebuffer.h"
#include <memory>
#include <list>


namespace Vel
{
	class VLightSource
	{
	protected:
		using ShaderPtr = std::shared_ptr<VGLSLShader>;
		using ShadowMapFBOPtr = std::unique_ptr<VShadowMap>;
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
		VLightSource(const VLightColor& color);
		void ActivateShader() { _depthShader->Activate(); }
		void DeactivateShader() { _depthShader->Deactivate(); }
		virtual void SetLightUniforms(GLuint lPassProgram) = 0;
		//virtual void BindShadowMapForWriting() = 0; //TODO
		void BindShadowMapForReading();
		void UnbindShadowMapForReading();

		void SetShader(const ShaderPtr& shader) { _depthShader = shader; }
		const VLightColor& GetColor() const { return _color; }
		const ShaderPtr& GetShader() const { return _depthShader; } //add default depth shader? might want to add cube depth
	protected:
		VLightColor _color;
		ShaderPtr _depthShader;
		ShadowMapFBOPtr _shadowMap;
	};

	/*class VDynamicLight : public VLightSource
	{

	public:
		VDynamicLight(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
		VDynamicLight(const VDynamicLight& colors);
		 //TODO change to static shaders?
	protected:
		void CreateShadowMap();


	};

	class VStaticLight : public VLightSource
	{

	};

	*/

	class VPointLight : public VLightSource
	{
		/*class LightsPool
		{
		};*/
	public:
		VPointLight(const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
		VPointLight(const glm::vec3& position, const VLightColor& colors);
		
		void SetLightUniforms(GLuint lPassProgram);

		const glm::vec3& GetPosition() const { return _position; }
		void SetPosition(const glm::vec3 &pos) { _position = pos; }
		void SetLightID(int id); //TODO debug func
		
	protected:
		glm::vec3 _position;
		GLfloat _constant;
		GLfloat _linear;
		GLfloat _quadratic;
		GLuint _id;
	private:

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
	public:
		void DrawSceneShadows() {}; //TODO
		void CleanUpLights() {}; //TODO
		void AddLight(const std::shared_ptr<VLightSource>& lightSource);

		//requires active shader
		void SetLightUniforms(GLuint lPassProgram);
	private:
		glm::vec3 _ambientLight;
		std::list<std::shared_ptr<VLightSource>> _sceneLights;
	};
}