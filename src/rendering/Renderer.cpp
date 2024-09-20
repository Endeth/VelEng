#pragma once

#include "Rendering/Renderer.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#include "Rendering/VulkanTypes.h"
#include "Rendering/VulkanUtils.h"

#include "Rendering/Assets/Images.h"


#ifdef _DEBUG
constexpr bool useValidationLayers = true;
#else
constexpr bool useValidationLayers = false;
#endif

uint32_t Vel::MaterialInstance::instancesCount = 0;

Vel::Renderer::Renderer() : //Temp?
    renderThreadPool(RENDER_THREADS_COUNT)
{
}

Vel::Renderer::~Renderer()
{
}

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

    drawExtent = VkExtent3D {
        .width = windowExtent.width,
        .height = windowExtent.height,
        .depth = 1
    };

    mainCamera.SetPosition(glm::vec3(0, 0, 0));
    mainCamera.SetProjection((float)drawExtent.width / (float)drawExtent.height, 70.f);

    CreateAllocator();
    swapchain.Init(physicalDevice, device, surface, windowExtent);
    CreateCameraDescriptors(); //TODO move to frame data
    CreateCommandsInfo();

    InitTestLightData(); //TODO divide into creating buffers and test lights
    InitShadowPass();
    InitDeferred();
    CreateFrameData();

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

    UpdateCamera();

    UpdateGlobalLighting();

    UpdateActors();
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
}

void Vel::Renderer::UpdateActors()
{
    modelMatrix = glm::scale(glm::vec3{ 1, 1, 1 });
    modelMatrix2 = glm::translate(glm::vec3{ 0, -50, 0 });
    light1Matrix = glm::translate(glm::vec3{ light1Pos.x, light1Pos.y, light1Pos.z });
    light2Matrix = glm::translate(glm::vec3{ light2Pos.x, light2Pos.y, light2Pos.z });
}

void Vel::Renderer::UpdateFrameDescriptors()
{
    auto& currentFrame = GetCurrentFrame();

    SceneCameraData* sceneCameraGPUData = (SceneCameraData*)currentFrame.GetSceneData().cameraDataBuffer.info.pMappedData;
    *sceneCameraGPUData = sceneData;

    deferredPasses.SetCameraDescriptorSet(currentFrame.GetSceneData().cameraDescriptorSet);
}

void Vel::Renderer::AwaitFramePreviousRenderDone(FrameData& frame)
{
    VK_CHECK(vkWaitForFences(device, 1, &frame.GetSync().renderFence, true, FRAME_TIMEOUT));
    VK_CHECK(vkResetFences(device, 1, &frame.GetSync().renderFence));
}

