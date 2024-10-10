#pragma once

#include "Rendering/Renderer.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#include "Rendering/VulkanTypes.h"
#include "Rendering/VulkanUtils.h"

#include "Rendering/Resources/Images.h"


#ifdef _DEBUG
constexpr bool useValidationLayers = true;
#else
constexpr bool useValidationLayers = false;
#endif

constexpr uint8_t POINT_LIGHTS_COUNT = 2;

// TODO TEMP
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
    features13.dynamicRendering = true;
    features13.synchronization2 = true;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;
    features12.runtimeDescriptorArray = true;
    features12.timelineSemaphore = true;

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
    CreateCommandsInfo();

    //TODO remove from here
    DescriptorLayoutBuilder layoutBuilder;
    layoutBuilder.Clear();
    layoutBuilder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    sceneCameraDataDescriptorLayout = layoutBuilder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    InitTestLightData(); //TODO divide into creating buffers and test lights
    InitShadowPass();
    InitDeferred();
    CreateFrameData();

    InitTestTextures();
    InitSkyboxPass();
    InitTestData();

    CreateFramesGPUData();

    InitImgui();

    isInitialized = true;

    renderThreadPool.Init(RENDER_THREADS_COUNT);
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
    // TODO move to engine
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

    UpdateWorldActors();

    UpdateCameraCPUBuffer();
}

void Vel::Renderer::UpdateGlobalLighting()
{
    renderTarget.GetSceneLights().sunlight.UpdateCameraPosition(mainCamera);

    /*testLights.pointLights[0] = {
        .position = light1Pos,
        .color = {0.2f, 0.2f, 1.0f, 150.0f},
    };
    testLights.pointLights[1] = {
        .position = light2Pos,
        .color = {1.0f, 0.2f, 0.2f, 100.0f},
    };*/
}

void Vel::Renderer::UpdateLightDescriptorsData(FrameData& frame)
{
    LightData* lightsGPUData = (LightData*)frame.GetSceneData().globalLightsDataBuffer.info.pMappedData;
    lightsGPUData->ambient = renderTarget.GetSceneLights().ambient;
    lightsGPUData->sunlightDirection = glm::vec4(renderTarget.GetSceneLights().sunlight.direction, 1.0f);
    lightsGPUData->sunlightColor = glm::vec4(renderTarget.GetSceneLights().sunlight.color, 1.0f);
    lightsGPUData->sunlightViewProj = renderTarget.GetSceneLights().sunlight.viewProj;
}

void Vel::Renderer::UpdateCameraCPUBuffer()
{
    mainCamera.Update();

    sceneData.view = mainCamera.GetViewMatrix();
    sceneData.projection = mainCamera.GetProjectionMatrix();
    glm::mat4 viewProj = mainCamera.GetViewProjectionMatrix();
    sceneData.viewProjection = viewProj;
    sceneData.invViewProjection = glm::inverse(viewProj);
    sceneData.position = glm::vec4(mainCamera.GetPosition(), 1.0f);
}

void Vel::Renderer::UpdateWorldActors()
{
    //Drawable actors logic
    //modelMatrix = glm::scale(glm::vec3{ 1, 1, 1 });
    //modelMatrix2 = glm::translate(glm::vec3{ 0, -50, 0 });

    //float movement1 = sin(frameNumber / 30.f) * 10.0f;
    //float movement2 = cos(frameNumber / 30.f) * 10.0f;

    //light1Pos = { movement1 + 5.0f, 2.0f, -5.0f + movement2, 0.0f };
    //light2Pos = { 15.0f, 20.0f, -20.0f + movement1, 0.0f };

    //Light logic
    //light1Matrix = glm::translate(glm::vec3{ light1Pos.x, light1Pos.y, light1Pos.z });
    //light2Matrix = glm::translate(glm::vec3{ light2Pos.x, light2Pos.y, light2Pos.z });
}

void Vel::Renderer::UpdateCameraDescriptorsData(FrameData& frame)
{
    // TODO Non-threadsafe access to global camera data
    SceneCameraData* sceneCameraGPUData = (SceneCameraData*)frame.GetSceneData().cameraDataBuffer.info.pMappedData;
    *sceneCameraGPUData = sceneData;
}

