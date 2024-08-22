#pragma once

#include "Renderer.h"
#include "Images.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#include "VulkanTypes.h"
#include "VulkanUtils.h"


#ifdef _DEBUG
constexpr bool useValidationLayers = true;
#else
constexpr bool useValidationLayers = false;
#endif

uint32_t Vel::MaterialInstance::instancesCount = 0;

void Vel::Renderer::Init(SDL_Window* sdlWindow, const VkExtent2D& windowExtent)
{
    window = sdlWindow;
    vkb::InstanceBuilder builder;

    vkb::Instance vkbInstance = builder.set_app_name("VelEng app")
        .request_validation_layers(useValidationLayers)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build()
        .value();

    instance = vkbInstance.instance;
    debugMessenger = vkbInstance.debug_messenger;

    SDL_Vulkan_CreateSurface(window, instance, &surface);

    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = true; //TODO use renderpasses?
    features13.synchronization2 = true;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true; //For bindless textures
    features12.runtimeDescriptorArray = true;

    vkb::PhysicalDeviceSelector selector{ vkbInstance };
    vkb::PhysicalDevice vkbPhysicalDevice = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features13)
        .set_required_features_12(features12)
        .set_surface(surface)
        .select()
        .value();

    physicalDevice = vkbPhysicalDevice.physical_device;

    vkb::DeviceBuilder deviceBuilder{ vkbPhysicalDevice };
    vkb::Device vkbDevice = deviceBuilder.build().value();
    device = vkbDevice.device;

    graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    drawExtent = windowExtent;

    mainCamera.SetPosition(glm::vec3(0, 0, 0.1));
    mainCamera.SetProjection((float)drawExtent.width / (float)drawExtent.height, 70.f);

    CreateAllocator();
    swapchain.Init(physicalDevice, device, surface, windowExtent);
    CreateCommands();
    CreateSyncStructures();

    CreateCameraDescriptors();
    InitTestLightData(); //TODO divide into creating buffers and test lights
    InitShadowPass();
    InitDeferred();

    InitTestTextures();
    InitSkyboxPass();
    InitTestData();

    InitImgui();

    isInitialized = true;
}

void Vel::Renderer::InitImgui()
{
    VkDescriptorPoolSize poolSizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

    VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = (uint32_t)std::size(poolSizes),
        .pPoolSizes = poolSizes
    };

    VkDescriptorPool imguiPool;
    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &imguiPool));

    ImGui_ImplVulkan_InitInfo initInfo{
        .Instance = instance,
        .PhysicalDevice = physicalDevice,
        .Device = device,
        .Queue = graphicsQueue,
        .DescriptorPool = imguiPool,
        .MinImageCount = 3,
        .ImageCount = 3,
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .UseDynamicRendering = true,
        .PipelineRenderingCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &swapchain.GetImageFormat()
        }
    };

    vImgui.Init(initInfo, window);
    delQueue.Push([&, imguiPool]() {
        vImgui.Cleanup();
        vkDestroyDescriptorPool(device, imguiPool, nullptr);
    });
}

void Vel::Renderer::HandleSDLEvent(SDL_Event* sdlEvent)
{
    vImgui.HandleSDLEvent(sdlEvent);
    mainCamera.ProcessSDLEvent(*sdlEvent);

    if (sdlEvent->type == SDL_KEYDOWN)
    {
        if (sdlEvent->key.keysym.sym == SDLK_F2)
        {
            imageToPresent = ++imageToPresent % 6;
        }
    }
}

void Vel::Renderer::UpdateScene()
{
    FunctionTimeMeasure measure{ stats.sceneUpdateTime };
    for (auto& materialSurfaces : mainDrawContext.opaqueSurfaces)
    {
        materialSurfaces.clear();
    }
    for (auto& materialSurfaces : mainDrawContext.transparentSurfaces)
    {
        materialSurfaces.clear();
    }
    
    UpdateActors(); //TODO
    UpdateGlobalLighting();

    //auto modelMatrix = glm::scale(glm::vec3{ 60.0f, 60.0f, 60.0f });
    auto modelMatrix = glm::scale(glm::vec3{ 1, 1, 1 });
    auto light1Matrix = glm::translate(glm::vec3{ light1Pos.x, light1Pos.y, light1Pos.z });
    auto light2Matrix = glm::translate(glm::vec3{ light2Pos.x, light2Pos.y, light2Pos.z });

    loadedScenes["model"]->Draw(modelMatrix, mainDrawContext);
    loadedScenes["lightSource"]->Draw(light1Matrix, mainDrawContext);
    loadedScenes["lightSource"]->Draw(light2Matrix, mainDrawContext);

    UpdateCamera();
}

