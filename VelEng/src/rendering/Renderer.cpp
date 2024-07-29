#pragma once

#include "Renderer.h"


#include "VulkanTypes.h"
#include "VulkanUtils.h"
#include "vkbootstrap/VkBootstrap.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

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

    mainCamera.SetPosition(glm::vec3(0, 0, 5));

    CreateAllocator();
    CreateSwapchain(windowExtent.width, windowExtent.height);
    CreateDrawImage();
    CreateCommands();
    CreateSyncStructures();
    CreateDescriptors();
    CreatePipelines();

    InitTestTextures();
    InitTestData();
    InitTestLightData();
    InitDeferred();

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

    ImGui_ImplVulkan_InitInfo initInfo {
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
            .pColorAttachmentFormats = &swapchainImageFormat
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
        if (sdlEvent->key.keysym.sym == SDLK_1)
        {
            ++blitTarget;
            blitTarget %= 3;
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
    
    loadedScenes["model"]->Draw(glm::mat4{ 1.f }, mainDrawContext);

    UpdateGlobalLighting();
    UpdateCamera();
}

void Vel::Renderer::UpdateGlobalLighting()
{
    float movement1 = sin(frameNumber / 30.f) * 10.0f;
    float movement2 = cos(frameNumber / 30.f) * 10.0f;
    testLights.pointLightsGPUData[0] = {
        .position = {movement1 + 5.0f, 10.0f, -5.0f + movement2, 0.0f},
        .color = {0.2f, 0.2f, 1.0f, 150.0f},
    };
    testLights.pointLightsGPUData[1] = {
        .position = {15.0f, 20.0f, -20.0f + movement1, 0.0f},
        .color = {1.0f, 0.2f, 0.2f, 100.0f},
    };
}

void Vel::Renderer::UpdateCamera()
{
    mainCamera.Update();
    glm::mat4 view = mainCamera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)drawExtent.width / (float)drawExtent.height, 10000.f, 0.1f);
    projection[1][1] *= -1;

    sceneData.view = view;
    sceneData.projection = projection;
    sceneData.viewProjection = projection * view;
}