void Vel::Renderer::AwaitFrameRenderDone(FrameData& frame)
{
    VK_CHECK(vkWaitForFences(device, 1, &frame.GetSync().renderFence, true, FRAME_TIMEOUT));
}

void Vel::Renderer::AwaitTimelineSemaphore(VkSemaphore* semaphore)
{
    const uint64_t waitValue = 1;

    VkSemaphoreWaitInfo waitInfo;
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.pNext = NULL;
    waitInfo.flags = 0;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = semaphore;
    waitInfo.pValues = &waitValue;

    vkWaitSemaphores(device, &waitInfo, UINT64_MAX);
}

// TODO drawable objects grouping
void Vel::Renderer::GPassContextWork(std::vector<DrawContext>& drawContexts)
{
    // TODO renderTarget should now instances
    DrawContext workDrawContext(MaterialInstance::instancesCount);

    renderTarget.FillContext(workDrawContext);

    std::lock_guard lock(drawContextMutex);
    drawContexts.emplace_back(std::move(workDrawContext));
}

void Vel::Renderer::GPassCommandsRecord(FrameData& frame)
{
    VkCommandPool threadCmdPool = frame.GetAvailableCommandPool();
    VkCommandBuffer cmd = CreateCommandBuffer(threadCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VK_CHECK(vkResetCommandBuffer(cmd, 0));
    VK_CHECK(vkBeginCommandBuffer(cmd, &primaryCommandBegin));

    {
        FunctionTimeMeasure measure{ stats.gPassDrawTime };
        deferredPasses.DrawGPass(frame.resources.gPassDrawContexts, cmd, frame.resources.gPassFramebuffer, frame.GetSceneData().cameraDescriptorSet);
    }
    frame.resources.gPassFramebuffer.TransitionImagesForDescriptors(cmd);

    /*
    ++stats.gPassesCount;
    stats.gPassesAccTime += stats.gPassDrawTime;
    stats.gPassesAverage = stats.gPassesAccTime / stats.gPassesCount;
    //*/

    VK_CHECK(vkEndCommandBuffer(cmd));
    frame.resources.gPassDrawCommand = cmd;
    frame.ReaddCommandPool(threadCmdPool);
}

void Vel::Renderer::ShadowMapContextWork(FrameData& frame)
{
    DrawContext ctx(MaterialInstance::instancesCount);
    //loadedScenes["model"]->Draw(modelMatrix, ctx);
    //loadedScenes["lightSource"]->Draw(light1Matrix, ctx);
    //loadedScenes["lightSource"]->Draw(light2Matrix, ctx);

    renderTarget.FillContext(ctx);

    frame.resources.shadowDrawContext = std::move(ctx);
}

void Vel::Renderer::ShadowMapCommandsRecord(FrameData& frame)
{
    VkCommandPool threadCmdPool = frame.GetAvailableCommandPool();
    VkCommandBuffer shadowCmd = CreateCommandBuffer(threadCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VK_CHECK(vkResetCommandBuffer(shadowCmd, 0));
    VK_CHECK(vkBeginCommandBuffer(shadowCmd, &primaryCommandBegin));

    // Can be used by previous frame LPass, but submitting requires previous LPass to finish anyway
    TransitionDepthImage(shadowCmd, renderTarget.GetSceneLights().sunlight.shadowMap.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    // Sunlight resources not thread safe, await in queue submit
    shadowPass.Draw(frame.resources.shadowDrawContext, shadowCmd, renderTarget.GetSceneLights().sunlight);
    TransitionDepthImage(shadowCmd, renderTarget.GetSceneLights().sunlight.shadowMap.image, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL);

    VK_CHECK(vkEndCommandBuffer(shadowCmd));
    frame.resources.shadowsDrawCommand = shadowCmd;
    frame.ReaddCommandPool(threadCmdPool);
}

void Vel::Renderer::SkyboxDraw(FrameData& frame)
{
    VkCommandPool threadCmdPool = frame.GetAvailableCommandPool();
    VkCommandBuffer cmd = CreateCommandBuffer(threadCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VK_CHECK(vkResetCommandBuffer(cmd, 0));
    VK_CHECK(vkBeginCommandBuffer(cmd, &primaryCommandBegin));

    frame.resources.gPassFramebuffer.TransitionImagesForAttachment(cmd);
    {
        skyboxPass.Draw(cmd, mainCamera, frame.resources.gPassFramebuffer.color);
    }

    VK_CHECK(vkEndCommandBuffer(cmd));

    //QueueGPUWork(cmd, { frame.GetSync().swapchainSemaphore }, { frame.GetSync().skyboxWorkSemaphore }, {} );

    frame.ReaddCommandPool(threadCmdPool);

    VkCommandBufferSubmitInfo cmdSubmitInfo = CreateCommandBufferSubmitInfo(cmd);

    VkSemaphoreSubmitInfo waitSemaphore;
    waitSemaphore = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, frame.GetSync().swapchainSemaphore);
    VkSemaphoreSubmitInfo signalSemaphore;
    signalSemaphore = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, frame.GetSync().skyboxWorkSemaphore);

    VkSubmitInfo2 submitInfo = CreateSubmitInfo(cmdSubmitInfo, &waitSemaphore, 1, &signalSemaphore, 1);

    std::lock_guard lock(graphicsQueueMutex);
    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
}

void Vel::Renderer::GPassQueueSubmit(FrameData& frame)
{
    VkCommandBufferSubmitInfo cmdSubmitInfo = CreateCommandBufferSubmitInfo(frame.resources.gPassDrawCommand);

    VkSemaphoreSubmitInfo waitSemaphore;
    waitSemaphore = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, frame.GetSync().skyboxWorkSemaphore);
    VkSemaphoreSubmitInfo signalSemaphore;
    signalSemaphore = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, frame.GetSync().gPassWorkSemaphore);

    VkSubmitInfo2 submitInfo = CreateSubmitInfo(cmdSubmitInfo, &waitSemaphore, 1, &signalSemaphore, 1);

    std::lock_guard lock(graphicsQueueMutex);
    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
}