void Vel::Renderer::UpdateActors()
{
}

void Vel::Renderer::UpdateGlobalLighting()
{
    float movement1 = sin(frameNumber / 30.f) * 10.0f;
    float movement2 = cos(frameNumber / 30.f) * 10.0f;

    testLights.sunlight.UpdateCameraPosition(mainCamera);
    LightData* lightsGPUData = (LightData*)testLights.lightsDataBuffer.info.pMappedData;
    lightsGPUData->sunlightViewProj = testLights.sunlight.viewProj;

    light1Pos = { movement1 + 5.0f, 2.0f, -5.0f + movement2, 0.0f };
    light2Pos = { 15.0f, 20.0f, -20.0f + movement1, 0.0f };

    testLights.pointLightsGPUData[0] = {
        .position = light1Pos,
        .color = {0.2f, 0.2f, 1.0f, 150.0f},
    };
    testLights.pointLightsGPUData[1] = {
        .position = light2Pos,
        .color = {1.0f, 0.2f, 0.2f, 100.0f},
    };
}

void Vel::Renderer::UpdateCamera()
{
    mainCamera.Update();

    sceneData.view = mainCamera.GetViewMatrix();
    sceneData.projection = mainCamera.GetProjectionMatrix();
    glm::mat4 viewProj = mainCamera.GetViewProjectionMatrix();
    sceneData.viewProjection = viewProj;
    sceneData.invViewProjection = glm::inverse(viewProj);
    sceneData.position = glm::vec4(mainCamera.GetPosition(), 1.0f);
    sceneData.testData.g = testRoughness;
    sceneData.testData.b = testMetallic * 0.1f;
}

