// VelTest.cpp : Defines the entry point for the console application.
//

#include "../VelEng/VelGL.h"
#include "../VelEng/VUti/VelEngUti.h"
#include "../VelEng/VLights/VLight.h"
#include "../VelEng/VMaterials/VMaterial.h"
#include <functional>
#include "../VelUti/VTime.h"
//#include "json.hpp"
//S£ONECZNIKI
using namespace Vel;

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
	LPassShader->SetUniforms({ "gDiffSpec", "gPosition", "gNormal", "gDepth", "viewPos", "ambientLight", "lightSpaceMatrix" });
	LPassShader->SetUniforms({ "pointShadowMap[0]",  "pointShadowMap[1]",  "pointShadowMap[2]",  "pointShadowMap[3]", "dirLightShadowMap" });
	GPassShader->Activate();
	GPassShader->SetUniformsValue(Uniform<int>{ "diffuse", 0});
	GPassShader->SetUniformsValue(Uniform<int>{ "specular", 1});
	GPassShader->Deactivate();

	LPassShader->Activate();
	LPassShader->SetUniformsValue(Uniform<int>{ "gDiffSpec", 0});
	LPassShader->SetUniformsValue(Uniform<int>{ "gPosition", 1});
	LPassShader->SetUniformsValue(Uniform<int>{ "gNormal", 2});
	LPassShader->SetUniformsValue(Uniform<int>{ "gDepth", 3});
	LPassShader->SetUniformsValue(Uniform<int>{ "pointShadowMap[0]", 4});
	LPassShader->SetUniformsValue(Uniform<int>{ "pointShadowMap[1]", 5});
	LPassShader->SetUniformsValue(Uniform<int>{ "pointShadowMap[2]", 6});
	LPassShader->SetUniformsValue(Uniform<int>{ "pointShadowMap[3]", 7});
	LPassShader->SetUniformsValue(Uniform<int>{ "dirLightShadowMap", 8});
	LPassShader->Deactivate();

}

std::shared_ptr<PointLight> plight1; 
std::shared_ptr<PointLight> plight2; // second light off until multiple lights can cast shadows
//std::shared_ptr<PointLight> plight3;
//std::shared_ptr<PointLight> plight4;
//std::shared_ptr<PointLight> plight5; // no shadow for this one
glm::vec3 originalLight1Pos{ 7.0f, 4.25f, 0.0f };
glm::vec3 originalLight2Pos{ 0.0f,4.25f, 0.0f };

