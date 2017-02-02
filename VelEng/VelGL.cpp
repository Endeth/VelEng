
#include "VelGL.h"
#include <iostream>

namespace Vel
{
	using namespace std;

	void ChangeRenderMode(std::shared_ptr<VGLSLShader> &shader, int mode)
	{
		shader->Activate();
		shader->SetUniformsValue(Uniform<int>{ "renderMode", mode});
		shader->Deactivate();
	}

	VelEng* VelEng::_instance = nullptr;

	VelEng::VelEng()
	{

		GLFWInit();
		InitWindow();
		GlewInit();
		InitCamera();

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		auto err = glGetError(); //1280 err

		auto keyFunc = [](GLFWwindow* window, int key, int scancode, int action, int mods) 
		{
			VelEng::Instance()->GetKeyboard().KeyHandler(key, action, mods);
		};
		glfwSetKeyCallback(_mainWindow->GetGLFWWindow(), keyFunc);

		auto mouseFunc = [](GLFWwindow* window, double x, double y)
		{
			VelEng::Instance()->GetMouse().SetCurrentPosition(x, y);
			auto posDif = VelEng::Instance()->GetMouse().GetPositionDifference();
			VelEng::Instance()->GetMainCamera()->Rotate(posDif.x / 2.0f, posDif.y / 2.0f, 0);

			auto winSize = VelEng::Instance()->_mainWindow->GetSize();
			auto lowerBoundaries = glm::ivec2{ 50, 50 };
			auto upperBoundaries = glm::ivec2{ winSize.x - 50, winSize.y - 50 };
			VelEng::Instance()->GetMouse().ResetIfOutside(window, lowerBoundaries, upperBoundaries);
		};
		glfwSetCursorPosCallback(_mainWindow->GetGLFWWindow(), mouseFunc);

		auto mouseButtonFunc = [](GLFWwindow* window, int button, int action, int mods)
		{
			if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
				std::cout << "Button pressed" << std::endl;
		};
		glfwSetMouseButtonCallback(_mainWindow->GetGLFWWindow(), mouseButtonFunc);
	}

	
	bool VelEng::AddShaderProgram(const string & Name, const string & VertFilename, const string & FragFilename)
	{
		auto shader = _shaderPrograms.emplace(Name, make_shared<VGLSLShader>()).first->second;
		shader->LoadFromFile(GL_VERTEX_SHADER, VertFilename);
		shader->LoadFromFile(GL_FRAGMENT_SHADER, FragFilename);
		shader->CreateAndLinkProgram();

		return true;
	}



	bool VelEng::AddShaderProgram(const string & Name, const string & VertFilename, const string & FragFilename, const string & GeoFilename)
	{
		auto shader = _shaderPrograms.emplace(Name, make_shared<VGLSLShader>()).first->second;
		shader->LoadFromFile(GL_VERTEX_SHADER, VertFilename);
		shader->LoadFromFile(GL_FRAGMENT_SHADER, FragFilename);
		shader->LoadFromFile(GL_GEOMETRY_SHADER, GeoFilename);
		shader->CreateAndLinkProgram();

		return true;
	}

	const shared_ptr<VGLSLShader>& VelEng::GetShader(const string & name)
	{
		return _shaderPrograms[name];
	}

	void VelEng::CreateScene(const string& Name)
	{
		_scenes.emplace(Name, make_unique<Vel::VScene>());
	}

	void VelEng::AddModelToScene(const string & sceneName, const shared_ptr<VModel>& modelPtr)
	{
		_scenes[sceneName]->AddModel(modelPtr);
	}

	void VelEng::AddLightSourceToScene(const string & sceneName, const shared_ptr<VLightSource>& lightSourcePtr)
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
		if (_keyboard.IsKeyPressed(GLFW_KEY_UP))
		{

		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_DOWN))
		{

		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_1))
		{
			ChangeRenderMode(_shaderPrograms["LPass"], 0);
		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_2))
		{
			ChangeRenderMode(_shaderPrograms["LPass"], 1);
		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_3))
		{
			ChangeRenderMode(_shaderPrograms["LPass"], 2);
		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_4))
		{
			ChangeRenderMode(_shaderPrograms["LPass"], 3);
		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_5))
		{
			ChangeRenderMode(_shaderPrograms["LPass"], 4);
		}
		if (_keyboard.IsKeyPressed(GLFW_KEY_6))
		{
			ChangeRenderMode(_shaderPrograms["LPass"], 5);
		}
		if(_keyboard.IsKeyPressed(GLFW_KEY_F1))
			VelEng::Instance()->GetMouse().ChangeResetting();
		if (_keyboard.IsKeyPressed(GLFW_KEY_ESCAPE))
			VelEng::Instance()->_shouldRun = false;
	}

	//first forward rendering of skybox, then deffered rendering of rest
	void VelEng::RenderScenes()
	{
		_renderer->BindGBufferForWriting();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		_scenes["Sky"]->DrawScene();
		_renderer->UnbindGBufferForWriting();
		_renderer->SetScene(_scenes["World"]);
		_renderer->Render();
		glfwSwapBuffers(_mainWindow->GetGLFWWindow());
	}

	//adjusts camera and vp matrices in shaders, then renders scenes
	void Vel::VelEng::RenderFrame()
	{
		auto vMat = _mainCamera->GetViewMatrix();
		auto pMat = _mainCamera->GetProjectionMatrix();

		auto gPassShd = VelEng::Instance()->GetShader("GPass");
		gPassShd->Activate();
		gPassShd->SetUniformsValue(Uniform<glm::mat4>{ "V", vMat });
		gPassShd->SetUniformsValue(Uniform<glm::mat4>{ "P", pMat });
		gPassShd->Deactivate();

		auto lpass = VelEng::Instance()->GetShader("LPass");
		auto camPosition = VelEng::Instance()->_mainCamera->GetPosition();
		lpass->Activate();
		lpass->SetUniformsValue(Uniform<glm::vec3>{"viewPos", camPosition});
		_scenes["World"]->SetLightUniforms(lpass->GetID());
		lpass->Deactivate();


		auto skybox = VelEng::Instance()->GetShader("SkyboxShader");
		skybox->Activate();
		skybox->SetUniformsValue(Uniform<glm::mat4>{ "V", vMat });
		skybox->SetUniformsValue(Uniform<glm::mat4>{ "P", pMat });
		skybox->Deactivate();

		RenderScenes();
	}



	void VelEng::GLFWInit()
	{
		int init = glfwInit();
		assert(init);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);		
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	}

	void VelEng::GlewInit()
	{
		glewExperimental = GL_TRUE;
		GLenum Err = glewInit();
		if (GLEW_OK != Err)
		{
			cerr << "Error: " << glewGetErrorString(Err) << endl;
		}
	}

	void VelEng::InitWindow()
	{
		_mainWindow = make_shared<VWindow>();
		auto size = _mainWindow->GetSize();
		_mouse.SetCurrentPosition(size.x / 2, size.y / 2);
	}
	void VelEng::InitCamera()
	{
		_mainCamera = std::make_shared<VFreeCamera>();
		_mainCamera->SetupProjection(60, (GLfloat)(_mainWindow->GetSize().x / (GLfloat)_mainWindow->GetSize().y));
		_mainCamera->Update();
	}
}