void Vel::Renderer::SkyboxDraw(FrameData& frame)
{
    VkCommandPool threadCmdPool = frame.GetAvailableCommandPool();
    VkCommandBuffer skyboxCmd = CreateCommandBuffer(threadCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VK_CHECK(vkResetCommandBuffer(skyboxCmd, 0));
    VK_CHECK(vkBeginCommandBuffer(skyboxCmd, &primaryCommandBegin));

    frame.resources.gPassFramebuffer.TransitionImagesForAttachment(skyboxCmd);
    {
        skyboxPass.Draw(skyboxCmd, mainCamera, frame.resources.gPassFramebuffer.color);
    }

    VK_CHECK(vkEndCommandBuffer(skyboxCmd));

    QueueGPUWork(skyboxCmd, { frame.GetSync().swapchainSemaphore }, frame.GetSync().skyboxSemaphore);

    frame.ReaddCommandPool(threadCmdPool);
}

//TODO Some model grouping, Batch created after updating some actors?
void Vel::Renderer::GPassContextWork(std::shared_ptr<RenderableGLTF> model, const glm::mat4& modelMatrix, std::vector<DrawContext>& drawContexts)
{
    DrawContext workDrawContext(MaterialInstance::instancesCount);;

    model->Draw(modelMatrix, workDrawContext);

    std::lock_guard lock(drawContextMutex);
    drawContexts.emplace_back(std::move(workDrawContext));
}

void Vel::Renderer::GPassCommandRecord(FrameData& frame)
{
    VkCommandPool threadCmdPool = frame.GetAvailableCommandPool();
    VkCommandBuffer gPassCmd = CreateCommandBuffer(threadCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VK_CHECK(vkResetCommandBuffer(gPassCmd, 0));
    VK_CHECK(vkBeginCommandBuffer(gPassCmd, &primaryCommandBegin));

    {
        FunctionTimeMeasure measure{ stats.gPassDrawTime };
        deferredPasses.DrawGPass(frame.resources.gPassDrawContexts, gPassCmd, frame.resources.gPassFramebuffer);
    }
    ++stats.gPassesCount;
    stats.gPassesAccTime += stats.gPassDrawTime;
    stats.gPassesAverage = stats.gPassesAccTime / stats.gPassesCount;

    VK_CHECK(vkEndCommandBuffer(gPassCmd));

    QueueGPUWork(gPassCmd, { frame.GetSync().skyboxSemaphore }, frame.GetSync().gPassSemaphore);

    frame.ReaddCommandPool(threadCmdPool);
}

void Vel::Renderer::LightSourceShadowWork(FrameData& frame)
{
    DrawContext shadowContext(MaterialInstance::instancesCount);

    loadedScenes["model"]->Draw(modelMatrix, shadowContext);
    loadedScenes["lightSource"]->Draw(light1Matrix, shadowContext);
    loadedScenes["lightSource"]->Draw(light2Matrix, shadowContext);

    VkCommandPool threadCmdPool = frame.GetAvailableCommandPool();
    VkCommandBuffer shadowCmd = CreateCommandBuffer(threadCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VK_CHECK(vkResetCommandBuffer(shadowCmd, 0));
    VK_CHECK(vkBeginCommandBuffer(shadowCmd, &primaryCommandBegin));

    TransitionDepthImage(shadowCmd, testLights.sunlight.shadowMap.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    shadowPass.Draw(shadowContext, shadowCmd, testLights.sunlight);

    VK_CHECK(vkEndCommandBuffer(shadowCmd));

    QueueGPUWork(shadowCmd, {}, frame.GetSync().shadowsSemaphore);

    frame.ReaddCommandPool(threadCmdPool);
}

void Vel::Renderer::LPassCommandRecord(FrameData& frame)
{
    VkCommandPool threadCmdPool = frame.GetAvailableCommandPool();
    VkCommandBuffer lPassCmd = CreateCommandBuffer(threadCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VK_CHECK(vkResetCommandBuffer(lPassCmd, 0));
    VK_CHECK(vkBeginCommandBuffer(lPassCmd, &primaryCommandBegin));

    TransitionDepthImage(lPassCmd, testLights.sunlight.shadowMap.image, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);

    frame.resources.gPassFramebuffer.TransitionImagesForDescriptors(lPassCmd);
    TransitionImage(lPassCmd, frame.resources.lPassDrawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    {
        FunctionTimeMeasure measure{ stats.lPassDrawTime };
        deferredPasses.DrawLPass(lPassCmd, frame.resources.lPassDrawImage, frame.resources.gPassFramebuffer);
    }

    PreparePresentableImage(lPassCmd, frame); //TODO rename to OnFrameRenderEnd

    DrawImgui(lPassCmd, swapchain.GetImage(), swapchain.GetImageView(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_CHECK(vkEndCommandBuffer(lPassCmd));

    const auto& sync = frame.GetSync();
    QueueGPUWork(lPassCmd, { sync.shadowsSemaphore, sync.gPassSemaphore }, sync.lPassSemaphore, sync.renderFence);

    frame.ReaddCommandPool(threadCmdPool);
}

void Vel::Renderer::QueueGPUWork(VkCommandBuffer cmd, const std::vector<VkSemaphore>&& wait, VkSemaphore signal, VkFence fence)
{
    VkCommandBufferSubmitInfo lPassBufferSubmitInfo = CreateCommandBufferSubmitInfo(cmd);
    std::vector<VkSemaphoreSubmitInfo> waitSemaphores(wait.size());
    for (int i = 0; i < wait.size(); ++i)
    {
        waitSemaphores[i] = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, wait[i]);
    }
    VkSemaphoreSubmitInfo signalInfo = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, signal);
    VkSubmitInfo2 submitInfo = CreateSubmitInfo(lPassBufferSubmitInfo, waitSemaphores.data(), wait.size(), &signalInfo, 1);

    std::lock_guard lock(graphicsQueueMutex);
    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, fence));
}

void Vel::Renderer::Draw()
{
    FunctionTimeMeasure measure{ stats.frametime };

    if (swapchain.IsAwaitingResize())
    {
        vkDeviceWaitIdle(device);
        swapchain.Resize(window); //TODO fix matrix
    }

    //UpdateScene();
    ++frameNumber;
    auto& currentFrame = GetCurrentFrame();

    fmt::println("--------------------------------------------------");
    fmt::println("You spin me right round .                   Frame ID {}", frameNumber);
    while (currentFrame.GetSync().inProgress.load()){} //TODO temp spinlock
    fmt::println("Awaiting this frame previous work done .    Frame ID {}", frameNumber);
    AwaitFramePreviousRenderDone(currentFrame);
    fmt::println("This frame previous render await finish.    Frame ID {}", frameNumber);

    currentFrame.StartNew(frameNumber);

    if (frames[(frameNumber + 1) % FRAME_DATA_SIZE].GetSync().preparing.load())
    {
        //With 2 frames this condition means that the work queue is empty, so we can put other frame's prepare there
        //Probably not, because other frame will still wait for Fence, which by when the 
        renderThreadPool.SetWorkFrame(&frames[(frameNumber + 1) % FRAME_DATA_SIZE]);
        fmt::println("Previous frame moved to work.           Frame ID {}", frameNumber);
    }

    currentFrame.GetSync().inProgress.store(true);
    currentFrame.GetSync().preparing.store(true);
    //In this place the other thread can move prepare to work and so one prepare will be lost
    renderThreadPool.SetPrepareFrame(&currentFrame);
    fmt::println("Frame set for prepare.                 Frame ID {}", frameNumber);

    currentFrame.AddWork(GENERAL, [&]() {
        UpdateScene();
        UpdateFrameDescriptors();
        for (auto& context : currentFrame.resources.gPassDrawContexts)
        {
            context.Clear();
        }
        currentFrame.resources.gPassDrawContexts.clear();

        swapchain.AcquireNextImageIndex(currentFrame.GetSync().swapchainSemaphore);
        if (swapchain.IsAwaitingResize())
        {
            currentFrame.Discard();
        }

        fmt::println("General work done.                     Frame ID {}", currentFrame.GetFrameIdx());
    }, true);

    currentFrame.AddQueueWorkFinishCallback(GENERAL, [&]() {
        fmt::println("Start Skybox queue.                    Frame ID {}", currentFrame.GetFrameIdx());
        SkyboxDraw(currentFrame);
        fmt::println("General cb done.                       Frame ID {}", currentFrame.GetFrameIdx());
    });

    currentFrame.AddQueueWorkFinishCallback(MAIN_CONTEXT, [&]() {
        //GPassCommandRecord(currentFrame);
        fmt::println("Main context cb done.                  Frame ID {}", currentFrame.GetFrameIdx());
    });
    currentFrame.AddWork(MAIN_CONTEXT, [&]() {
        for (int i = 0; i < 25; ++i)
            GPassContextWork(loadedScenes["model"], modelMatrix2, currentFrame.resources.gPassDrawContexts);
    });
    currentFrame.AddWork(MAIN_CONTEXT, [&]() {
        for (int i = 0; i < 25; ++i)
            GPassContextWork(loadedScenes["model"], modelMatrix2, currentFrame.resources.gPassDrawContexts);
    });
    currentFrame.AddWork(MAIN_CONTEXT, [&]() {
        for (int i = 0; i < 25; ++i)
            GPassContextWork(loadedScenes["model"], modelMatrix2, currentFrame.resources.gPassDrawContexts);
    });
    currentFrame.AddWork(MAIN_CONTEXT, [&]() {
        fmt::println("Main context work done.                Frame ID {}", currentFrame.GetFrameIdx());
    }, true);

    currentFrame.AddWork(SHADOW_CONTEXT, [&]() {
        fmt::println("Start Shadows context/queue.           Frame ID {}", currentFrame.GetFrameIdx());
        LightSourceShadowWork(currentFrame);
        fmt::println("Shadows context/queue work done.       Frame ID {}", currentFrame.GetFrameIdx());
    }, true);

    currentFrame.AddWork(PREPARED_FRAME_HANDLER, [&]() {
        fmt::println("Frame prepared.                        Frame ID {}", currentFrame.GetFrameIdx());
        //LightSourceShadowWork(currentFrame);
        //Await fence

        while (frames[(currentFrame.GetFrameIdx() + 1) % FRAME_DATA_SIZE].GetSync().rendering.load()){}
        currentFrame.GetSync().rendering.store(true);
        renderThreadPool.SetWorkFrame(&currentFrame);
    }, true);

    currentFrame.AddWork(MAIN_COMMANDS_RECORD, [&]() {
        fmt::println("Start GPass queue.                     Frame ID {}", currentFrame.GetFrameIdx());
        GPassCommandRecord(currentFrame);
        fmt::println("Main commmands done.                   Frame ID {}", currentFrame.GetFrameIdx());
    }, true);

    currentFrame.AddWork(SHADOW_COMMANDS_RECORD, [&]() {
        //LightSourceShadowWork(currentFrame);
        fmt::println("Shadows commands work done (MOCK).     Frame ID {}", currentFrame.GetFrameIdx());
    }, true);

    currentFrame.AddWork(LPASS_COMMANDS_RECORD, [&]() {
        fmt::println("Start LPass queue.                     Frame ID {}", currentFrame.GetFrameIdx());
        LPassCommandRecord(currentFrame); //Signals render fence
        fmt::println("Finish LPass queue.                    Frame ID {}", currentFrame.GetFrameIdx());

        swapchain.PresentImage(&currentFrame.GetSync().lPassSemaphore, graphicsQueue);
        currentFrame.LockQueues();
        currentFrame.GetSync().rendering.store(false);
        currentFrame.GetSync().inProgress.store(false);
        fmt::println("Finish present.                        Frame ID {}", currentFrame.GetFrameIdx());
    }, true);

    fmt::println("Work added.                            Frame ID {}", frameNumber);

    PrepareImguiFrame();

    //TODO fix draw extent in deferred after resize, update in passes
    const VkExtent2D& swapchainSize = swapchain.GetImageSize();
    //drawExtent.width = static_cast<uint32_t>(std::min(swapchainSize.width, drawExtent.width) * renderScale);
    //drawExtent.height = static_cast<uint32_t>(std::min(swapchainSize.height, drawExtent.height) * renderScale);
    drawExtent.width = static_cast<uint32_t>(swapchainSize.width * renderScale);
    drawExtent.height = static_cast<uint32_t>(swapchainSize.height * renderScale);

    //TODO temp spinlock
    fmt::println("Waiting for previous frame.            Frame ID {}", frameNumber);
    
    //Should await some CPU work
    fmt::println("Start frame work.                      Frame ID {}", frameNumber);
    //renderThreadPool.StartFrameWork();
    currentFrame.UnlockQueuesLock(PREPARE_CPU_DATA);
}

void Vel::Renderer::PreparePresentableImage(VkCommandBuffer cmd, FrameData& frame)
{
    VkImage presentableImage = VK_NULL_HANDLE;
    VkImageLayout transitionSrcLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    AllocatableBuffer& buff = gpuAllocator.GetStagingBuffer();

    TransitionImage(cmd, swapchain.GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    auto& framebuffer = frame.resources.gPassFramebuffer;
    switch (imageToPresent)
    {
    case 1:
        presentableImage = framebuffer.position.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case 2:
        presentableImage = framebuffer.color.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case 3:
        presentableImage = framebuffer.normals.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case 4:
        presentableImage = framebuffer.metallicRoughness.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        break;
    case 5:
        presentableImage = testLights.sunlight.shadowMap.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        TransitionDepthImage(cmd, presentableImage, transitionSrcLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        CopyDepthToColorImage(cmd, presentableImage, buff, swapchain.GetImage(), testLights.sunlight.shadowMap.extent);
        break;
    default:
        presentableImage = frame.resources.lPassDrawImage.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        break;
    }

    if (imageToPresent != 5)
    {
        VkExtent3D swapchainExtent = { swapchain.GetImageSize().width, swapchain.GetImageSize().height };
        TransitionImage(cmd, presentableImage, transitionSrcLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        BlitImage(cmd, presentableImage, swapchain.GetImage(), drawExtent, swapchainExtent);
    }
}

void Vel::Renderer::PrepareImguiFrame()
{
    vImgui.PrepareFrame([&]() {
        ImGui::Text("Background");
        ImGui::SliderFloat("Render Scale", &renderScale, 0.3f, 1.f);
        ImGui::Text("Camera");
        ImGui::Text("position  %f  %f  %f", mainCamera.GetPosition().x, mainCamera.GetPosition().y, mainCamera.GetPosition().z);
        ImGui::SliderFloat("Speed", &mainCamera.speed, 0.001f, 1.f);
        ImGui::Text("Perf");
        ImGui::Text("frametime %f ms", stats.frametime);
        ImGui::Text("context creation %f ms", stats.contextCreation);
        ImGui::Text("gpass drawtime %f ms", stats.gPassDrawTime);
        ImGui::Text("gpass average %f ms", stats.gPassesAverage);
        ImGui::Text("lpass drawtime %f ms", stats.lPassDrawTime);
        ImGui::Text("scene update %f ms", stats.sceneUpdateTime);
    });
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
    //mainDrawContext.opaqueSurfaces.resize(MaterialInstance::instancesCount);
    //mainDrawContext.transparentSurfaces.resize(MaterialInstance::instancesCount);
}

void Vel::Renderer::InitTestLightData()
{
    testLights.lights.ambient = glm::vec4{ 0.05f, 0.05f, 0.05f, 1.0f };

    VkExtent3D shadowMapResolution = {
        .width = 1024,
        .height = 1024,
        .depth = 1,
    };
    testLights.sunlight.InitShadowData(gpuAllocator, shadowMapResolution);
    testLights.sunlight.InitLightData(glm::normalize(glm::vec4{ 0.5f, 1.f, 0.5f, 1.0f }), glm::vec4{ 1.0f, 0.85f, 0.3f, 1.0f });

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
    skyboxPass.Init(device, defaultCubeImage);
}

void Vel::Renderer::InitShadowPass()
{
    shadowPass.Init(device, testLights.sunlight);
}

void Vel::Renderer::InitDeferred()
{
    deferredPasses.Init(device,sceneCameraDataDescriptorLayout, testLights.lightsDataBuffer.buffer, sizeof(LightData), testLights.sunlight.shadowMap.imageView);
    deferredPasses.SetRenderExtent(VkExtent2D{drawExtent.width, drawExtent.height});
}

void Vel::Renderer::InitTestTextures()
{
    uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
    whiteImage = gpuAllocator.CreateImageFromData((void*)&white, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

    uint32_t normals = glm::packUnorm4x8(glm::vec4(0.5f, 0.5f, 1.0f, 0.0f));
    defaultNormalsImage = gpuAllocator.CreateImageFromData((void*)&normals, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
    RenderableGLTF::defaultNormalsImage = defaultNormalsImage.image; //TODO temp until asset manager

    uint32_t metallicRoughness = glm::packUnorm4x8(glm::vec4(0.0f, 1.0f, 1.0f, 0.0f));
    defaultMetallicRoughnessImage = gpuAllocator.CreateImageFromData((void*)&metallicRoughness, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
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
    errorCheckerboardImage = gpuAllocator.CreateImageFromData((void*)pixels.data(), VkExtent3D{16, 16, 1}, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
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

    defaultCubeImage = gpuAllocator.CreateCubeImageFromData(cubeImages, front.GetSize(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

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

void Vel::Renderer::CreateFrameData()
{
    for (auto& frame : frames)
    {
        frame.Init(device, graphicsQueueFamily);
        renderThreadPool.SetupFrameData(frame);

        frame.resources.gPassFramebuffer = deferredPasses.CreateUnallocatedFramebuffer(drawExtent);
        gpuAllocator.AllocateImage(frame.resources.gPassFramebuffer.position);
        gpuAllocator.AllocateImage(frame.resources.gPassFramebuffer.color);
        gpuAllocator.AllocateImage(frame.resources.gPassFramebuffer.normals);
        gpuAllocator.AllocateImage(frame.resources.gPassFramebuffer.metallicRoughness);
        gpuAllocator.AllocateImage(frame.resources.gPassFramebuffer.depth);
        deferredPasses.SetFramebufferDescriptor(frame.resources.gPassFramebuffer);

        frame.resources.lPassDrawImage = deferredPasses.CreateUnallocatedLPassDrawImage(drawExtent);
        gpuAllocator.AllocateImage(frame.resources.lPassDrawImage);

        //TODO TEMP; we'll probably move that to GPUImage Deallocate()
        delQueue.Push([&]() {
            gpuAllocator.DestroyImage(frame.resources.gPassFramebuffer.position);
            gpuAllocator.DestroyImage(frame.resources.gPassFramebuffer.color);
            gpuAllocator.DestroyImage(frame.resources.gPassFramebuffer.normals);
            gpuAllocator.DestroyImage(frame.resources.gPassFramebuffer.metallicRoughness);
            gpuAllocator.DestroyImage(frame.resources.gPassFramebuffer.depth);
            gpuAllocator.DestroyImage(frame.resources.lPassDrawImage);
        });
    }
}

void Vel::Renderer::CreateCommandsInfo()
{
    primaryCommandBegin = VkCommandBufferBeginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };
}

VkCommandBuffer Vel::Renderer::CreateCommandBuffer(VkCommandPool pool, VkCommandBufferLevel level)
{
    VkCommandBuffer buffer;

    VkCommandBufferAllocateInfo cmdAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool,
        .level = level,
        .commandBufferCount = 1
    };

    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocateInfo, &buffer));

    return buffer;
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

        frameInfo.GetSceneData().frameDescriptors = DescriptorAllocatorDynamic{};
        frameInfo.GetSceneData().frameDescriptors.InitPool(device, 100, frameSizes);

        frameInfo.GetSceneData().cameraDataBuffer = gpuAllocator.CreateBuffer(sizeof(SceneCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        frameInfo.GetSceneData().cameraDescriptorSet = frameInfo.GetSceneData().frameDescriptors.Allocate(sceneCameraDataDescriptorLayout);
        DescriptorWriter writer;
        writer.WriteBuffer(0, frameInfo.GetSceneData().cameraDataBuffer.buffer, sizeof(SceneCameraData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        writer.UpdateSet(device, frameInfo.GetSceneData().cameraDescriptorSet);

        frameInfo.cleanupQueue.Push([buffer = frameInfo.GetSceneData().cameraDataBuffer, this]() {
            gpuAllocator.DestroyBuffer(buffer);
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
        renderThreadPool.Cleanup();
        vkDeviceWaitIdle(device);

        loadedScenes.clear();

        shadowPass.Cleanup();
        skyboxPass.Cleanup();
        deferredPasses.Cleanup();

        for (auto& frame : frames)
        {
            frame.Cleanup();
        }

        delQueue.Flush();

        swapchain.DestroySwapchain();

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyDevice(device, nullptr);
        vkb::destroy_debug_utils_messenger(instance, debugMessenger);
        vkDestroyInstance(instance, nullptr);
    }
}