void Vel::Renderer::UpdateGlobalDescriptors()
{
    AllocatedBuffer sceneCameraDataBuffer = gpuAllocator.CreateBuffer(sizeof(SceneCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    GetCurrentFrame().frameCleanupQueue.Push([=, this]() {
        gpuAllocator.DestroyBuffer(sceneCameraDataBuffer);
    });

    SceneCameraData* sceneCameraGPUData = (SceneCameraData*)sceneCameraDataBuffer.allocation->GetMappedData();
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

    if (resizeRequested)
    {
        OnWindowResize();
    }

    UpdateScene();

    vImgui.PrepareFrame([&]()
        {
            ImGui::Text("Background");
            ImGui::SliderFloat("Render Scale", &renderScale, 0.3f, 1.f);
            ImGui::Text("Camera");
            ImGui::Text("position  %f  %f  %f", mainCamera.GetPosition().x, mainCamera.GetPosition().y, mainCamera.GetPosition().z);
            ImGui::SliderFloat("Speed", &mainCamera.speed, 0.1f, 1.f);
            ImGui::Text("Perf");
            ImGui::Text("frametime %f ms", stats.frametime);
            ImGui::Text("drawtime %f ms", stats.dedicatedMaterialDrawTime);
            ImGui::Text("gpass drawtime %f ms", stats.gPassDrawTime);
            ImGui::Text("gpass drawtime %f ms", stats.lPassDrawTime);
            ImGui::Text("scene update %f ms", stats.sceneUpdateTime);
        });

    drawExtent.width = static_cast<uint32_t>(std::min(swapchainExtent.width, drawImage.imageExtent.width) * renderScale);
    drawExtent.height = static_cast<uint32_t>(std::min(swapchainExtent.height, drawImage.imageExtent.height) * renderScale);

    constexpr uint32_t FRAME_TIMEOUT = 1000000000;
    auto& currentFrame = GetCurrentFrame();

    VK_CHECK(vkWaitForFences(device, 1, &currentFrame.renderFence, true, FRAME_TIMEOUT));
    currentFrame.frameCleanupQueue.Flush();
    currentFrame.frameDescriptors.ClearPools();

    VK_CHECK(vkResetFences(device, 1, &currentFrame.renderFence));

    uint32_t swapchainImageIndex;
    VkResult acquireResult = vkAcquireNextImageKHR(device, swapchain, FRAME_TIMEOUT, currentFrame.swapchainSemaphore, nullptr, &swapchainImageIndex);
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        resizeRequested = true;
        return;
    }

    UpdateGlobalDescriptors();

    auto& currentCmdBuffer = currentFrame.commandBuffer;
    VK_CHECK(vkResetCommandBuffer(currentCmdBuffer, 0));

    VkCommandBufferBeginInfo cmdBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    VK_CHECK(vkBeginCommandBuffer(currentCmdBuffer, &cmdBufferBeginInfo));
    TransitionImage(currentCmdBuffer, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    {
        FunctionTimeMeasure measure{ stats.gPassDrawTime };
        deferred.DrawGPass(mainDrawContext, currentCmdBuffer);
    }

    VK_CHECK(vkEndCommandBuffer(currentCmdBuffer));

    VkCommandBufferSubmitInfo cmdBufferInfo = CreateCommandBufferSubmitInfo(currentCmdBuffer);
    VkSemaphoreSubmitInfo waitSemaphoreInfo = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, currentFrame.swapchainSemaphore);
    VkSemaphoreSubmitInfo signalSemaphoreInfo = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, deferred.GetGPassSemaphore());
    VkSubmitInfo2 submitInfo = CreateSubmitInfo(cmdBufferInfo, &waitSemaphoreInfo, &signalSemaphoreInfo);

    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

    auto& lPassCmd = currentFrame.lPassCommands;
    VK_CHECK(vkResetCommandBuffer(lPassCmd, 0));
    VK_CHECK(vkBeginCommandBuffer(lPassCmd, &cmdBufferBeginInfo));

    {
        FunctionTimeMeasure measure{ stats.lPassDrawTime };
        deferred.DrawLPass(mainDrawContext, lPassCmd);
    }

    TransitionImage(lPassCmd, deferred.GetDrawImage().image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    BlitImage(lPassCmd, deferred.GetDrawImage().image, swapchainImages[swapchainImageIndex], drawExtent, swapchainExtent);

    DrawImgui(lPassCmd, swapchainImages[swapchainImageIndex], swapchainImageViews[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK_CHECK(vkEndCommandBuffer(lPassCmd));

    VkCommandBufferSubmitInfo lPassBufferSubmitInfo = CreateCommandBufferSubmitInfo(lPassCmd);
    VkSemaphoreSubmitInfo waitInfo = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, deferred.GetGPassSemaphore());
    VkSemaphoreSubmitInfo signalInfo = CreateSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, deferred.GetLPassSemaphore());
    VkSubmitInfo2 lPassSubmitInfo = CreateSubmitInfo(lPassBufferSubmitInfo, &waitInfo, &signalInfo);

    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &lPassSubmitInfo, currentFrame.renderFence));

    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &deferred.GetLPassSemaphore(),
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &swapchainImageIndex
    };

    VkResult presentResult = vkQueuePresentKHR(graphicsQueue, &presentInfo);
    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        resizeRequested = true;
        return;
    }

    frameNumber++;
}

void Vel::Renderer::DrawCompute(VkCommandBuffer cmdBuffer)
{
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, gradientPipeline.GetPipeline());
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, gradientPipeline.GetPipelineLayout(), 0, 1, &drawImageDescriptors, 0, nullptr);

    vkCmdDispatch(cmdBuffer, static_cast<uint32_t>(std::ceil(drawExtent.width / 16.0f)), static_cast<uint32_t>(std::ceil(drawExtent.height / 16.0f)), 1);
}

