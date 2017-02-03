// VelTest.cpp : Defines the entry point for the console application.
//

#include "../VelEng/VelGL.h"
#include "../VelEng/VUti/VelEngUti.h"
#include "../VelEng/VLights/VLight.h"
#include "../VelEng/VMaterials/VMaterial.h"
#include <functional>
#include "../VelUti/VTime.h"
//#include "json.hpp"

using namespace Vel;

glm::vec3 SunlightDirection{ -1.0, -0.35, 0.4 };

void BasicShaderSetUp()
{
	VelEng::Instance()->AddShaderProgram("BasicShader", "Resources\\Shaders\\BasicVertex.vert", "Resources\\Shaders\\BasicFragment.frag");
	auto BasicShdr = VelEng::Instance()->GetShader("BasicShader");
	BasicShdr->SetAttributes({ "vVertex", "vNormal", "vUV" });
	BasicShdr->SetUniforms({ "Time", "M", "V", "P", "viewPos" });
	BasicShdr->SetUniforms({ "material.diffuse", "material.specular", "material.shininess" });
	BasicShdr->SetUniforms({ "light.ambient", "light.diffuse", "light.specular", "light.direction" });


	BasicShdr->Activate();

	BasicShdr->SetUniformsValue(Uniform<int>{ "material.diffuse", 0});
	BasicShdr->SetUniformsValue(Uniform<int>{ "material.specular", 1});
	BasicShdr->SetUniformsValue(Uniform<float>{ "material.shininess", 32.0f});

	BasicShdr->Deactivate();
}

void DefShaderSetUp()
{

	auto EngineInstance = VelEng::Instance();
	EngineInstance->AddShaderProgram("SkyboxShader", "Resources\\Shaders\\Deffered\\Skybox.vert", "Resources\\Shaders\\Deffered\\Skybox.frag");
	EngineInstance->AddShaderProgram("GPass", "Resources\\Shaders\\Deffered\\GPass.vert", "Resources\\Shaders\\Deffered\\GPass.frag");
	EngineInstance->AddShaderProgram("LPass", "Resources\\Shaders\\Deffered\\LPass.vert", "Resources\\Shaders\\Deffered\\LPass.frag");

	auto SkyboxShader = VelEng::Instance()->GetShader("SkyboxShader");
	auto GPassShader = VelEng::Instance()->GetShader("GPass");
	auto LPassShader = VelEng::Instance()->GetShader("LPass");

	SkyboxShader->SetAttributes("vVertex");
	SkyboxShader->SetUniforms({ "M", "V", "P" });
	GPassShader->SetAttributes({ "vVertex", "vNormal", "vUV" });
	GPassShader->SetUniforms({ "M", "V", "P", "diffuse", "specular" });

	LPassShader->SetAttributes(std::vector<std::string>{ "vVertex", "vUV"});
	LPassShader->SetUniforms({ "gDiffSpec", "gPosition", "gNormal", "gDepth", "renderMode", "viewPos", "ambientLight" });
	GPassShader->Activate();
	GPassShader->SetUniformsValue(Uniform<int>{ "diffuse", 0});
	GPassShader->SetUniformsValue(Uniform<int>{ "specular", 1});
	GPassShader->Deactivate();

	LPassShader->Activate();
	LPassShader->SetUniformsValue(Uniform<int>{ "gDiffSpec", 0});
	LPassShader->SetUniformsValue(Uniform<int>{ "gPosition", 1});
	LPassShader->SetUniformsValue(Uniform<int>{ "gNormal", 2});
	LPassShader->SetUniformsValue(Uniform<int>{ "gDepth", 3});
	LPassShader->Deactivate();

}

std::shared_ptr<VPointLight> plight1; 
//std::shared_ptr<VPointLight> plight2; // second light off until multiple lights can cast shadows
glm::vec3 originalPos1{ 2.5f, 1.5f, 0.0f };
//glm::vec3 originalPos2{ -2.5f, 1.5f, 0.0f }; for second light

