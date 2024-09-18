#include "Rendering/MeshLoader.h"

#include <memory>
#include <unordered_map>
#include <fstream>
#include <iostream>

#include "stb_image.h"
#include <glm/gtx/quaternion.hpp>

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>

#include "Rendering/Renderer.h"
#include "Rendering/Images.h"

VkFilter ExtractFilter(fastgltf::Filter filter)
{
    switch (filter)
    {
    case fastgltf::Filter::Nearest:
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::NearestMipMapLinear:
        return VK_FILTER_NEAREST;

    case fastgltf::Filter::Linear:
    case fastgltf::Filter::LinearMipMapNearest:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
        return VK_FILTER_LINEAR;
    }
}

VkSamplerMipmapMode ExtractMipmapMode(fastgltf::Filter filter)
{
    switch (filter)
    {
    case fastgltf::Filter::NearestMipMapNearest:
    case fastgltf::Filter::LinearMipMapNearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;

    case fastgltf::Filter::NearestMipMapLinear:
    case fastgltf::Filter::LinearMipMapLinear:
    default:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
}

void Vel::MeshLoader::Init(VkDevice dev, Renderer* ren)
{
    device = dev;
    renderer = ren;
}

void Vel::MeshLoader::Cleanup()
{
    meshes.clear();
    materials.clear();
}

std::optional<std::shared_ptr<Vel::RenderableGLTF>> Vel::MeshLoader::loadGltf(const std::filesystem::path& filePath)
{
    std::shared_ptr<RenderableGLTF> scene = std::make_shared<RenderableGLTF>();
    scene->device = device;
    scene->allocator = renderer->GetAllocator();
    if(filePath.has_parent_path())
        parentPath = filePath.parent_path();

    RenderableGLTF& sceneData = *scene.get();
    fastgltf::Asset gltfAsset;

    Cleanup();

    if (!LoadAsset(filePath, gltfAsset))
        return {};

    CreateSamplers(sceneData, gltfAsset);
    CreateMaterials(sceneData, gltfAsset);
    CreateSurfaces(sceneData, gltfAsset);
    CreateNodeTree(sceneData, gltfAsset);

    return scene;
}

bool Vel::MeshLoader::LoadAsset(const std::filesystem::path& filePath, fastgltf::Asset& gltfAsset)
{
    fastgltf::Parser parser{};

    constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember
        | fastgltf::Options::AllowDouble
        | fastgltf::Options::LoadGLBBuffers
        | fastgltf::Options::LoadExternalBuffers;

    fastgltf::GltfDataBuffer dataBuf;
    dataBuf.loadFromFile(filePath);

    auto type = fastgltf::determineGltfFileType(&dataBuf);
    if (type == fastgltf::GltfType::glTF)
    {
        auto load = parser.loadGLTF(&dataBuf, filePath.parent_path(), gltfOptions);
        if (load)
        {
            gltfAsset = std::move(load.get());
        }
        else
        {
            std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
            return false;
        }
    }
    else if (type == fastgltf::GltfType::GLB)
    {
        auto load = parser.loadBinaryGLTF(&dataBuf, filePath.parent_path(), gltfOptions);
        if (load)
        {
            gltfAsset = std::move(load.get());
        }
        else
        {
            std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
            return false;
        }
    }
    else
    {
        std::cerr << "Failed to determine glTF container" << std::endl;
    }

    return true;
}

void Vel::MeshLoader::CreateSamplers(RenderableGLTF& sceneData, fastgltf::Asset& gltfAsset)
{
    for (fastgltf::Sampler& sampler : gltfAsset.samplers)
    {
        VkSamplerCreateInfo samplerInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .magFilter = ExtractFilter(sampler.magFilter.value_or(fastgltf::Filter::Nearest)),
            .minFilter = ExtractFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest)),
            .mipmapMode = ExtractMipmapMode(sampler.minFilter.value_or(fastgltf::Filter::Nearest)),
            .minLod = 0,
            .maxLod = VK_LOD_CLAMP_NONE
        };

        VkSampler newSampler;
        vkCreateSampler(device, &samplerInfo, nullptr, &newSampler);

        sceneData.samplers.push_back(newSampler);
    }
}