void Vel::Renderer::DrawGeometry(VkCommandBuffer cmdBuffer)
{
    FunctionTimeMeasure measure{ stats.dedicatedMaterialDrawTime };

    VkRenderingAttachmentInfo colorAttachmentInfo = BuildColorAttachmentInfo(drawImage.imageView);
    VkRenderingAttachmentInfo depthAttachmentInfo = BuildDepthAttachmentInfo();
    VkRenderingInfo renderInfo = BuildGeometryDrawRenderInfo(&colorAttachmentInfo, 1, &depthAttachmentInfo);
    VkViewport viewport = BuildGeometryDrawViewport();
    VkRect2D scissor = BuildGeometryDrawScissors();

    vkCmdBeginRendering(cmdBuffer, &renderInfo);

    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

    MaterialInstance* lastMaterial = nullptr;
    MaterialPipeline* lastPipeline = nullptr;
    VkBuffer lastIndexBuffer = VK_NULL_HANDLE;

    auto drawSurface = [&](const RenderData& drawData)
    {
        VkPipelineLayout& layout = drawData.materialData->pipeline->pipelineLayout;

        lastMaterial = drawData.materialData;
        if (drawData.materialData->pipeline != lastPipeline)
        {
            vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawData.materialData->pipeline->pipeline);
            //viewproj + light
            vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &sceneCameraDataDescriptorSet, 0, nullptr);
        }

        if (drawData.indexBuffer != lastIndexBuffer)
        {
            lastIndexBuffer = drawData.indexBuffer;
            vkCmdBindIndexBuffer(cmdBuffer, drawData.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        }

        GPUDrawPushConstants pushConstants {
            .worldMatrix = drawData.transform,
            .vertexBuffer = drawData.vertexBufferAddress
        };

        vkCmdPushConstants(cmdBuffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

        vkCmdDrawIndexed(cmdBuffer, drawData.indexCount, 1, drawData.firstIndex, 0, 0);
    };

    for (const auto& materialDraws : mainDrawContext.opaqueSurfaces)
    {
        if(!materialDraws.empty())
        {
            auto material = materialDraws[0].materialData;
            vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipeline->pipelineLayout, 1, 1, &material->descriptorSet, 0, nullptr);
            for (const auto& drawData : materialDraws)
            {
                drawSurface(drawData);
            }
        }
    }

    for (const auto& materialDraws : mainDrawContext.transparentSurfaces)
    {
        if (!materialDraws.empty())
        {
            auto material = materialDraws[0].materialData;
            vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipeline->pipelineLayout, 1, 1, &material->descriptorSet, 0, nullptr);
            for (const auto& drawData : materialDraws)
            {
                drawSurface(drawData);
            }
        }
    }

    vkCmdEndRendering(cmdBuffer);
}

void Vel::Renderer::DrawImgui(VkCommandBuffer cmdBuffer, VkImage drawImage, VkImageView drawImageView, VkImageLayout srcLayout, VkImageLayout dstLayout)
{
    TransitionImage(cmdBuffer, drawImage, srcLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    vImgui.Draw(cmdBuffer, drawImageView, swapchainExtent);
    TransitionImage(cmdBuffer, drawImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, dstLayout);
}

VkRenderingAttachmentInfo Vel::Renderer::BuildColorAttachmentInfo(VkImageView imageView)
{
    VkRenderingAttachmentInfo colorAttachment
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
    };

    return colorAttachment;
}

VkRenderingAttachmentInfo Vel::Renderer::BuildGPassAttachmentInfo(VkImageView imageView)
{
    VkRenderingAttachmentInfo colorAttachment
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue =
        {
            .color = {0, 0, 0, 0}
        }
    };

    return colorAttachment;
}

VkRenderingAttachmentInfo Vel::Renderer::BuildLPassAttachmentInfo(VkImageView imageView)
{
    VkRenderingAttachmentInfo colorAttachment
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };

    return colorAttachment;
}

