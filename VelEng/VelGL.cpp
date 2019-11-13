#include <iostream>

#include "VelGL.h"

namespace Vel
{
	using namespace std;

	VelEng* VelEng::instance = nullptr;

	VelEng::VelEng()
	{
	}

    VelEng::~VelEng()
    {
        rendererTest.Destroy();
    }
 
    void VelEng::Init(const Settings &set)
    {
        InitGLFW();
        InitWindow();
        InitVulkan();
        InitCamera();

        auto keyFunc = []( GLFWwindow* window, int key, int scancode, int action, int mods )
        {
            VelEng::Instance()->GetKeyboard().KeyHandler( key, action, mods );
        };
        glfwSetKeyCallback( mainWindow->GetGLFWWindow(), keyFunc );

        auto mouseFunc = []( GLFWwindow* window, double x, double y )
        {
            VelEng::Instance()->GetMouse().SetCurrentPosition( x, y );
            auto posDif = VelEng::Instance()->GetMouse().GetPositionDifference();
            VelEng::Instance()->GetMainCamera()->Rotate( posDif.x / 2.0f, posDif.y / 2.0f, 0 );

            auto winSize = VelEng::Instance()->mainWindow->GetSize();
            auto lowerBoundaries = glm::ivec2{ 50, 50 };
            auto upperBoundaries = glm::ivec2{ winSize.x - 50, winSize.y - 50 };
            VelEng::Instance()->GetMouse().ResetIfOutside( window, lowerBoundaries, upperBoundaries );
        };
        glfwSetCursorPosCallback( mainWindow->GetGLFWWindow(), mouseFunc );

        auto mouseButtonFunc = []( GLFWwindow* window, int button, int action, int mods )
        {
            if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS )
                std::cout << "Button pressed" << std::endl;
        };
        glfwSetMouseButtonCallback( mainWindow->GetGLFWWindow(), mouseButtonFunc );
    }

    void VelEng::InitVulkan()
    {
        rendererTest.Init( mainWindow->GetGLFWWindow() ); 
    }

    void VelEng::InitGLFW()
    {
        int init = glfwInit();
        assert( init );
        glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    }

    void VelEng::InitWindow()
    {
        WindowInfo info( "Vulkan", glm::uvec2{ 100, 100 }, glm::uvec2{ 1366, 768 }, false );
        mainWindow = make_unique<Window>(info);
        auto size = mainWindow->GetSize();
        mouse.SetCurrentPosition( size.x / 2, size.y / 2 );
    }
    void VelEng::InitCamera()
    {
        mainCamera = std::make_shared<FreeCamera>();
        mainCamera->SetupProjection( 60, (float)(mainWindow->GetSize().x / (float)mainWindow->GetSize().y) );
        mainCamera->Update();
    }

	bool VelEng::AddShaderProgram(const string & Name, const string & VertFilename, const string & FragFilename)
	{
		auto shader = shaderPrograms.emplace(Name, make_shared<Shader>()).first->second;
		//shader->LoadFromFile(GL_VERTEX_SHADER, VertFilename); --TODO vulkanize
		//shader->LoadFromFile(GL_FRAGMENT_SHADER, FragFilename);
		shader->CreateAndLinkProgram();

		return true;
	}



	bool VelEng::AddShaderProgram(const string & Name, const string & VertFilename, const string & FragFilename, const string & GeoFilename)
	{
		auto shader = shaderPrograms.emplace(Name, make_shared<Shader>()).first->second;
		//shader->LoadFromFile(GL_VERTEX_SHADER, VertFilename); --TODO vulkanize
		//shader->LoadFromFile(GL_FRAGMENT_SHADER, FragFilename);
		//shader->LoadFromFile(GL_GEOMETRY_SHADER, GeoFilename);
		shader->CreateAndLinkProgram();

		return true;
	}

	const shared_ptr<Shader>& VelEng::GetShader(const string & name)
	{
		return shaderPrograms[name];
	}

	void VelEng::CreateScene(const string& Name)
	{
		scenes.emplace(Name, make_unique<Vel::Scene>());
	}

	void VelEng::AddModelToScene(const string & sceneName, const shared_ptr<Model>& modelPtr)
	{
		scenes[sceneName]->AddModel(modelPtr);
	}

	void VelEng::AddLightSourceToScene(const string & sceneName, const shared_ptr<LightSource>& lightSourcePtr)
	{
		scenes[sceneName]->AddLightSource(lightSourcePtr);
	}

	void VelEng::HandleInput()
	{
		glfwPollEvents();
		if (keyboard.IsKeyPressed(GLFW_KEY_W))
		{
			mainCamera->Walk(1);
		}
		if (keyboard.IsKeyPressed(GLFW_KEY_S))
		{
			mainCamera->Walk(-1);
		}
		if (keyboard.IsKeyPressed(GLFW_KEY_A))
		{
			mainCamera->Strafe(-1);
		}
		if (keyboard.IsKeyPressed(GLFW_KEY_D))
		{
			mainCamera->Strafe(1);
		}
		if (keyboard.IsKeyPressed(GLFW_KEY_SPACE))
		{
			mainCamera->Lift(1);
		}
		if (keyboard.IsKeyPressed(GLFW_KEY_Z))
		{
			mainCamera->Lift(-1);
		}
		if(keyboard.IsKeyPressed(GLFW_KEY_1))
			VelEng::Instance()->GetMouse().ChangeResetting();
		if (keyboard.IsKeyPressed(GLFW_KEY_ESCAPE))
			VelEng::Instance()->shouldRun = false;
	}

	//first forward rendering of skybox, then deffered rendering of rest
	void VelEng::RenderScenes()
	{
		renderer->BindGBufferForWriting(); //TODO separate from deferred
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); --TODO vulkanize
		scenes["Sky"]->DrawScene();
		renderer->UnbindGBufferForWriting();
		scenes["World"]->DrawShadows();
		renderer->SetScene(scenes["World"]);
		renderer->BindShadowMapReading();
		
		renderer->Render();

		glfwSwapBuffers(mainWindow->GetGLFWWindow());
	}

	//adjusts camera and vp matrices in shaders, then renders scenes
	void Vel::VelEng::RenderFrame()
	{	
		auto viewMat = mainCamera->GetViewMatrix(); //TODO adjust uniforms in signal
		auto projMat = mainCamera->GetProjectionMatrix(); //TODO setting projection once

		/*
		auto gPassShd = GetShader("GPass"); //TODO separate from deferred. Shaders inf renderer?
		gPassShd->Activate();
		gPassShd->SetUniformsValue(Uniform<glm::mat4>{ "V", vMat });
		gPassShd->SetUniformsValue(Uniform<glm::mat4>{ "P", pMat });
		gPassShd->Deactivate();

		auto lpass = GetShader("LPass");
		auto camPosition = mainCamera->GetPosition();
		scenes["World"]->SetCameraPosition(camPosition);
		lpass->Activate();
		lpass->SetUniformsValue(Uniform<glm::vec3>{"viewPos", camPosition});
		scenes["World"]->SetLPassLightUniforms(lpass->GetProgramID());
		lpass->Deactivate();



		auto skybox = GetShader("SkyboxShader");
		skybox->Activate();
		skybox->SetUniformsValue(Uniform<glm::mat4>{ "V", vMat });
		skybox->SetUniformsValue(Uniform<glm::mat4>{ "P", pMat });
		skybox->Deactivate();

		RenderScenes();*/

		rendererTest.UpdateCamera( viewMat, projMat );
		rendererTest.Draw();
	}

}