void Vel::Renderer::UpdateGlobalDescriptors()
{
    AllocatedBuffer sceneCameraDataBuffer = gpuAllocator.CreateBuffer(sizeof(SceneCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    GetCurrentFrame().frameCleanupQueue.Push([=, this]() {
        gpuAllocator.DestroyBuffer(sceneCameraDataBuffer);
    });

    SceneCameraData* sceneCameraGPUData = (SceneCameraData*)sceneCameraDataBuffer.info.pMappedData;
    *sceneCameraGPUData = sceneData;

    sceneCameraDataDescriptorSet = GetCurrentFrame().frameDescriptors.Allocate(sceneCameraDataDescriptorLayout);
    DescriptorWriter writer;
    writer.WriteBuffer(0, sceneCameraDataBuffer.buffer, sizeof(SceneCameraData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    writer.UpdateSet(device, sceneCameraDataDescriptorSet);

    deferred.UpdateCameraDescriptorSet(sceneCameraDataDescriptorSet);
}

void Vel::Renderer::Draw()
{
    FunctionTimeMeasure measure{ stats.frametime };

    if (swapchain.IsAwaitingResize())
    {
        vkDeviceWaitIdle(device);
        swapchain.Resize(window);
    }

    UpdateScene();

    PrepareImguiFrame();

    //TODO fix draw extent in deferred after resize
    const VkExtent2D& swapchainSize = swapchain.GetImageSize();
    drawExtent.width = static_cast<uint32_t>(std::min(swapchainSize.width, drawExtent.width) * renderScale);
    drawExtent.height = static_cast<uint32_t>(std::min(swapchainSize.height, drawExtent.height) * renderScale);

    auto& currentFrame = GetCurrentFrame();
    VK_CHECK(vkWaitForFences(device, 1, &currentFrame.renderFence, true, FRAME_TIMEOUT));
    currentFrame.frameCleanupQueue.Flush();
    currentFrame.frameDescriptors.ClearPools();

    VK_CHECK(vkResetFences(device, 1, &currentFrame.renderFence));

    swapchain.AcquireNextImageIndex(currentFrame.swapchainSemaphore);
    if (swapchain.IsAwaitingResize())
    {
        return;
    }

    UpdateGlobalDescriptors();

    VkCommandBufferBeginInfo cmdBufferBeginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    DrawShadows();

    //--SKYBOX-- --SHADOW PASS--
    auto& skyboxCmd = currentFrame.skyboxCommands;
    VK_CHECK(vkResetCommandBuffer(skyboxCmd, 0));
    VK_CHECK(vkBeginCommandBuffer(skyboxCmd, &cmdBufferBeginInfo));

    deferred.GetFramebuffer().TransitionImages(skyboxCmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    {
        skyboxPass.Draw(skyboxCmd, mainCamera);
        shadowPass.Draw(mainDrawContext, skyboxCmd);
    }

    VK_CHECK(vkEndCommandBuffer(skyboxCmd));

    VkCommandBufferSubmitInfo skyboxBufferInfo = CreateCommandBufferSubmitInfo(skyboxCmd);
    VkSemaphoreSubmitInfo skyboxWaitInfo = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, currentFrame.swapchainSemaphore);
    VkSemaphoreSubmitInfo signalSemaphores[2];
    signalSemaphores[0] = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, skyboxPass.GetSemaphore());
    signalSemaphores[1] = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, shadowPass.GetSemaphore());
    VkSubmitInfo2 skyboxSubmitInfo = CreateSubmitInfo(skyboxBufferInfo, &skyboxWaitInfo, 1, signalSemaphores, 2);

    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &skyboxSubmitInfo, VK_NULL_HANDLE));


    //--SHADOW PASS--
    /*auto& shadowCmd = currentFrame.shadowCommands;
    VK_CHECK(vkResetCommandBuffer(shadowCmd, 0));
    VK_CHECK(vkBeginCommandBuffer(shadowCmd, &cmdBufferBeginInfo));

    {
        shadowPass.Draw(mainDrawContext, shadowCmd);
    }

    VK_CHECK(vkEndCommandBuffer(shadowCmd));

    VkCommandBufferSubmitInfo shadowPassBufferInfo = CreateCommandBufferSubmitInfo(shadowCmd);
    VkSemaphoreSubmitInfo shadowPassWaitInfo = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, currentFrame.swapchainSemaphore);
    VkSemaphoreSubmitInfo shadowPassSignalInfo = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, shadowPass.GetSemaphore());
    VkSubmitInfo2 shadowPassSubmitInfo = CreateSubmitInfo(shadowPassBufferInfo, &shadowPassWaitInfo, 1, &shadowPassSignalInfo);

    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &shadowPassSubmitInfo, VK_NULL_HANDLE));*/


    //--GPASS--
    auto& gPassCmd = currentFrame.gPassCommands;
    VK_CHECK(vkResetCommandBuffer(gPassCmd, 0));
    VK_CHECK(vkBeginCommandBuffer(gPassCmd, &cmdBufferBeginInfo));

    {
        FunctionTimeMeasure measure{ stats.gPassDrawTime };
        stats.gPassesAccTime += stats.gPassDrawTime;
        deferred.DrawGPass(mainDrawContext, gPassCmd);
        ++stats.gPassesCount;
    }
    stats.gPassesAverage = stats.gPassesAccTime / stats.gPassesCount;

    VK_CHECK(vkEndCommandBuffer(gPassCmd));

    VkCommandBufferSubmitInfo gPassBufferInfo = CreateCommandBufferSubmitInfo(gPassCmd);
    VkSemaphoreSubmitInfo gPassWaitInfo = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, skyboxPass.GetSemaphore());
    VkSemaphoreSubmitInfo gPassSignalInfo = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, deferred.GetGPassSemaphore());
    VkSubmitInfo2 gPassSubmitInfo = CreateSubmitInfo(gPassBufferInfo, &gPassWaitInfo, 1, &gPassSignalInfo, 1);

    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &gPassSubmitInfo, VK_NULL_HANDLE));


    //--LPASS--
    auto& lPassCmd = currentFrame.lPassCommands;
    VK_CHECK(vkResetCommandBuffer(lPassCmd, 0));
    VK_CHECK(vkBeginCommandBuffer(lPassCmd, &cmdBufferBeginInfo));

    TransitionDepthImage(lPassCmd, testLights.sunlight.shadowMap.image, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);
    deferred.GetFramebuffer().TransitionImages(lPassCmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    {
        FunctionTimeMeasure measure{ stats.lPassDrawTime };
        deferred.DrawLPass(mainDrawContext, lPassCmd);
    }

    PreparePresentableImage(lPassCmd);

    VK_CHECK(vkEndCommandBuffer(lPassCmd));

    //TODO semaphores are shared between frames
    VkCommandBufferSubmitInfo lPassBufferSubmitInfo = CreateCommandBufferSubmitInfo(lPassCmd);
    VkSemaphoreSubmitInfo waitSemaphores[2];
    waitSemaphores[0] = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, shadowPass.GetSemaphore());
    waitSemaphores[1] = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, deferred.GetGPassSemaphore());
    VkSemaphoreSubmitInfo lPassSignalInfo = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, deferred.GetLPassSemaphore());
    VkSubmitInfo2 lPassSubmitInfo = CreateSubmitInfo(lPassBufferSubmitInfo, waitSemaphores, 2, &lPassSignalInfo, 1);

    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &lPassSubmitInfo, currentFrame.renderFence));


    //--PRESENT--
    swapchain.PresentImage(&deferred.GetLPassSemaphore(), graphicsQueue);
    if (swapchain.IsAwaitingResize())
    {
        return;
    }

    frameNumber++;
}