void Vel::Renderer::ShadowMapQueueSubmit(FrameData& frame)
{
    VkCommandBufferSubmitInfo cmdSubmitInfo = CreateCommandBufferSubmitInfo(frame.resources.shadowsDrawCommand);

    VkSemaphoreSubmitInfo signalSemaphore = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, frame.GetSync().shadowsWorkSemaphore);

    VkSubmitInfo2 submitInfo = CreateSubmitInfo(cmdSubmitInfo, nullptr, 0, &signalSemaphore, 1);

    std::lock_guard lock(graphicsQueueMutex);
    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
}

void Vel::Renderer::LPassCommandRecord(FrameData& frame)
{
    VkCommandPool threadCmdPool = frame.GetAvailableCommandPool();
    VkCommandBuffer cmd = CreateCommandBuffer(threadCmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VK_CHECK(vkResetCommandBuffer(cmd, 0));
    VK_CHECK(vkBeginCommandBuffer(cmd, &primaryCommandBegin));

    TransitionImage(cmd, frame.resources.lPassDrawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    {
        FunctionTimeMeasure measure{ stats.lPassDrawTime };
        deferredPasses.UpdateFramebufferDescriptor(frame.resources.gPassFramebuffer);
        deferredPasses.DrawLPass(cmd, frame.resources.lPassDrawImage,
            frame.GetSceneData().cameraDescriptorSet,
            frame.resources.gPassFramebuffer.framebufferDescriptor,
            frame.GetSceneData().globalLightsDescriptorSet);
    }

    PreparePresentableImage(cmd, frame); //TODO rename to OnFrameRenderEnd

    // TODO remove from lpass
    DrawImgui(cmd, swapchain.GetImage(frame), swapchain.GetImageView(frame), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_CHECK(vkEndCommandBuffer(cmd));

    frame.ReaddCommandPool(threadCmdPool);

    VkCommandBufferSubmitInfo cmdSubmitInfo = CreateCommandBufferSubmitInfo(cmd);

    const auto& sync = frame.GetSync();
    VkSemaphoreSubmitInfo waitSemaphores[2];
    waitSemaphores[0] = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, sync.gPassWorkSemaphore);
    waitSemaphores[1] = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, sync.shadowsWorkSemaphore);
    VkSemaphoreSubmitInfo signalSemaphore = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, sync.lPassWorkSemaphore);

    VkSubmitInfo2 submitInfo = CreateSubmitInfo(cmdSubmitInfo, waitSemaphores, 2, &signalSemaphore, 1);

    std::lock_guard lock(graphicsQueueMutex);
    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, sync.renderFence));

    frame.GetSync().workingOnGPU.store(false);
    frame.GetSync().GPUCondVar.notify_all();
}

