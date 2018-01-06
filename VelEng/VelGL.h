#pragma once
#include <unordered_map>
#include <map>
#include <memory>
#include <string>

#include "external/glfw/glfw3.h"

#include "../VelUti/VTime.h"

#include "vShaders/vGLSLShader.h"
#include "vWindow/vWindow.h"
#include "vUti/VelEngUti.h"
#include "vDrawable/vScene.h"
#include "vDrawable/vModel.h"
#include "vDrawable/vSky.h"
#include "vCameras/vCamera.h"
#include "vTextures/vTexture.h"
#include "vRendering/vRenderer.h"
#include "vRendering/vFramebuffer.h"
#include "vLights/vLight.h"
#include "vAssets/vAssets.h"

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
        static VelEng* _instance;
		VelEng();

		void GLFWInit();
        void VulkanInit();

		std::unordered_map<std::string, std::shared_ptr<Shader>> _shaderPrograms;
		std::map<std::string, std::shared_ptr<Scene>> _scenes;

		std::shared_ptr<FreeCamera> _mainCamera;
		std::shared_ptr<Window> _mainWindow;

        AssetsManager _assetsMgr;

		RendererPtr _renderer;
		FrameClock _frameClock; //TODO - different clock for logic?
        //InputrMgr - TODO implement
		Mouse _mouse;
		Keyboard _keyboard;
		bool _shouldRun{ true }; //TODO something different
	};

}