void Vel::Renderer::PreparePresentableImage(VkCommandBuffer cmd)
{
    VkImage presentableImage = VK_NULL_HANDLE;
    VkImageLayout transitionSrcLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    AllocatedBuffer& buff = gpuAllocator.GetStagingBuffer();

    TransitionImage(cmd, swapchain.GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    switch (imageToPresent)
    {
    case 1:
        presentableImage = deferred.GetFramebuffer().position.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case 2:
        presentableImage = deferred.GetFramebuffer().color.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case 3:
        presentableImage = deferred.GetFramebuffer().normals.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case 4:
        presentableImage = deferred.GetFramebuffer().metallicRoughness.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case 5:
        presentableImage = testLights.sunlight.shadowMap.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        TransitionDepthImage(cmd, presentableImage, transitionSrcLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        CopyDepthToColorImage(cmd, presentableImage, buff, swapchain.GetImage(), testLights.sunlight.shadowMap.imageExtent);
        break;
    default:
        presentableImage = deferred.GetDrawImage().image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        break;
    }

    if (imageToPresent != 5)
    {
        TransitionImage(cmd, presentableImage, transitionSrcLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        BlitImage(cmd, presentableImage, swapchain.GetImage(), drawExtent, swapchain.GetImageSize());
    }

    DrawImgui(cmd, swapchain.GetImage(), swapchain.GetImageView(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

void Vel::Renderer::PrepareImguiFrame()
{
    vImgui.PrepareFrame([&]() {
        ImGui::Text("Background");
        ImGui::SliderFloat("Render Scale", &renderScale, 0.3f, 1.f);
        ImGui::Text("Camera");
        ImGui::Text("position  %f  %f  %f", mainCamera.GetPosition().x, mainCamera.GetPosition().y, mainCamera.GetPosition().z);
        ImGui::SliderFloat("Speed", &mainCamera.speed, 0.001f, 1.f);
        ImGui::SliderFloat("roughness", &testRoughness, 0.01f, 1.f);
        ImGui::SliderFloat("metallic", &testMetallic, 0.0001f, 0.1f);
        ImGui::Text("Perf");
        ImGui::Text("frametime %f ms", stats.frametime);
        ImGui::Text("drawtime %f ms", stats.dedicatedMaterialDrawTime);
        ImGui::Text("gpass drawtime %f ms", stats.gPassDrawTime);
        ImGui::Text("gpass drawtime %f ms", stats.gPassesAverage);
        ImGui::Text("lpass drawtime %f ms", stats.lPassDrawTime);
        ImGui::Text("scene update %f ms", stats.sceneUpdateTime);
    });
}

void Vel::Renderer::DrawShadows()
{
}

void Vel::Renderer::DrawImgui(VkCommandBuffer cmdBuffer, VkImage drawImage, VkImageView drawImageView, VkImageLayout srcLayout, VkImageLayout dstLayout)
{
    TransitionImage(cmdBuffer, drawImage, srcLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vImgui.Draw(cmdBuffer, drawImageView, swapchain.GetImageSize());
    TransitionImage(cmdBuffer, drawImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, dstLayout);
}

void Vel::Renderer::InitTestData()
{
    meshLoader.Init(device, this);
    //auto mainModel = meshLoader.loadGltf(GET_MESH_PATH("corset/Corset.gltf"));
    //auto mainModel = meshLoader.loadGltf(GET_MESH_PATH("house/house.glb"));
    auto mainModel = meshLoader.loadGltf(GET_MESH_PATH("sponza/Sponza.gltf"));
    auto cube = meshLoader.loadGltf(GET_MESH_PATH("test/cube.glb"));

    assert(mainModel.has_value());

    loadedScenes["model"] = *mainModel;
    loadedScenes["lightSource"] = *cube;

    //TODO size per surface type material
    mainDrawContext.opaqueSurfaces.resize(MaterialInstance::instancesCount);
    mainDrawContext.transparentSurfaces.resize(MaterialInstance::instancesCount);
}

void Vel::Renderer::InitTestLightData()
{
    testLights.lights.ambient = glm::vec4{ 0.05f, 0.05f, 0.05f, 1.0f };

    VkExtent3D shadowMapResolution = {
        .width = 2048,
        .height = 2048,
        .depth = 1,
    };
    testLights.sunlight.InitShadowData(gpuAllocator, shadowMapResolution);
    testLights.sunlight.InitLightData(glm::normalize(glm::vec4{ 0.5f, 1.f, 0.5f, 1.0f }), glm::vec4{ 1.0f, 0.85f, 0.3f, 1.0f });
    //testLights.sunlight.direction = glm::normalize(glm::vec4{ 0.5f, -0.5f, 0.5f, 1.0f });
    //testLights.sunlight.color = glm::vec4{ 1.0f, 0.85f, 0.2f, 1.0f };

    testLights.lights.pointLightsCount = 2;
    testLights.pointLightsBuffer = gpuAllocator.CreateBuffer(sizeof(PointLight) * testLights.lights.pointLightsCount, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    testLights.pointLightsGPUData = (PointLight*)testLights.pointLightsBuffer.info.pMappedData;

    testLights.pointLightsGPUData[0] = {
        .position = {0.0f, 0.0f, 0.0f, 0.0f},
        .color = {0.0f, 0.0f, 1.0f, 10.0f},
    };

    testLights.pointLightsGPUData[1] = {
        .position = {0.0f, 0.0f, 0.0f, 0.0f},
        .color = {0.0f, 0.5f, 1.0f, 1.0f},
    };

    VkBufferDeviceAddressInfo deviceAdressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = testLights.pointLightsBuffer.buffer
    };

    testLights.lightsDataBuffer = gpuAllocator.CreateBuffer(sizeof(LightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    LightData* lightsGPUData = (LightData*)testLights.lightsDataBuffer.info.pMappedData;

    *lightsGPUData = {
        .ambient = glm::vec4{ 0.05f, 0.05f, 0.1f, 1.0f},
        .sunlightDirection = glm::vec4(testLights.sunlight.direction, 1.0f),
        .sunlightColor = testLights.sunlight.color,
        .sunlightViewProj = testLights.sunlight.viewProj,
        .sunlightShadowMapID = 0,
        .pointLightsCount = testLights.lights.pointLightsCount,
        .pointLightBuffer = vkGetBufferDeviceAddress(device, &deviceAdressInfo)
    };

    delQueue.Push([&]() {
        gpuAllocator.DestroyImage(testLights.sunlight.shadowMap);
        gpuAllocator.DestroyBuffer(testLights.sunlight.gpuViewProjData);
        gpuAllocator.DestroyBuffer(testLights.lightsDataBuffer);
        gpuAllocator.DestroyBuffer(testLights.pointLightsBuffer);
    });
}

Vel::GPUMeshBuffers Vel::Renderer::CreateRectangle()
{
    std::array<Vertex, 4> rectVertices;

    rectVertices[0].position = { 1.0f,-1.0f, 0.0f };
    rectVertices[0].uv_x = 1;
    rectVertices[0].uv_y = 0;
    rectVertices[1].position = { 1.0f, 1.0f, 0.0f };
    rectVertices[1].uv_x = 1;
    rectVertices[1].uv_y = 1;
    rectVertices[2].position = { -1.0f, -1.0f, 0.0f };
    rectVertices[2].uv_x = 0;
    rectVertices[2].uv_y = 0;
    rectVertices[3].position = { -1.0f, 1.0f, 0.0f };
    rectVertices[3].uv_x = 0;
    rectVertices[3].uv_y = 1;

    rectVertices[0].tangent = { 1.0f, 0.0f, 0.0f, 0.0f };
    rectVertices[1].tangent = { 1.0f, 0.0f, 0.0f, 0.0f };
    rectVertices[2].tangent = { 1.0f, 0.0f, 0.0f, 0.0f };
    rectVertices[3].tangent = { 1.0f, 0.0f, 0.0f, 0.0f };

    std::array<uint32_t, 6> rectIndices;

    rectIndices[0] = 0;
    rectIndices[1] = 1;
    rectIndices[2] = 2;

    rectIndices[3] = 2;
    rectIndices[4] = 1;
    rectIndices[5] = 3;

    return gpuAllocator.UploadMesh(rectIndices, rectVertices);
}

void Vel::Renderer::InitSkyboxPass()
{
    skyboxPass.Init(device, drawExtent, defaultCubeImage, deferred.GetFramebuffer().color);
}

void Vel::Renderer::InitShadowPass()
{
    shadowPass.Init(device, testLights.sunlight);
}

void Vel::Renderer::InitDeferred()
{
    deferred.Init(device, &gpuAllocator, drawExtent, sceneCameraDataDescriptorLayout, testLights.lightsDataBuffer.buffer, sizeof(LightData), testLights.sunlight.shadowMap.imageView, CreateRectangle());
}

void Vel::Renderer::InitTestTextures()
{
    uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
    whiteImage = gpuAllocator.CreateImage((void*)&white, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    uint32_t normals = glm::packUnorm4x8(glm::vec4(0.5f, 0.5f, 1.0f, 0.0f));
    defaultNormalsImage = gpuAllocator.CreateImage((void*)&normals, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    RenderableGLTF::defaultNormalsImage = defaultNormalsImage.image; //TODO temp until asset manager

    uint32_t metallicRoughness = glm::packUnorm4x8(glm::vec4(0.0f, 1.0f, 1.0f, 0.0f));
    defaultMetallicRoughnessImage = gpuAllocator.CreateImage((void*)&metallicRoughness, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    RenderableGLTF::defaultMetallicRoughnessImage = defaultMetallicRoughnessImage.image; //TODO temp until asset manager

    uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
    uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
    std::array<uint32_t, 16 * 16 > pixels;
    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
        }
    }
    errorCheckerboardImage = gpuAllocator.CreateImage((void*)pixels.data(), VkExtent3D{16, 16, 1}, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    RenderableGLTF::errorCheckerboardImage = errorCheckerboardImage.image; //TODO temp until asset manager

    STBImage front(GET_TEXTURE_PATH("skybox/front.png"));
    STBImage back(GET_TEXTURE_PATH("skybox/back.png"));
    STBImage left(GET_TEXTURE_PATH("skybox/left.png"));
    STBImage right(GET_TEXTURE_PATH("skybox/right.png"));
    STBImage top(GET_TEXTURE_PATH("skybox/top.png"));
    STBImage bottom(GET_TEXTURE_PATH("skybox/bottom.png"));

    std::array<unsigned char*, GPUAllocator::cubeTextureLayers> cubeImages;
    cubeImages[0] = right.GetImageData();
    cubeImages[1] = left.GetImageData();
    cubeImages[2] = top.GetImageData();
    cubeImages[3] = bottom.GetImageData();
    cubeImages[4] = front.GetImageData();
    cubeImages[5] = back.GetImageData();

    defaultCubeImage = gpuAllocator.CreateCubeImage(cubeImages, front.GetSize(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    VkSamplerCreateInfo samplerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR
    };

    vkCreateSampler(device, &samplerCreateInfo, nullptr, &defaultSamplerLinear);

    delQueue.Push([&]() {
        vkDestroySampler(device, defaultSamplerLinear, nullptr);

        gpuAllocator.DestroyImage(whiteImage);
        gpuAllocator.DestroyImage(defaultNormalsImage);
        gpuAllocator.DestroyImage(defaultMetallicRoughnessImage);
        gpuAllocator.DestroyImage(defaultCubeImage);
        gpuAllocator.DestroyImage(errorCheckerboardImage);
    });
}

void Vel::Renderer::CreateCommands()
{
    VkCommandPoolCreateInfo commandPoolInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphicsQueueFamily
    };

    VkCommandBufferAllocateInfo cmdAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    for (auto& frameInfo : frames)
    {
        VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frameInfo.commandPool));

        cmdAllocateInfo.commandPool = frameInfo.commandPool;

        VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocateInfo, &frameInfo.shadowCommands));
        VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocateInfo, &frameInfo.skyboxCommands));
        VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocateInfo, &frameInfo.gPassCommands));
        VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocateInfo, &frameInfo.lPassCommands));

        frameInfo.cleanupQueue.Push( [&]() { vkDestroyCommandPool(device, frameInfo.commandPool, nullptr); } );
    }
}

void Vel::Renderer::CreateSyncStructures()
{
    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkSemaphoreCreateInfo semaphoreInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr
    };

    for (auto& frame : frames)
    {
        VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &frame.renderFence));
        frame.cleanupQueue.Push([&]() { vkDestroyFence(device, frame.renderFence, nullptr); });

        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frame.swapchainSemaphore));
        frame.cleanupQueue.Push([&]()
        {
            vkDestroySemaphore(device, frame.swapchainSemaphore, nullptr);
        });
    }
}

