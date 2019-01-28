#include <iostream>

#include "VelGL.h"

namespace Vel
{
	using namespace std;

	VelEng* VelEng::_instance = nullptr;

	VelEng::VelEng()
	{
	}

    VelEng::~VelEng()
    {
        _vulkan.Destroy();
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
        glfwSetKeyCallback( _mainWindow->GetGLFWWindow(), keyFunc );

        auto mouseFunc = []( GLFWwindow* window, double x, double y )
        {
            VelEng::Instance()->GetMouse().SetCurrentPosition( x, y );
            auto posDif = VelEng::Instance()->GetMouse().GetPositionDifference();
            VelEng::Instance()->GetMainCamera()->Rotate( posDif.x / 2.0f, posDif.y / 2.0f, 0 );

            auto winSize = VelEng::Instance()->_mainWindow->GetSize();
            auto lowerBoundaries = glm::ivec2{ 50, 50 };
            auto upperBoundaries = glm::ivec2{ winSize.x - 50, winSize.y - 50 };
            VelEng::Instance()->GetMouse().ResetIfOutside( window, lowerBoundaries, upperBoundaries );
        };
        glfwSetCursorPosCallback( _mainWindow->GetGLFWWindow(), mouseFunc );

        auto mouseButtonFunc = []( GLFWwindow* window, int button, int action, int mods )
        {
            if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS )
                std::cout << "Button pressed" << std::endl;
        };
        glfwSetMouseButtonCallback( _mainWindow->GetGLFWWindow(), mouseButtonFunc );
    }

    void VelEng::InitVulkan()
    {
        _vulkan.Init( _mainWindow->GetGLFWWindow() ); 
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
        _mainWindow = make_unique<Window>(info);
        auto size = _mainWindow->GetSize();
        _mouse.SetCurrentPosition( size.x / 2, size.y / 2 );
    }
    void VelEng::InitCamera()
    {
        _mainCamera = std::make_shared<FreeCamera>();
        _mainCamera->SetupProjection( 60, (float)(_mainWindow->GetSize().x / (float)_mainWindow->GetSize().y) );
        _mainCamera->Update();
    }

	bool VelEng::AddShaderProgram(const string & Name, const string & VertFilename, const string & FragFilename)
	{
		auto shader = _shaderPrograms.emplace(Name, make_shared<Shader>()).first->second;
		//shader->LoadFromFile(GL_VERTEX_SHADER, VertFilename); --TODO vulkanize
		//shader->LoadFromFile(GL_FRAGMENT_SHADER, FragFilename);
		shader->CreateAndLinkProgram();

		return true;
	}



	bool VelEng::AddShaderProgram(const string & Name, const string & VertFilename, const string & FragFilename, const string & GeoFilename)
	{
		auto shader = _shaderPrograms.emplace(Name, make_shared<Shader>()).first->second;
		//shader->LoadFromFile(GL_VERTEX_SHADER, VertFilename); --TODO vulkanize
		//shader->LoadFromFile(GL_FRAGMENT_SHADER, FragFilename);
		//shader->LoadFromFile(GL_GEOMETRY_SHADER, GeoFilename);
		shader->CreateAndLinkProgram();

		return true;
	}

	const shared_ptr<Shader>& VelEng::GetShader(const string & name)
	{
		return _shaderPrograms[name];
	}

	void VelEng::CreateScene(const string& Name)
	{
		_scenes.emplace(Name, make_unique<Vel::Scene>());
	}

	void VelEng::AddModelToScene(const string & sceneName, const shared_ptr<Model>& modelPtr)
	{
		_scenes[sceneName]->AddModel(modelPtr);
	}

	void VelEng::AddLightSourceToScene(const string & sceneName, const shared_ptr<LightSource>& lightSourcePtr)
	{
		_scenes[sceneName]->AddLightSource(lightSourcePtr);
	}

	void VelEng::HandleInput()
	{
		glfwPollEvents();
		if (_keyboard.IsKeyPressed(GLFW_KEY_W))
		{
			_mainCamera->Walk(1);
		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_S))
		{
			_mainCamera->Walk(-1);
		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_A))
		{
			_mainCamera->Strafe(-1);
		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_D))
		{
			_mainCamera->Strafe(1);
		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_SPACE))
		{
			_mainCamera->Lift(1);
		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_Z))
		{
			_mainCamera->Lift(-1);
		}
		if(_keyboard.IsKeyPressed(GLFW_KEY_1))
			VelEng::Instance()->GetMouse().ChangeResetting();
		if (_keyboard.IsKeyPressed(GLFW_KEY_ESCAPE))
			VelEng::Instance()->_shouldRun = false;
	}

	//first forward rendering of skybox, then deffered rendering of rest
	void VelEng::RenderScenes()
	{
		_renderer->BindGBufferForWriting(); //TODO separate from deferred
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); --TODO vulkanize
		_scenes["Sky"]->DrawScene();
		_renderer->UnbindGBufferForWriting();
		_scenes["World"]->DrawShadows();
		_renderer->SetScene(_scenes["World"]);
		_renderer->BindShadowMapReading();
		
		_renderer->Render();

		glfwSwapBuffers(_mainWindow->GetGLFWWindow());
	}

	//adjusts camera and vp matrices in shaders, then renders scenes
	void Vel::VelEng::RenderFrame()
	{	
		/*auto vMat = _mainCamera->GetViewMatrix(); //TODO adjust uniforms in signal
		auto pMat = _mainCamera->GetProjectionMatrix();

		auto gPassShd = GetShader("GPass"); //TODO separate from deferred. Shaders inf renderer?
		gPassShd->Activate();
		gPassShd->SetUniformsValue(Uniform<glm::mat4>{ "V", vMat });
		gPassShd->SetUniformsValue(Uniform<glm::mat4>{ "P", pMat });
		gPassShd->Deactivate();

		auto lpass = GetShader("LPass");
		auto camPosition = _mainCamera->GetPosition();
		_scenes["World"]->SetCameraPosition(camPosition);
		lpass->Activate();
		lpass->SetUniformsValue(Uniform<glm::vec3>{"viewPos", camPosition});
		_scenes["World"]->SetLPassLightUniforms(lpass->GetProgramID());
		lpass->Deactivate();



		auto skybox = GetShader("SkyboxShader");
		skybox->Activate();
		skybox->SetUniformsValue(Uniform<glm::mat4>{ "V", vMat });
		skybox->SetUniformsValue(Uniform<glm::mat4>{ "P", pMat });
		skybox->Deactivate();

		RenderScenes();*/
		_vulkan.Draw();
	}

}
