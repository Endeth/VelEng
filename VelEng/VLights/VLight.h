#pragma once
#include "../VOpenGL/glew.h"
#include "../VOpenGL/glm/glm.hpp"
#include "../VShaders/VGLSLShader.h"
#include "../VRendering/VFramebuffer.h"
#include "../VOpenGL/glm/gtc/matrix_transform.hpp"
#include "../VDrawable/VModel.h"
#include <memory>
#include <list>


namespace Vel
{
	class LightSource //TODO make a generic class GEOMETRY OBJECT or something like that
	{
	protected:
		using ShaderPtr = std::shared_ptr<Shader>;
		using ShadowMapFBOPtr = std::unique_ptr<ShadowMap2D>;
	public:
		class LightColor
		{

		public:
			LightColor(const glm::vec3& diffuse, const glm::vec3& specular);
			const glm::vec3& GetDiffuse() const { return _diffuse; }
			const glm::vec3& GetSpecular() const { return _specular; }
		private:
			glm::vec3 _diffuse;
			glm::vec3 _specular;
		};

		LightSource(const glm::vec3& diffuse, const glm::vec3& specular); 
		LightSource(const LightColor& color);
		void ActivateShader() { _depthShader->Activate(); }
		void DeactivateShader() { _depthShader->Deactivate(); }

		/*Sets uniforms in light using shader
		Pass iterator from VSceneLights to uniformID when setting uniforms*/
		virtual void SetLPassLightUniforms(GLuint program, GLuint uniformID) = 0;
		//Sets uniforms in shadow map generating shader
		virtual void SetShadowUniforms();

		virtual void SetTextureUnit(GLuint textureUnit);
		virtual void BindShadowMapForWriting();
		virtual void UnbindShadowMapForWriting(); 
		virtual void BindShadowMapForReading();
		virtual void UnbindShadowMapForReading();

		void SetShader(const ShaderPtr& shader) { _depthShader = shader; }
		const LightColor& GetColor() const { return _color; }
		const ShaderPtr& GetShader() const { return _depthShader; }
		const glm::ivec2& GetShadowResolution() const { return _shadowResolution; }
	protected:
		LightColor _color;
		ShaderPtr _depthShader;
		ShadowMapFBOPtr _shadowMap;
		glm::mat4 _lightSpaceMatrix;
		glm::ivec2 _shadowResolution;
	};

	class PointLight : public LightSource
	{
	public:
		PointLight(const glm::vec3& position, const glm::vec3& diffuse, const glm::vec3& specular); //no reason to keep ambient
		PointLight(const glm::vec3& position, const LightColor& colors);
		

		virtual void SetLPassLightUniforms(GLuint program, GLuint uniformID) override;
		virtual void SetShadowUniforms() override;

		//sets position and recalculates shadow transforms matrices - might want to set those apart
		void SetPosition(const glm::vec3 &pos);
		const glm::vec3& GetPosition() const { return _position; }

		virtual void SetTextureUnit(GLuint textureUnit) override { _shadowMap->SetTextureUnit(textureUnit); }
		virtual void BindShadowMapForWriting() override { _shadowMap->BindFBOWriting(); }
		virtual void UnbindShadowMapForWriting() override { _shadowMap->UnbindFBOWriting(); }
		virtual void BindShadowMapForReading() override { _shadowMap->BindTexturesReading(); }
		virtual void UnbindShadowMapForReading() override { _shadowMap->UnbindTexturesReading(); }
		
	protected:
		glm::vec3 _position;
		GLfloat _constant;
		GLfloat _linear;
		GLfloat _quadratic;
		
	private:
		std::vector<glm::mat4> _shadowTransforms;
		std::unique_ptr<ShadowMapCube> _shadowMap;
		void UpdateShadowTransforms();
	};

	class DirectionalLight : public LightSource
	{
	public:
		DirectionalLight(const glm::vec3& direction, const glm::vec3& diffuse, const glm::vec3& specular);
		DirectionalLight(const glm::vec3& direction, const LightColor& colors);

		virtual void SetLPassLightUniforms(GLuint program);
		virtual void SetShadowUniforms() override;

		//sets direction and recalculates shadow transforms matrices - might want to set those apart
		void SetDirection(const glm::vec3 &dir);
		const glm::vec3& GetDirection() const { return _direction; }

		void SetCameraPosition(const glm::vec3 &pos);
	protected:
		glm::vec3 _direction;
	private:
		glm::vec3 _camPosition;
		glm::vec3 _shadowCastingPosition;
		virtual void SetLPassLightUniforms(GLuint program, GLuint uniformID) override {}; //not needed
		void UpdateShadowTransforms(); //TODO - implement directional light following camera
	};

	//not yet implemented
	class SpotLight : public LightSource
	{
	public:
		SpotLight(const glm::vec3& direction, const glm::vec3& diffuse, const glm::vec3& specular);
		SpotLight(const glm::vec3& direction, const LightColor& colors);

		virtual void SetLPassLightUniforms(GLuint program, GLuint uniformID) override;
		virtual void SetShadowUniforms() override;

		//sets direction and recalculates shadow transforms matrices - might want to set those apart
		void SetDirection(const glm::vec3 &dir) { _direction = dir; }
		const glm::vec3& GetDirection() const { return _direction; }
	protected:
		glm::vec3 _position;
		glm::vec3 _direction;
		GLfloat _constant;
		GLfloat _linear;
		GLfloat _quadratic;
	private:
		void UpdateShadowTransforms();
	};

	class SceneLighting
	{
	/*
		maybe light sorting? then calculating first X lights, same principle with shadows?
	*/
	public:

		void CleanUpLights() {}; //TODO
		void AddLight(const std::shared_ptr<LightSource>& lightSource);
		void CreateDirectionalLight(const glm::vec3 &dir, const LightSource::LightColor &color);
		void CreateDirectionalLight(std::unique_ptr<DirectionalLight> &&light);

		//creates shadowmap where camera position is center
		void DrawDirectionalLightShadowMap(const glm::vec3 &cameraPosition);
		//Draws MAX 4 shadow maps
		void DrawSceneShadows(const std::vector<std::shared_ptr<Model>>& models);

		/*Activates first 4 (or less if there aren't that many textures) unit textures
		from GL_TEXTURE4 to GL_TEXTURE7 and binds appropriate textures*/
		void ActivateShadowMaps();

		//requires active shader
		void SetLPassLightUniforms(GLuint lPassProgram);
		void SetAmbientLight(const glm::vec3 &amb) { _ambientLight = amb; }
		void SetCameraPosition(const glm::vec3 &pos);
	private:
		glm::vec3 _cameraPosition; //TODO change to pointer
		glm::vec3 _ambientLight{0.1,0.1,0.1};
		std::unique_ptr<DirectionalLight> _directionalLight;
		std::list<std::shared_ptr<LightSource>> _sceneLights;
	};
}