void Vel::Renderer::CreateAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = physicalDevice,
        .device = device,
        .instance = instance
    };

    gpuAllocator.Init(device, allocatorInfo, graphicsQueueFamily, graphicsQueue);
    delQueue.Push([&]() {
        gpuAllocator.Cleanup();
    });
}

void Vel::Renderer::CreateCameraDescriptors()
{
    DescriptorLayoutBuilder builder;
    builder.Clear();
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    sceneCameraDataDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    for (auto& frameInfo : frames)
    {
        std::vector<DescriptorPoolSizeRatio> frameSizes = {
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4}
        };

        frameInfo.frameDescriptors = DescriptorAllocatorDynamic{};
        frameInfo.frameDescriptors.InitPool(device, 100, frameSizes);

        delQueue.Push([&]() {
            frameInfo.frameDescriptors.Cleanup();
        });
    }

    delQueue.Push([&]() {
        vkDestroyDescriptorSetLayout(device, sceneCameraDataDescriptorLayout, nullptr);
    });
}

void Vel::Renderer::Cleanup()
{
    if (isInitialized)
    {
        vkDeviceWaitIdle(device);

        loadedScenes.clear();

        shadowPass.Cleanup();
        skyboxPass.Cleanup();
        deferred.Cleanup();

        for (auto& frame : frames)
        {
            frame.cleanupQueue.Flush();
            frame.frameCleanupQueue.Flush();
        }

        delQueue.Flush();

        swapchain.DestroySwapchain();

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyDevice(device, nullptr);
        vkb::destroy_debug_utils_messenger(instance, debugMessenger);
        vkDestroyInstance(instance, nullptr);
    }
}