VkRenderingAttachmentInfo Vel::Renderer::BuildDepthAttachmentInfo()
{
    VkRenderingAttachmentInfo depthAttachment
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = depthImage.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue
        {
            .depthStencil
            {
                .depth = 0.f
            }
        }
    };

    return depthAttachment;
}

VkRenderingInfo Vel::Renderer::BuildGeometryDrawRenderInfo(VkRenderingAttachmentInfo* color, uint32_t colorAttachmentsCount, VkRenderingAttachmentInfo* depth)
{
    VkRenderingInfo renderInfo {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext = nullptr,
        .renderArea = { .offset = { 0, 0 }, .extent = drawExtent },
        .layerCount = 1,
        .colorAttachmentCount = colorAttachmentsCount,
        .pColorAttachments = color,
        .pDepthAttachment = depth,
        .pStencilAttachment = nullptr
    };

    return renderInfo;
}

VkViewport Vel::Renderer::BuildGeometryDrawViewport()
{
    VkViewport viewport {
        .x = 0,
        .y = 0,
        .width = (float)drawExtent.width,
        .height = (float)drawExtent.height,
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    return viewport;
}

VkRect2D Vel::Renderer::BuildGeometryDrawScissors()
{
    VkRect2D scissorsRect {
        .offset = {
            .x = 0,
            .y = 0
        },
        .extent = {
            .width = drawExtent.width,
            .height = drawExtent.height
        }
    };
    return scissorsRect;
}

void Vel::Renderer::OnWindowResize()
{
    vkDeviceWaitIdle(device);

    DestroySwapchain();

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    CreateSwapchain(width, height);

    resizeRequested = false;
}

void Vel::Renderer::InitTestData()
{
    auto structureFile = loadGltf(device, GET_MESH_PATH("house"), this);

    assert(structureFile.has_value());

    loadedScenes["model"] = *structureFile;

    //TODO size per surface type material
    mainDrawContext.opaqueSurfaces.resize(MaterialInstance::instancesCount);
    mainDrawContext.transparentSurfaces.resize(MaterialInstance::instancesCount);
}

void Vel::Renderer::InitTestLightData()
{
    testLights.lightsDataBuffer = gpuAllocator.CreateBuffer(sizeof(LightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    LightData* lightsGPUData = (LightData*)testLights.lightsDataBuffer.allocation->GetMappedData();

    testLights.lights.ambient = glm::vec4{ 0.1f, 0.1f, 0.1f, 1.0f };
    testLights.lights.sunlightDirection = glm::normalize(glm::vec4{ 0.5f, -0.5f, 0.5f, 1.0f });
    testLights.lights.sunlightColor = glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f };
    testLights.lights.pointLightsCount = 2;

    testLights.pointLightsBuffer = gpuAllocator.CreateBuffer(sizeof(PointLight) * testLights.lights.pointLightsCount, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    testLights.pointLightsGPUData = (PointLight*)testLights.pointLightsBuffer.allocation->GetMappedData();

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
    testLights.pointLightsBufferAddress = vkGetBufferDeviceAddress(device, &deviceAdressInfo);

    *lightsGPUData = {
        .ambient = glm::vec4{ 0.1f, 0.1f, 0.1f, 1.0f},
        .sunlightDirection = glm::normalize(glm::vec4{ 0.5f, -0.5f, 0.5f, 1.0f}),
        .sunlightColor = glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f },
        .pointLightsCount = testLights.lights.pointLightsCount,
        .pointLightBuffer = testLights.pointLightsBufferAddress
    };

    delQueue.Push([&]() {
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

    rectVertices[0].color = { 0,0, 0,1 };
    rectVertices[1].color = { 0.5,0.5,0.5 ,1 };
    rectVertices[2].color = { 1,0, 0,1 };
    rectVertices[3].color = { 0,1, 0,1 };

    std::array<uint32_t, 6> rectIndices;

    rectIndices[0] = 0;
    rectIndices[1] = 1;
    rectIndices[2] = 2;

    rectIndices[3] = 2;
    rectIndices[4] = 1;
    rectIndices[5] = 3;

    return UploadMesh(rectIndices, rectVertices);
}

void Vel::Renderer::InitDeferred()
{
    deferred.Init(device, &gpuAllocator, drawImage.imageExtent, sceneCameraDataDescriptorLayout, testLights.lightsDataBuffer.buffer, sizeof(LightData), CreateRectangle());
}

void Vel::Renderer::InitTestTextures()
{
    uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
    whiteImage = gpuAllocator.CreateImage((void*)&white, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);

    uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
    uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
    std::array<uint32_t, 16 * 16 > pixels;
    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
        }
    }
    errorCheckerboardImage = gpuAllocator.CreateImage((void*)&magenta, VkExtent3D{ 16, 16, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, false);
    RenderableGLTF::errorCheckerboardImage = errorCheckerboardImage.image; //TODO temp until asset manager

    VkSamplerCreateInfo samplerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST
    };

    vkCreateSampler(device, &samplerCreateInfo, nullptr, &defaultSamplerNearest);

    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;

    vkCreateSampler(device, &samplerCreateInfo, nullptr, &defaultSamplerLinear);

    delQueue.Push([&]() {
        vkDestroySampler(device, defaultSamplerNearest, nullptr);
        vkDestroySampler(device, defaultSamplerLinear, nullptr);

        gpuAllocator.DestroyImage(whiteImage);
        gpuAllocator.DestroyImage(errorCheckerboardImage);
    });
}

void Vel::Renderer::CreateSwapchain(uint32_t width, uint32_t height)
{
    vkb::SwapchainBuilder swapchainBuilder{ physicalDevice, device, surface };

    swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .set_desired_format(VkSurfaceFormatKHR{ .format = swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(width, height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    swapchainExtent = vkbSwapchain.extent;
    swapchain = vkbSwapchain.swapchain;
    swapchainImages = vkbSwapchain.get_images().value();
    swapchainImageViews  = vkbSwapchain.get_image_views().value();
}

void Vel::Renderer::CreateDrawImage()
{
    VkExtent3D drawImageExtent {
        .width = drawExtent.width,
        .height = drawExtent.height,
        .depth = 1
    };

    VkImageUsageFlags drawImageUsages { 
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
        VK_IMAGE_USAGE_STORAGE_BIT | 
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 
    };

    drawImage = gpuAllocator.CreateImage(drawImageExtent, VK_FORMAT_R16G16B16A16_SFLOAT, drawImageUsages, false);
    depthImage = gpuAllocator.CreateImage(drawImageExtent, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, false);

    delQueue.Push([&]() {
        gpuAllocator.DestroyImage(drawImage);
        gpuAllocator.DestroyImage(depthImage);
    });
}

Vel::GPUMeshBuffers Vel::Renderer::UploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices)
{
    const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

    GPUMeshBuffers newSurface;

    newSurface.vertexBuffer = gpuAllocator.CreateBuffer(vertexBufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    newSurface.indexBuffer = gpuAllocator.CreateBuffer(indexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo deviceAdressInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = newSurface.vertexBuffer.buffer
    };
    newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(device, &deviceAdressInfo);

    //TODO create a single staging, resize it first launch, keep max size in config?
    AllocatedBuffer staging = gpuAllocator.CreateBuffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    void* data = staging.allocation->GetMappedData();

    memcpy(data, vertices.data(), vertexBufferSize);
    memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

    gpuAllocator.ImmediateSubmit([&](VkCommandBuffer cmd) {
        VkBufferCopy vertexCopy {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = vertexBufferSize
        };

        vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

        VkBufferCopy indexCopy {
            .srcOffset = vertexBufferSize,
            .dstOffset = 0,
            .size = indexBufferSize
        };

        vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy);
        });

    gpuAllocator.DestroyBuffer(staging);

    return newSurface;
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

        VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocateInfo, &frameInfo.commandBuffer));
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
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frame.renderSemaphore));
        frame.cleanupQueue.Push([&]()
        {
            vkDestroySemaphore(device, frame.renderSemaphore, nullptr);
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

void Vel::Renderer::CreateDescriptors()
{
    std::vector<DescriptorPoolSizeRatio> sizes =
    {
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f }
    };

    globalDescriptorAllocator.InitPool(device, 10, sizes);

    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    drawImageDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_COMPUTE_BIT);
    drawImageDescriptors = globalDescriptorAllocator.Allocate(drawImageDescriptorLayout);

    builder.Clear();
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    sceneCameraDataDescriptorLayout = builder.Build(device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

    DescriptorWriter writer;
    writer.WriteImage(0, drawImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    writer.UpdateSet(device, drawImageDescriptors);

    for (auto& frameInfo : frames)
    {
        std::vector<DescriptorPoolSizeRatio> frameSizes = {
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4}
        };

        frameInfo.frameDescriptors = DescriptorAllocatorDynamic{};
        frameInfo.frameDescriptors.InitPool(device, 1000, frameSizes);

        delQueue.Push([&]() {
            frameInfo.frameDescriptors.Cleanup();
        });
    }


    delQueue.Push([&]() {
        globalDescriptorAllocator.Cleanup();
        vkDestroyDescriptorSetLayout(device, drawImageDescriptorLayout, nullptr);
        vkDestroyDescriptorSetLayout(device, sceneCameraDataDescriptorLayout, nullptr);
        });
}

void Vel::Renderer::CreatePipelines()
{
    gltfMaterialPipeline.Init(device, sceneCameraDataDescriptorLayout, drawImage.imageFormat, depthImage.imageFormat);
    gltfMaterialPipeline.BuildPipelines();
    gradientPipeline.SetDevice(device);
    gradientPipeline.CreatePipeline(&drawImageDescriptorLayout, 1);

    delQueue.Push([&]() {
        gltfMaterialPipeline.ClearResources();
        gradientPipeline.Cleanup();
        });
}

void Vel::Renderer::DestroySwapchain()
{
    for (auto& swapchainImageView : swapchainImageViews)
    {
        vkDestroyImageView(device, swapchainImageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

VkSemaphoreSubmitInfo Vel::Renderer::CreateSemaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore)
{
    VkSemaphoreSubmitInfo info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semaphore,
        .value = 1,
        .stageMask = stageMask,
        .deviceIndex = 0
    };

    return info;
}

VkCommandBufferSubmitInfo Vel::Renderer::CreateCommandBufferSubmitInfo(VkCommandBuffer cmdBuffer)
{
    VkCommandBufferSubmitInfo info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = nullptr,
        .commandBuffer = cmdBuffer,
        .deviceMask = 0
    };

    return info;
}

VkSubmitInfo2 Vel::Renderer::CreateSubmitInfo(VkCommandBufferSubmitInfo &cmdBufferInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo, VkSemaphoreSubmitInfo* signalSemaphoreInfo)
{
    VkSubmitInfo2 info {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nullptr,

        .waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0U : 1U,
        .pWaitSemaphoreInfos = waitSemaphoreInfo,

        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdBufferInfo,

        .signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0U : 1U,
        .pSignalSemaphoreInfos = signalSemaphoreInfo
    };

    return info;
}

void Vel::Renderer::Cleanup()
{
    if (isInitialized)
    {
        vkDeviceWaitIdle(device);

        loadedScenes.clear();

        deferred.Cleanup();

        for (auto& frame : frames)
        {
            frame.cleanupQueue.Flush();
            frame.frameCleanupQueue.Flush();
        }

        delQueue.Flush();

        DestroySwapchain();

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyDevice(device, nullptr);
        vkb::destroy_debug_utils_messenger(instance, debugMessenger);
        vkDestroyInstance(instance, nullptr);
    }
}

Vel::Renderer::FrameData& Vel::Renderer::GetCurrentFrame()
{
    return frames[frameNumber % FRAME_DATA_SIZE];
}
