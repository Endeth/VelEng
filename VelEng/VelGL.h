#pragma once
#include <unordered_map>
#include <map>
#include <memory>
#include <string>

#include "glew.h"
#include "glfw3.h"

#include "../../VelUti/VTime.h"

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


namespace Vel
{

	class VelEng
	{
		using RendererPtr = std::shared_ptr<VDefferedRenderer>;
		using ScenePtr = std::shared_ptr<VScene>;
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

		const std::shared_ptr<VGLSLShader>& GetShader(const std::string& name);
		const glm::ivec2& GetMainWindowSize() const { return _mainWindow->GetSize(); }
		std::shared_ptr<VFreeCamera>& GetMainCamera() { return _mainCamera; }
		VMouse& GetMouse() { return _mouse; }
		VKeyboard& GetKeyboard() { return _keyboard; }
		VFrameClock& GetFrameClock() { return _frameClock; }

		void CreateScene(const std::string& name);
		ScenePtr& GetScene(const std::string& name) { return _scenes[name]; }
		void AddModelToScene(const std::string & sceneName, const std::shared_ptr<VModel>& modelPtr);
		void AddLightSourceToScene(const std::string & sceneName, const std::shared_ptr<VLightSource>& lightSourcePtr);
		
		void InitWindow();
		void InitCamera();

		void HandleInput();
		void RenderScenes();
		void RenderFrame();
	
	private:
		VelEng();

		void GLFWInit();
		void GlewInit();
		
		std::unordered_map<std::string, std::shared_ptr<VGLSLShader>> _shaderPrograms;
		std::map<std::string, std::shared_ptr<VScene>> _scenes;

		std::shared_ptr<VFreeCamera> _mainCamera;
		std::shared_ptr<VWindow> _mainWindow;

		static VelEng* _instance;

		RendererPtr _renderer;
		VFrameClock _frameClock;
		VMouse _mouse;
		VKeyboard _keyboard;
		bool _shouldRun{ true };
	};

}