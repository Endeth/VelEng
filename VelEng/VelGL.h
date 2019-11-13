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
#include "vInput/vInput.h"
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
#include "vScripts/vScripts.h"
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
			if (!instance)
				instance = new VelEng;
			return instance;
		}
        static void Destroy()
        {
            delete instance;
            glfwTerminate();
        }
		VelEng(const VelEng&) = delete;
        ~VelEng();
        void Init(const Settings &set);

		const bool ShouldRun() const { return shouldRun; }
		void SetRenderer(const RendererPtr& rnd ) { renderer = rnd; }
		bool AddShaderProgram(const std::string& name, const std::string& vertFilename, const std::string& fragFilename);
		bool AddShaderProgram(const std::string& name, const std::string& vertFilename, const std::string& fragFilename, const std::string& geoFilename);

		const std::shared_ptr<Shader>& GetShader(const std::string& name);

		void CreateScene(const std::string& name);
		ScenePtr& GetScene(const std::string& name) { return scenes[name]; }
		void AddModelToScene(const std::string & sceneName, const std::shared_ptr<Model>& modelPtr);
		void AddLightSourceToScene(const std::string & sceneName, const std::shared_ptr<LightSource>& lightSourcePtr);


		void HandleInput();
		void RenderScenes();
		void RenderFrame();


        const glm::ivec2& GetMainWindowSize() const { return mainWindow->GetSize(); }
        std::shared_ptr<FreeCamera>& GetMainCamera() { return mainCamera; }
        Mouse& GetMouse() { return mouse; }
        Keyboard& GetKeyboard() { return keyboard; }
        FrameClock& GetFrameClock() { return frameClock; }

	private:
        static VelEng* instance;
		VelEng();

        void InitVulkan();
        void InitWindow();
        void InitCamera();
		void InitGLFW();

        VulkanRenderer rendererTest;
		std::unordered_map<std::string, std::shared_ptr<Shader>> shaderPrograms;
		std::map<std::string, std::shared_ptr<Scene>> scenes;

		std::shared_ptr<FreeCamera> mainCamera;
		std::unique_ptr<Window> mainWindow;

        AssetsManager assetsMgr;

		RendererPtr renderer;
		FrameClock frameClock; //TODO - different clock for logic?
        //InputrMgr - TODO implement
		Mouse mouse;
		Keyboard keyboard;
		bool shouldRun{ true }; //TODO something different
	};

}