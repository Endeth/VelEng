#pragma once
#include <unordered_map>
#include <map>
#include <memory>
#include <string>

#include "VOpenGL/../VOpenGL/glew.h"
#include "VOpenGL/glfw3.h"

#include "../VelUti/VTime.h"

#include "VShaders/VGLSLShader.h"
#include "VWindow/VWindow.h"
#include "VUti/VelEngUti.h"
#include "VDrawable/VScene.h"
#include "VDrawable/VModel.h"
#include "VDrawable/VSky.h"
#include "VCameras/VCamera.h"
#include "VTextures/VTexture.h"
#include "VRendering/VRenderer.h"
#include "VRendering/VFramebuffer.h"
#include "VLights/VLight.h"

#include "VelEngConfig.h"

namespace Vel
{

	class VelEng
	{
		using RendererPtr = std::shared_ptr<DefferedRenderer>;
		using ScenePtr = std::shared_ptr<Scene>;
	public:
		static VelEng* Instance()
		{
			if (!_instance)
				_instance = new VelEng;
			return _instance;
		}
		VelEng(const VelEng&) = delete;
		
		const bool ShouldRun() const { return _shouldRun; }
		void SetRenderer(const RendererPtr& renderer) { _renderer = renderer; }
		bool AddShaderProgram(const std::string& name, const std::string& vertFilename, const std::string& fragFilename);
		bool AddShaderProgram(const std::string& name, const std::string& vertFilename, const std::string& fragFilename, const std::string& geoFilename);

		const std::shared_ptr<Shader>& GetShader(const std::string& name);
		const glm::ivec2& GetMainWindowSize() const { return _mainWindow->GetSize(); }
		std::shared_ptr<FreeCamera>& GetMainCamera() { return _mainCamera; }
		Mouse& GetMouse() { return _mouse; }
		Keyboard& GetKeyboard() { return _keyboard; }
		FrameClock& GetFrameClock() { return _frameClock; }

		void CreateScene(const std::string& name);
		ScenePtr& GetScene(const std::string& name) { return _scenes[name]; }
		void AddModelToScene(const std::string & sceneName, const std::shared_ptr<Model>& modelPtr);
		void AddLightSourceToScene(const std::string & sceneName, const std::shared_ptr<LightSource>& lightSourcePtr);
		
		void InitWindow();
		void InitCamera();

		void HandleInput();
		void RenderScenes();
		void RenderFrame();
	
	private:
		VelEng();

		void GLFWInit();
		void GlewInit();
		
		std::unordered_map<std::string, std::shared_ptr<Shader>> _shaderPrograms;
		std::map<std::string, std::shared_ptr<Scene>> _scenes;

		std::shared_ptr<FreeCamera> _mainCamera;
		std::shared_ptr<Window> _mainWindow;

		static VelEng* _instance;

		RendererPtr _renderer;
		FrameClock _frameClock;
		Mouse _mouse;
		Keyboard _keyboard;
		bool _shouldRun{ true };
	};

}