// TODO Group resources by resources groups used by specific queues?
// right now potentialy I can have all but one thread in a state of executing inprogress frame work, while work frame has more work
void Vel::Renderer::Draw()
{
    FunctionTimeMeasure measure{ stats.frametime };

    if (swapchain.IsAwaitingResize())
    {
        vkDeviceWaitIdle(device);
        swapchain.Resize(window); //TODO fix matrix
    }

    //TODO fix draw extent in deferred after resize, update in passes
    const VkExtent2D& swapchainSize = swapchain.GetImageSize();
    drawExtent.width = static_cast<uint32_t>(swapchainSize.width * renderScale);
    drawExtent.height = static_cast<uint32_t>(swapchainSize.height * renderScale);

    auto& oldFrame = GetCurrentFrame();
    {
        auto& oldFrameSync = oldFrame.GetSync();
        std::unique_lock<std::mutex> lock(oldFrameSync.workMutex);
        oldFrameSync.CPUCondVar.wait(lock, [&]() {
            return !oldFrameSync.workingOnCPU.load();
        });
    }

    ++frameNumber;
    auto& currentFrame = GetCurrentFrame();
    renderThreadPool.SetPrepareFrame(&currentFrame);
    currentFrame.StartNew(frameNumber);

    // TODO temporary
    SetDrawContextAsFilled();
    // Always unlocked
    currentFrame.AddWork(GENERAL, [&]() {
        fmt::println("GENERAL: Start work {}", currentFrame.GetFrameIdx());
        UpdateScene();
        UpdateGlobalLighting();

        swapchain.AcquireNextImageIndex(currentFrame);
        if (swapchain.IsAwaitingResize())
        {
            currentFrame.Discard();
        }
    }, true);

    currentFrame.AddWork(MAIN_CONTEXT, [&]() {
        fmt::println("MAIN_CONTEXT: Start work {}", currentFrame.GetFrameIdx());
        //for (int i = 0; i < 25; ++i)
        GPassContextWork(currentFrame.resources.gPassDrawContexts);
    }, true);

    currentFrame.AddWork(SHADOW_CONTEXT, [&]() {
        fmt::println("SHADOW_CONTEXT: Start work {}", currentFrame.GetFrameIdx());
        ShadowMapContextWork(currentFrame);
    }, true);

    currentFrame.AddWork(MAIN_COMMANDS_RECORD, [&]() {
        fmt::println("MAIN_RECORD: Start work {}", currentFrame.GetFrameIdx());
        GPassCommandsRecord(currentFrame);
    }, true);

    currentFrame.AddWork(SHADOW_COMMANDS_RECORD, [&]() {
        fmt::println("SHADOW_RECORD: Start work {}", currentFrame.GetFrameIdx());
        ShadowMapCommandsRecord(currentFrame);
    }, true);
    
    currentFrame.AddWork(CPU_WORK_DONE, [&]() {
        fmt::println("CPU_WORK_DONE: Start work {}", currentFrame.GetFrameIdx());

        {
            auto& oldFrameSync = oldFrame.GetSync();
            std::unique_lock<std::mutex> lock(oldFrameSync.workMutex);
            oldFrameSync.GPUCondVar.wait(lock, [&]() {
                return !oldFrameSync.workingOnGPU.load();
            });
            AwaitFrameRenderDone(oldFrame);
            VK_CHECK(vkResetFences(device, 1, &currentFrame.GetSync().renderFence));
            fmt::println("GPU_WORK_DONE: Start work {}", oldFrame.GetFrameIdx());
            currentFrame.GetSync().workingOnGPU.store(true);

            // TODO make thread safe
            PrepareImguiFrame();
        }

        UpdateCameraDescriptorsData(currentFrame);
        UpdateLightDescriptorsData(currentFrame);

        renderThreadPool.SetWorkFrame(&currentFrame);
        currentFrame.GetSync().workingOnCPU.store(false);
        currentFrame.GetSync().CPUCondVar.notify_one();
    }, true);

    currentFrame.AddWork(MAIN_COMMANDS_QUEUE, [&]() {
        fmt::println("MAIN_QUEUE: Start work {}", currentFrame.GetFrameIdx());
        SkyboxDraw(currentFrame);
        GPassQueueSubmit(currentFrame);
    }, true);
    currentFrame.AddWork(SHADOW_COMMANDS_QUEUE, [&]() {
        fmt::println("SHADOW_QUEUE: Start work {}", currentFrame.GetFrameIdx());
        ShadowMapQueueSubmit(currentFrame);
    }, true);

    currentFrame.AddWork(LPASS_COMMANDS_RECORD, [&]() {
        fmt::println("LPASS_QUEUE: Start work {}", currentFrame.GetFrameIdx());
        LPassCommandRecord(currentFrame);

        swapchain.PresentImage(currentFrame, graphicsQueue);
        currentFrame.LockQueues();
    }, true);
}