void AddLightsAndCubesToScene(const std::shared_ptr<Scene>& scene, const std::shared_ptr<Mesh>& cubeMesh)
{
	LightSource::LightColor pointLight1Color{ glm::vec3{ 2.0f,2.0f,2.0f }, glm::vec3{ 2.0f,2.0f,2.0f } };
	LightSource::LightColor pointLight2Color{ glm::vec3{ 1.5f,0.0f,1.5f }, glm::vec3{ 1.5f,0.0f,1.5f } };
	LightSource::LightColor directionalLightColor{ glm::vec3{ 0.2f, 0.25f, 0.45f}, glm::vec3{ 0.2f, 0.25f, 0.45f } };
	glm::vec3 SunlightDirection{ 0.5, -0.5, 1.0 };
	


	plight1 = std::make_shared<PointLight>(originalLight1Pos, pointLight1Color);
	plight2 = std::make_shared<PointLight>(originalLight2Pos, pointLight2Color);
	//plight3 = std::make_shared<PointLight>(originalLight1Pos, pointLight2Color);
	//plight4 = std::make_shared<PointLight>(originalLight2Pos, pointLight2Color); 
	VelEng::Instance()->AddShaderProgram("ShadowMapping", "Resources\\Shaders\\Deffered\\ShadowMapping.vert", "Resources\\Shaders\\Deffered\\ShadowMapping.frag");
	VelEng::Instance()->AddShaderProgram("ShadowMappingCube", "Resources\\Shaders\\Deffered\\ShadowsCube.vert", "Resources\\Shaders\\Deffered\\ShadowsCube.frag", "Resources\\Shaders\\Deffered\\ShadowsCube.geo");
	auto shadowShader = VelEng::Instance()->GetShader("ShadowMapping");
	shadowShader->SetAttributes({ "vVertex", "vNormal", "vUV" });
	shadowShader->Activate();
	shadowShader->SetUniforms(std::vector<std::string>{ "lightSpaceMatrix", "M" });
	shadowShader->Deactivate();

	auto shadowCubeShader = VelEng::Instance()->GetShader("ShadowMappingCube");
	shadowCubeShader->SetAttributes({ "vVertex", "vNormal", "vUV" });
	shadowCubeShader->Activate();
	shadowCubeShader->SetUniforms({"M", "lightPos", "farPlane" });
	shadowCubeShader->Deactivate();

	plight1->SetShader(shadowCubeShader);
	plight2->SetShader(shadowCubeShader);
	//plight3->SetShader(shadowCubeShader);
	//plight4->SetShader(shadowCubeShader);

	std::unique_ptr<DirectionalLight> dLight = std::make_unique<DirectionalLight>(SunlightDirection, directionalLightColor);
	dLight->SetShader(shadowShader);
	scene->CreateDirectionalLight(std::move(dLight));

	scene->AddLightSource(plight1);
	scene->AddLightSource(plight2);
	//scene->AddLightSource(plight3);
	//scene->AddLightSource(plight4);

	std::vector<std::shared_ptr<Model>> cubes(25);
	for (auto& model : cubes)
	{
		model = std::make_shared<Model>();
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


	VelEng::Instance()->SetRenderer(std::make_shared<DefferedRenderer>(windowSize, gPShader, lPShader));

	
	std::shared_ptr<Model> modelPlane = std::make_shared<Model>();
	std::shared_ptr<Model> skyboxModel = std::make_shared<Model>();

	//Vel::DirectionalLight dirLightSource(SunlightDirection, glm::vec3{ 0.3, 0.3, 0.3 }, glm::vec3{ 0.6, 0.4, 0.5 }, glm::vec3{ 1.0, 1.0, 1.0 });

	VelEng::Instance()->AddModelToScene("Sky", skyboxModel);
	VelEng::Instance()->AddModelToScene("World", modelPlane);

	

	GLfloat shin = 32.0f;
	auto diffuseTex = std::make_shared<Texture>("Resources\\Images\\BoxDiffuse.png");
	auto specularTex = std::make_shared<Texture>("Resources\\Images\\BoxSpecular.png");
	std::shared_ptr<Material> materialTest = std::make_shared<Material>(diffuseTex, specularTex, shin);

	std::shared_ptr<Mesh> cubeMeshTest = std::make_shared<Mesh>();
	std::shared_ptr<Mesh> planeMeshTest = std::make_shared<PlaneMesh>();
	std::shared_ptr<Mesh> skyboxMesh = std::make_shared<Skybox>(std::make_shared<TextureCube>("Resources\\Skybox"));

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
	

	while (VelEng::Instance()->ShouldRun())
	{
		VelEng::Instance()->GetFrameClock().Tick();
		auto posDiff1 = glm::sin(VelEng::Instance()->GetFrameClock().GetTime()) * 2;
		auto xposDiff2 = glm::sin(VelEng::Instance()->GetFrameClock().GetTime()*2) * 3;
		auto zposDiff2 = glm::cos(VelEng::Instance()->GetFrameClock().GetTime() * 2) * 3;
		plight1->SetPosition({ originalLight1Pos.x, originalLight1Pos.y, originalLight1Pos.z + posDiff1 });
		plight2->SetPosition({ originalLight2Pos.x + xposDiff2, originalLight2Pos.y, originalLight2Pos.z + zposDiff2 });
		//plight3->SetPosition({ originalPos1.x, originalPos1.y, originalPos1.z - posDiff1 });
		//plight4->SetPosition({ originalLight2Pos.x - posDiff2, originalLight2Pos.y, originalPos2.z });
		VelEng::Instance()->HandleInput();
		VelEng::Instance()->RenderFrame();
		VelEng::Instance()->GetFrameClock().CapFPS();
	}

    return 0;
}
