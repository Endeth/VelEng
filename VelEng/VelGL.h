#pragma once
#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE

#include <unordered_map>
#include <map>
#include <memory>
#include <string>

#include "VelEngConfig.h"
#include "external/glfw/glfw3.h"

#include "../VelUti/VTime.h"

#include <vulkan/vulkan.hpp>
#include "vAssets/vAssets.h"
#include "vGraphics/vVulkan/vVulkan.h"
#include "vGraphics/vShaders/vGLSLShader.h"
#include "vGraphics/vWindow/vWindow.h"
#include "vGraphics/vDrawable/vScene.h"
#include "vGraphics/vDrawable/vModel.h"
#include "vGraphics/vDrawable/vSky.h"
#include "vGraphics/vCameras/vCamera.h"
#include "vGraphics/vTextures/vTexture.h"
#include "vGraphics/vRendering/vRenderer.h"
#include "vGraphics/vRendering/vFramebuffer.h"
#include "vGraphics/vLights/vLight.h"
#include "vUti/VelEngUti.h"

namespace Vel
{

	class VelEng
	{
		using RendererPtr = std::shared_ptr<DefferedRenderer>;
		using ScenePtr = std::shared_ptr<Scene>;
	public:
        struct Settings //TODO
        {

        };
		static VelEng* Instance()
		{
			if (!_instance)
				_instance = new VelEng;
			return _instance;
		}
        static void Destroy()
        {
            delete _instance;
            glfwTerminate();
        }
		VelEng(const VelEng&) = delete;
        ~VelEng();
        void Init(const Settings &set);

		const bool ShouldRun() const { return _shouldRun; }
		void SetRenderer(const RendererPtr& renderer) { _renderer = renderer; }
		bool AddShaderProgram(const std::string& name, const std::string& vertFilename, const std::string& fragFilename);
		bool AddShaderProgram(const std::string& name, const std::string& vertFilename, const std::string& fragFilename, const std::string& geoFilename);

		const std::shared_ptr<Shader>& GetShader(const std::string& name);

		void CreateScene(const std::string& name);
		ScenePtr& GetScene(const std::string& name) { return _scenes[name]; }
		void AddModelToScene(const std::string & sceneName, const std::shared_ptr<Model>& modelPtr);
		void AddLightSourceToScene(const std::string & sceneName, const std::shared_ptr<LightSource>& lightSourcePtr);


		void HandleInput();
		void RenderScenes();
		void RenderFrame();


        const glm::ivec2& GetMainWindowSize() const { return _mainWindow->GetSize(); }
        std::shared_ptr<FreeCamera>& GetMainCamera() { return _mainCamera; }
        Mouse& GetMouse() { return _mouse; }
        Keyboard& GetKeyboard() { return _keyboard; }
        FrameClock& GetFrameClock() { return _frameClock; }

	private:
        static VelEng* _instance;
		VelEng();

        void InitVulkan();
        void InitWindow();
        void InitCamera();
		void InitGLFW();

        Vulkan _vulkan;
		std::unordered_map<std::string, std::shared_ptr<Shader>> _shaderPrograms;
		std::map<std::string, std::shared_ptr<Scene>> _scenes;

		std::shared_ptr<FreeCamera> _mainCamera;
		std::unique_ptr<Window> _mainWindow;

        AssetsManager _assetsMgr;

		RendererPtr _renderer;
		FrameClock _frameClock; //TODO - different clock for logic?
        //InputrMgr - TODO implement
		Mouse _mouse;
		Keyboard _keyboard;
		bool _shouldRun{ true }; //TODO something different
	};

}