void Vel::Renderer::AddToRenderScene(std::shared_ptr<IRenderable> target, const glm::mat4 transformation)
{
    renderTarget.AddModelInstance(target.get(), transformation);
}

void Vel::Renderer::AddSunlightToRenderScene(const glm::vec3& direction, const glm::vec3& color)
{
    renderTarget.SetSunlight(direction, color);
}

void Vel::Renderer::AddPointLightToRenderScene(const glm::vec3& position, const glm::vec3& color)
{
    renderTarget.AddPointLight(position, color);
}

std::shared_ptr<Vel::RenderableGLTF> Vel::Renderer::LoadGLTF(const std::filesystem::path& filePath)
{
    return gltfLoader.loadGltf(filePath).value();
}

void Vel::Renderer::SetDrawContextAsFilled()
{
    auto& frameToRender = GetFillableFrame();
    frameToRender.UnlockQueuesLock(PREPARE_CPU_DATA);
}

void Vel::Renderer::PreparePresentableImage(VkCommandBuffer cmd, FrameData& frame)
{
    VkImage presentableImage = VK_NULL_HANDLE;
    VkImageLayout transitionSrcLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    AllocatableBuffer& buff = gpuAllocator.GetStagingBuffer();

    auto swapchainImage = swapchain.GetImage(frame);
    TransitionImage(cmd, swapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

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
        presentableImage = renderTarget.GetSceneLights().sunlight.shadowMap.image;
        transitionSrcLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
        TransitionDepthImage(cmd, presentableImage, transitionSrcLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        CopyDepthToColorImage(cmd, presentableImage, buff, swapchainImage, renderTarget.GetSceneLights().sunlight.shadowMap.extent);
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
        BlitImage(cmd, presentableImage, swapchainImage, drawExtent, swapchainExtent);
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
    gltfLoader.Init(device, this, &gpuAllocator);
}

void Vel::Renderer::InitTestLightData()
{
    VkExtent3D shadowMapResolution = {
        .width = 1024,
        .height = 1024,
        .depth = 1,
    };

    auto& sceneLights = renderTarget.GetSceneLights();
    sceneLights.sunlight.InitShadowData(gpuAllocator, shadowMapResolution);
    sceneLights.sunlight.SetLightData(glm::normalize(glm::vec4{ 0.5f, 1.f, 0.5f, 1.0f }), glm::vec4{ 1.0f, 0.85f, 0.3f, 1.0f });

    delQueue.Push([&]() {
        // Shadow buffers
        gpuAllocator.DestroyImage(sceneLights.sunlight.shadowMap);
        gpuAllocator.DestroyBuffer(sceneLights.sunlight.shadowViewProj);
    });
}

void Vel::Renderer::InitSkyboxPass()
{
    skyboxPass.Init(device, defaultCubeImage);
}

void Vel::Renderer::InitShadowPass()
{
    shadowPass.Init(device);
    //TODO critical - wrong place
    shadowPass.UpdateDescriptorSet(renderTarget.GetSceneLights().sunlight);
}

void Vel::Renderer::InitDeferred()
{
    deferredPasses.Init(device, sceneCameraDataDescriptorLayout);
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

    VkSamplerCreateInfo shadowsSamplerCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST
    };
    vkCreateSampler(device, &shadowsSamplerCreateInfo, nullptr, &shadowSamplerNearest);

    delQueue.Push([&]() {
        vkDestroySampler(device, defaultSamplerLinear, nullptr);
        vkDestroySampler(device, shadowSamplerNearest, nullptr);

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

void Vel::Renderer::CreateFramesGPUData()
{
    for (auto& frameInfo : frames)
    {
        std::vector<DescriptorPoolSizeRatio> frameSizes = {
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4}
        };
        auto& frameSceneData = frameInfo.GetSceneData();

        frameSceneData.frameDescriptors = DescriptorAllocatorDynamic{};
        frameSceneData.frameDescriptors.InitPool(device, 100, frameSizes);

        //TODO cleanup
        { // Camera data
            // TODO get requirements, pass to allocator, assign frame buffer to allocator return
            frameSceneData.cameraDataBuffer = gpuAllocator.CreateBuffer(sizeof(SceneCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
            frameSceneData.cameraDescriptorSet = frameSceneData.frameDescriptors.Allocate(sceneCameraDataDescriptorLayout);

            DescriptorWriter writer;
            writer.WriteBuffer(0, frameSceneData.cameraDataBuffer.buffer, sizeof(SceneCameraData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            writer.UpdateSet(device, frameSceneData.cameraDescriptorSet);
        }

        { // Light data
            frameSceneData.globalLightsDataBuffer = gpuAllocator.CreateBuffer(sizeof(LightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
            frameSceneData.pointLightsDataBuffer = gpuAllocator.CreateBuffer(sizeof(PointLight) * POINT_LIGHTS_COUNT, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
            frameSceneData.globalLightsDescriptorSet = frameSceneData.frameDescriptors.Allocate(deferredPasses.GetLightDescriptorSetLayout());

            VkBufferDeviceAddressInfo pointLightsAdressInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .buffer = frameSceneData.pointLightsDataBuffer.buffer
            };
            LightData* lightsGPUData = (LightData*)frameSceneData.globalLightsDataBuffer.info.pMappedData;
            *lightsGPUData = {
                .ambient = renderTarget.GetSceneLights().ambient,
                .sunlightDirection = glm::vec4(renderTarget.GetSceneLights().sunlight.direction, 1.0f),
                .sunlightColor = glm::vec4(renderTarget.GetSceneLights().sunlight.color, 1.0f),
                .sunlightViewProj = renderTarget.GetSceneLights().sunlight.viewProj,

                //TODO Currently not used
                .sunlightShadowMapID = 0,

                //Changed only on dynamic lighting change
                .pointLightsCount = renderTarget.GetPointLightsCount(),
                // Const part
                .pointLightBuffer = vkGetBufferDeviceAddress(device, &pointLightsAdressInfo)
            };

            DescriptorWriter descriptorWriter;
            descriptorWriter.WriteBuffer(0, frameSceneData.globalLightsDataBuffer.buffer, sizeof(LightData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            descriptorWriter.WriteImages(1, &renderTarget.GetSceneLights().sunlight.shadowMap.imageView, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1);
            descriptorWriter.WriteSampler(2, shadowSamplerNearest);
            descriptorWriter.UpdateSet(device, frameSceneData.globalLightsDescriptorSet);
        }

        // TODO move to frame
        frameInfo.cleanupQueue.Push([&sceneData = frameInfo.GetSceneData(), this]() {
            gpuAllocator.DestroyBuffer(sceneData.cameraDataBuffer);
            gpuAllocator.DestroyBuffer(sceneData.globalLightsDataBuffer);
            gpuAllocator.DestroyBuffer(sceneData.pointLightsDataBuffer);
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

        //loadedScenes.clear();

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