void AddLightsAndCubesToScene(const std::shared_ptr<VScene>& scene, const std::shared_ptr<VMesh>& cubeMesh)
{
	VLightSource::VLightColor c{ glm::vec3{0.1f,0.1f,0.1f}, glm::vec3{ 1.0f,1.0f,1.0f }, glm::vec3{ 1.0f,1.0f,1.0f } };

	plight1 = std::make_shared<VPointLight>(glm::vec3{2.5f, 2.0f, 0.0f}, c);
	VelEng::Instance()->AddShaderProgram("ShadowShaderCube", "Resources\\Shaders\\Deffered\\ShadowsCube.vert", "Resources\\Shaders\\Deffered\\ShadowsCube.frag", "Resources\\Shaders\\Deffered\\ShadowsCube.geo");
	plight1->SetLightID(0);
	plight1->SetShader(VelEng::Instance()->GetShader("ShadowShaderCube"));
	//plight2 = std::make_shared<VPointLight>(glm::vec3{ -2.5f, 2.0f, 0.0f }, c);
	//plight2->SetLightID(1);

	scene->AddLightSource(plight1);
	//scene->AddLightSource(plight2);

	std::vector<std::shared_ptr<VModel>> cubes(25);
	for (auto& model : cubes)
	{
		model = std::make_shared<VModel>();
		VelEng::Instance()->AddModelToScene("World", model);
		model->AddMesh(cubeMesh);
	}

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			auto ite = 5 * i + j;
			cubes[ite]->ModelMatrixTranslation(glm::vec3{ (i - 2) * 3, 0, (j - 2) * 3 });
		}
	}
}


int main()
{
	VelEng::Instance()->CreateScene("World");
	VelEng::Instance()->CreateScene("Sky");

	BasicShaderSetUp();
	DefShaderSetUp();
	auto windowSize = VelEng::Instance()->GetMainWindowSize();

	auto gPShader = VelEng::Instance()->GetShader("GPass");
	auto lPShader = VelEng::Instance()->GetShader("LPass");


	VelEng::Instance()->SetRenderer(std::make_shared<VDefferedRenderer>(windowSize, gPShader, lPShader));

	
	std::shared_ptr<VModel> modelPlane = std::make_shared<VModel>();
	std::shared_ptr<VModel> skyboxModel = std::make_shared<VModel>();

	//Vel::VDirectionalLight dirLightSource(SunlightDirection, glm::vec3{ 0.3, 0.3, 0.3 }, glm::vec3{ 0.6, 0.4, 0.5 }, glm::vec3{ 1.0, 1.0, 1.0 });

	VelEng::Instance()->AddModelToScene("Sky", skyboxModel);
	VelEng::Instance()->AddModelToScene("World", modelPlane);

	

	GLfloat shin = 32.0f;
	auto diffuseTex = std::make_shared<VTexture>("Resources\\Images\\BoxDiffuse.png");
	auto specularTex = std::make_shared<VTexture>("Resources\\Images\\BoxSpecular.png");
	std::shared_ptr<VMaterial> materialTest = std::make_shared<VMaterial>(diffuseTex, specularTex, shin);

	std::shared_ptr<VMesh> cubeMeshTest = std::make_shared<VMesh>();
	std::shared_ptr<VMesh> planeMeshTest = std::make_shared<VPlaneMesh>();
	std::shared_ptr<VMesh> skyboxMesh = std::make_shared<VSkybox>(std::make_shared<VTextureCube>("Resources\\Skybox"));

	cubeMeshTest->SetShader(VelEng::Instance()->GetShader("BasicShader"));
	planeMeshTest->SetShader(VelEng::Instance()->GetShader("BasicShader"));
	skyboxMesh->SetShader(VelEng::Instance()->GetShader("SkyboxShader"));

	cubeMeshTest->SetMaterial(materialTest);
	planeMeshTest->SetMaterial(materialTest);
	cubeMeshTest->LoadVerticesOnly();

	AddLightsAndCubesToScene(VelEng::Instance()->GetScene("World"), cubeMeshTest);

	
	skyboxModel->AddMesh(skyboxMesh);
	modelPlane->AddMesh(planeMeshTest);
	modelPlane->ModelMatrixTranslation(glm::vec3{ 0, -0.51, 0 });
	modelPlane->ModelMatrixScale(glm::vec3{ 10.0, 1.0, 10.0 });
	
	//glutReshapeFunc(OnResize);

	while (VelEng::Instance()->ShouldRun())
	{
		VelEng::Instance()->GetFrameClock().Tick();
		auto posDiff = glm::sin(VelEng::Instance()->GetFrameClock().GetTime()) * 5;
		plight1->SetPosition({originalPos1.x, originalPos1.y, originalPos1.z + posDiff});
		VelEng::Instance()->HandleInput();
		VelEng::Instance()->RenderFrame();
		VelEng::Instance()->GetFrameClock().CapFPS();
	}

    return 0;
}