void Vel::MeshLoader::CreateMaterials(RenderableGLTF& sceneData, fastgltf::Asset& gltfAsset)
{
    std::vector<DescriptorPoolSizeRatio> sizes = { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }
    };

    sceneData.descriptorPool.InitPool(device, gltfAsset.materials.size(), sizes);
    std::unordered_map<size_t, AllocatedImage> images;

    size_t counter = 0;
    for (fastgltf::Image& image : gltfAsset.images)
    {
        std::optional<AllocatedImage> img = LoadGltfAssetImage(*renderer->GetAllocator(), gltfAsset, image, parentPath);
        if (img.has_value())
        {
            images[counter] = *img;
            sceneData.images[counter] = *img;
        }
        ++counter;
    }

    size_t materialBufferSize = sizeof(MaterialConstants) * gltfAsset.materials.size();
    sceneData.materialDataBuffer = renderer->GetAllocator()->CreateBuffer(materialBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    MaterialConstants* sceneMaterialConstants = (MaterialConstants*)sceneData.materialDataBuffer.info.pMappedData;
    int materialConstantsIndex = 0;

    for (fastgltf::Material& mat : gltfAsset.materials)
    {
        std::shared_ptr<MaterialInstance> newMat = std::make_shared<MaterialInstance>();
        materials.push_back(newMat);
        sceneData.materials[mat.name.c_str()] = newMat;

        MaterialConstants& constants = sceneMaterialConstants[materialConstantsIndex];
        constants.color.x = mat.pbrData.baseColorFactor[0];
        constants.color.y = mat.pbrData.baseColorFactor[1];
        constants.color.z = mat.pbrData.baseColorFactor[2];
        constants.color.w = mat.pbrData.baseColorFactor[3];
        constants.metallicRoughnessFactor.g = mat.pbrData.metallicFactor;
        constants.metallicRoughnessFactor.b = mat.pbrData.roughnessFactor;

        MaterialResources materialResources;
        materialResources.colorImage = renderer->whiteImage;
        materialResources.colorSampler = renderer->defaultSamplerLinear;
        materialResources.normalsImage = renderer->defaultNormalsImage;
        materialResources.normalsSampler = renderer->defaultSamplerLinear;
        materialResources.metallicRoughnessImage = renderer->defaultMetallicRoughnessImage;
        materialResources.metallicRoughnessSampler = renderer->defaultSamplerLinear;
        materialResources.dataBuffer = sceneData.materialDataBuffer.buffer;
        materialResources.dataBufferOffset = materialConstantsIndex * sizeof(MaterialConstants);
        // grab textures from gltf file
        if (mat.pbrData.baseColorTexture.has_value())
        {
            size_t img = gltfAsset.textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
            if (images.contains(img))
                materialResources.colorImage = images[img];

            if (gltfAsset.samplers.size() > 0)
            {
                size_t sampler = gltfAsset.textures[mat.pbrData.baseColorTexture.value().textureIndex].samplerIndex.value();
                materialResources.colorSampler = sceneData.samplers[sampler];
            }
        }
        if (mat.pbrData.metallicRoughnessTexture.has_value())
        {
            size_t img = gltfAsset.textures[mat.pbrData.metallicRoughnessTexture.value().textureIndex].imageIndex.value();
            if (images.contains(img))
                materialResources.metallicRoughnessImage = images[img];
        }
        if (mat.normalTexture.has_value())
        {
            size_t img = gltfAsset.textures[mat.normalTexture.value().textureIndex].imageIndex.value();
            if(images.contains(img))
                materialResources.normalsImage = images[img];
        }
        // build material
        //MaterialPass passType = mat.alphaMode == fastgltf::AlphaMode::Blend ? MaterialPass::Transparent : MaterialPass::MainColor;
        //*newMat = renderer->gltfMaterialPipeline.WriteMaterial(passType, materialResources, sceneData.descriptorPool); TODO material not used right now

        *newMat = renderer->GetDeferredRenderer().CreateMaterialInstance(materialResources, sceneData.descriptorPool);

        materialConstantsIndex++;
    }
}

void Vel::MeshLoader::CreateSurfaces(RenderableGLTF& sceneData, fastgltf::Asset& gltfAsset)
{
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;

    for (fastgltf::Mesh& mesh : gltfAsset.meshes)
    {
        std::shared_ptr<MeshAsset> newMesh = std::make_shared<MeshAsset>();
        meshes.push_back(newMesh);
        sceneData.meshes[mesh.name.c_str()] = newMesh;
        newMesh->name = mesh.name;

        indices.clear();
        vertices.clear();

        for (auto&& p : mesh.primitives)
        {
            GeoSurface newSurface;
            newSurface.startIndex = (uint32_t)indices.size();
            newSurface.count = (uint32_t)gltfAsset.accessors[p.indicesAccessor.value()].count;

            size_t initial_vtx = vertices.size();

            // load indexes
            {
                fastgltf::Accessor& indexaccessor = gltfAsset.accessors[p.indicesAccessor.value()];
                indices.reserve(indices.size() + indexaccessor.count);

                fastgltf::iterateAccessor<std::uint32_t>(gltfAsset, indexaccessor,
                    [&](std::uint32_t idx) {
                    indices.push_back(idx + initial_vtx);
                });
            }

            // load vertex positions
            {
                fastgltf::Accessor& posAccessor = gltfAsset.accessors[p.findAttribute("POSITION")->second];
                vertices.resize(vertices.size() + posAccessor.count);

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltfAsset, posAccessor,
                    [&](glm::vec3 v, size_t index) {
                    Vertex newvtx;
                    newvtx.position = v;
                    newvtx.normal = { 0.5f, 0.5f, 1.0f };
                    newvtx.tangent = { 1.0f, 0.0f, 0.0f, 0.0f };
                    newvtx.uv_x = 0;
                    newvtx.uv_y = 0;
                    vertices[initial_vtx + index] = newvtx;
                });
            }

            // load vertex normals
            auto normals = p.findAttribute("NORMAL");
            if (normals != p.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltfAsset, gltfAsset.accessors[(*normals).second],
                    [&](glm::vec3 v, size_t index) {
                    vertices[initial_vtx + index].normal = v;
                });
            }

            // load vertex tangent
            //fastgltf::Primitive::attribute_type
            auto tangents = p.findAttribute("TANGENT");
            if (tangents != p.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltfAsset, gltfAsset.accessors[(*tangents).second],
                    [&](glm::vec4 v, size_t index) {
                    vertices[initial_vtx + index].tangent = v;
                });
            }

            // load UVs
            auto uv = p.findAttribute("TEXCOORD_0");
            if (uv != p.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec2>(gltfAsset, gltfAsset.accessors[(*uv).second],
                    [&](glm::vec2 v, size_t index) {
                    vertices[initial_vtx + index].uv_x = v.x;
                    vertices[initial_vtx + index].uv_y = v.y;
                });
            }

            // load vertex colors
            /*auto colors = p.findAttribute("COLOR_0");
            if (colors != p.attributes.end()) 
            {
                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltfAsset, gltfAsset.accessors[(*colors).second],
                    [&](glm::vec4 v, size_t index) {
                    vertices[initial_vtx + index].color = v;
                });
            }*/

            if (p.materialIndex.has_value())
            {
                newSurface.materialInstance = materials[p.materialIndex.value()];
            }
            else
            {
                newSurface.materialInstance = materials[0];
            }

            newMesh->surfaces.push_back(newSurface);
        }

        newMesh->meshBuffers = renderer->GetAllocator()->UploadMesh(indices, vertices);
    }
}

void Vel::MeshLoader::CreateNodeTree(RenderableGLTF& sceneData, fastgltf::Asset& gltfAsset)
{
    std::vector<std::shared_ptr<RenderableNode>> nodes;
    for (fastgltf::Node& node : gltfAsset.nodes)
    {
        std::shared_ptr<RenderableNode> newNode;

        // find if the node has a mesh, and if it does hook it to the mesh pointer and allocate it with the meshnode class
        if (node.meshIndex.has_value())
        {
            newNode = std::make_shared<MeshNode>();
            static_cast<MeshNode*>(newNode.get())->mesh = meshes[*node.meshIndex];
        }
        else
        {
            newNode = std::make_shared<RenderableNode>();
        }

        nodes.push_back(newNode);
        sceneData.nodes[node.name.c_str()];

        std::visit(fastgltf::visitor
            {
                [&](fastgltf::Node::TransformMatrix matrix) {
                    memcpy(&newNode->localTransform, matrix.data(), sizeof(matrix));
                },
                [&](fastgltf::Node::TRS transform) {
                    glm::vec3 tl(transform.translation[0], transform.translation[1], transform.translation[2]);
                    glm::quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1], transform.rotation[2]);
                    glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);

                    glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
                    glm::mat4 rm = glm::toMat4(rot);
                    glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);

                    newNode->localTransform = tm * rm * sm;
                }
            },
            node.transform);
    }

    for (int i = 0; i < gltfAsset.nodes.size(); i++)
    {
        fastgltf::Node& node = gltfAsset.nodes[i];
        std::shared_ptr<RenderableNode>& sceneNode = nodes[i];

        for (auto& c : node.children)
        {
            sceneNode->children.push_back(nodes[c]);
            nodes[c]->parent = sceneNode;
        }
    }

    // find the top nodes, with no parents
    for (auto& node : nodes)
    {
        if (node->parent.lock() == nullptr)
        {
            sceneData.topNodes.push_back(node);
            node->RefreshTransform(glm::mat4{ 1.f });
        }
    }
}
