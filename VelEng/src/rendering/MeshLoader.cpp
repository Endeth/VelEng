#include "MeshLoader.h"
#include "Renderer.h"
#include "Images.h"

#include <memory>
#include <unordered_map>
#include <fstream>
#include <iostream>

#include "stb_image.h"
#include <glm/gtx/quaternion.hpp>

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>

std::optional<std::vector<std::shared_ptr<Vel::MeshAsset>>> Vel::loadGltfMeshes(Renderer* renderer, const std::filesystem::path& filePath)
{
    fastgltf::GltfDataBuffer dataBuffer;
    dataBuffer.loadFromFile(filePath);

    constexpr fastgltf::Options gltfOptions = fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;

    fastgltf::Asset gltf;
    fastgltf::Parser parser{};

    auto load = parser.loadBinaryGLTF(&dataBuffer, filePath.parent_path(), gltfOptions);
    if (load)
    {
        gltf = std::move(load.get());
    }
    else
    {
        fmt::print("Failed to load gltf: {} \n", fastgltf::to_underlying(load.error()));
        return {};
    }

    std::vector<std::shared_ptr<Vel::MeshAsset>> meshes;

    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;

    for (fastgltf::Mesh& mesh : gltf.meshes)
    {
        Vel::MeshAsset newmesh;

        newmesh.name = mesh.name;

        indices.clear();
        vertices.clear();

        for (auto&& p : mesh.primitives)
        {
            GeoSurface newSurface;
            newSurface.startIndex = (uint32_t)indices.size();
            newSurface.count = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;

            size_t initialVertex = vertices.size();

            // load indexes
            fastgltf::Accessor& indexAccessor = gltf.accessors[p.indicesAccessor.value()];
            indices.reserve(indices.size() + indexAccessor.count);

            fastgltf::iterateAccessor<std::uint32_t>(gltf, indexAccessor,
                [&](std::uint32_t idx) {
                    indices.push_back(idx + initialVertex);
                });

            // load vertex positions
            fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
            vertices.resize(vertices.size() + posAccessor.count);

            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                [&](glm::vec3 v, size_t index) {
                    Vertex newVertex;
                    newVertex.position = v;
                    newVertex.normal = { 1, 0, 0 };
                    newVertex.color = glm::vec4{ 1.f };
                    newVertex.uv_x = 0;
                    newVertex.uv_y = 0;
                    vertices[initialVertex + index] = newVertex;
                });

            // load vertex normals
            auto normals = p.findAttribute("NORMAL");
            if (normals != p.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second],
                    [&](glm::vec3 v, size_t index) {
                        vertices[initialVertex + index].normal = v;
                    });
            }

            // load UVs
            auto uv = p.findAttribute("TEXCOORD_0");
            if (uv != p.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
                    [&](glm::vec2 v, size_t index) {
                        vertices[initialVertex + index].uv_x = v.x;
                        vertices[initialVertex + index].uv_y = v.y;
                    });
            }

            // load vertex colors
            auto colors = p.findAttribute("COLOR_0");
            if (colors != p.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).second],
                    [&](glm::vec4 v, size_t index) {
                        vertices[initialVertex + index].color = v;
                    });
            }
            newmesh.surfaces.push_back(newSurface);
        }

        // display the vertex normals
        constexpr bool OverrideColors = true;
        if (OverrideColors) {
            for (Vertex& vertex : vertices) {
                vertex.color = glm::vec4(vertex.normal, 1.f);
            }
        }
        newmesh.meshBuffers = renderer->UploadMesh(indices, vertices);

        meshes.emplace_back(std::make_shared<MeshAsset>(std::move(newmesh)));
    }

    return meshes;
}

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

std::optional<std::shared_ptr<Vel::RenderableGLTF>> Vel::loadGltf(VkDevice device, const std::filesystem::path& filePath, Renderer* renderer)
{
    GPUAllocator* allocator = renderer->GetAllocator();
    //fmt::print("Loading GLTF: {}", filePath.c_str());

    std::shared_ptr<RenderableGLTF> scene = std::make_shared<RenderableGLTF>();
    scene->device = device;
    scene->allocator = allocator;
    RenderableGLTF& sceneData = *scene.get();

    fastgltf::Parser parser{};

    constexpr auto gltfOptions = fastgltf::Options::DontRequireValidAssetMember
        | fastgltf::Options::AllowDouble
        | fastgltf::Options::LoadGLBBuffers
        | fastgltf::Options::LoadExternalBuffers;

    fastgltf::GltfDataBuffer dataBuf;
    dataBuf.loadFromFile(filePath);

    fastgltf::Asset gltf;

    auto type = fastgltf::determineGltfFileType(&dataBuf);
    if (type == fastgltf::GltfType::glTF)
    {
        auto load = parser.loadGLTF(&dataBuf, filePath.parent_path(), gltfOptions);
        if (load)
        {
            gltf = std::move(load.get());
        }
        else
        {
            std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
            return {};
        }
    }
    else if (type == fastgltf::GltfType::GLB)
    {
        auto load = parser.loadBinaryGLTF(&dataBuf, filePath.parent_path(), gltfOptions);
        if (load)
        {
            gltf = std::move(load.get());
        }
        else
        {
            std::cerr << "Failed to load glTF: " << fastgltf::to_underlying(load.error()) << std::endl;
            return {};
        }
    }
    else
    {
        std::cerr << "Failed to determine glTF container" << std::endl;
    }

    std::vector<DescriptorPoolSizeRatio> sizes = { { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }
    };

    sceneData.descriptorPool.InitPool(device, gltf.materials.size(), sizes);

    for (fastgltf::Sampler& sampler : gltf.samplers)
    {

        VkSamplerCreateInfo sampl = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, .pNext = nullptr };
        sampl.maxLod = VK_LOD_CLAMP_NONE;
        sampl.minLod = 0;

        sampl.magFilter = ExtractFilter(sampler.magFilter.value_or(fastgltf::Filter::Nearest));
        sampl.minFilter = ExtractFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

        sampl.mipmapMode = ExtractMipmapMode(sampler.minFilter.value_or(fastgltf::Filter::Nearest));

        VkSampler newSampler;
        vkCreateSampler(device, &sampl, nullptr, &newSampler);

        sceneData.samplers.push_back(newSampler);
    }

    std::vector<std::shared_ptr<MeshAsset>> meshes;
    std::vector<std::shared_ptr<RenderableNode>> nodes;
    std::vector<AllocatedImage> images;
    std::vector<std::shared_ptr<MaterialInstance>> materials;

    for (fastgltf::Image& image : gltf.images)
    {
        std::optional<AllocatedImage> img = LoadImage(*allocator, gltf, image);

        if (img.has_value())
        {
            images.push_back(*img);
            sceneData.images[image.name.c_str()] = *img;
        }
        else
        {
            images.push_back(renderer->errorCheckerboardImage);
        }

    }

    // create buffer to hold the material data
    sceneData.materialDataBuffer = allocator->CreateBuffer(sizeof(GLTFMetallicRoughness::MaterialConstants) * gltf.materials.size(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    GLTFMetallicRoughness::MaterialConstants* sceneMaterialConstants = (GLTFMetallicRoughness::MaterialConstants*)sceneData.materialDataBuffer.info.pMappedData;
    int materialConstantsIndex = 0;

    for (fastgltf::Material& mat : gltf.materials)
    {
        std::shared_ptr<MaterialInstance> newMat = std::make_shared<MaterialInstance>();
        materials.push_back(newMat);
        sceneData.materials[mat.name.c_str()] = newMat;

        GLTFMetallicRoughness::MaterialConstants& constants = sceneMaterialConstants[materialConstantsIndex];
        constants.color.x = mat.pbrData.baseColorFactor[0];
        constants.color.y = mat.pbrData.baseColorFactor[1];
        constants.color.z = mat.pbrData.baseColorFactor[2];
        constants.color.w = mat.pbrData.baseColorFactor[3];

        constants.metallicFactor.x = mat.pbrData.metallicFactor;
        constants.metallicFactor.y = mat.pbrData.roughnessFactor;

        MaterialPass passType = mat.alphaMode == fastgltf::AlphaMode::Blend ? MaterialPass::Transparent : MaterialPass::MainColor;

        GLTFMetallicRoughness::MaterialResources materialResources;
        materialResources.colorImage = renderer->whiteImage;
        materialResources.colorSampler = renderer->defaultSamplerLinear;
        materialResources.metallicImage = renderer->whiteImage;
        materialResources.metallicSampler = renderer->defaultSamplerLinear;
        materialResources.dataBuffer = sceneData.materialDataBuffer.buffer;
        materialResources.dataBufferOffset = materialConstantsIndex * sizeof(GLTFMetallicRoughness::MaterialConstants);
        // grab textures from gltf file
        if (mat.pbrData.baseColorTexture.has_value())
        {
            size_t img = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].imageIndex.value();
            size_t sampler = gltf.textures[mat.pbrData.baseColorTexture.value().textureIndex].samplerIndex.value();

            materialResources.colorImage = images[img];
            materialResources.colorSampler = sceneData.samplers[sampler];
        }
        // build material
        *newMat = renderer->gltfMaterialPipeline.WriteMaterial(passType, materialResources, sceneData.descriptorPool);

        materialConstantsIndex++;
    }
    
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;

    for (fastgltf::Mesh& mesh : gltf.meshes)
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
            newSurface.count = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;

            size_t initial_vtx = vertices.size();

            // load indexes
            {
                fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                indices.reserve(indices.size() + indexaccessor.count);

                fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor,
                    [&](std::uint32_t idx) {
                    indices.push_back(idx + initial_vtx);
                });
            }

            // load vertex positions
            {
                fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
                vertices.resize(vertices.size() + posAccessor.count);

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                    [&](glm::vec3 v, size_t index) {
                    Vertex newvtx;
                    newvtx.position = v;
                    newvtx.normal = { 1, 0, 0 };
                    newvtx.color = glm::vec4{ 1.f };
                    newvtx.uv_x = 0;
                    newvtx.uv_y = 0;
                    vertices[initial_vtx + index] = newvtx;
                });
            }

            // load vertex normals
            auto normals = p.findAttribute("NORMAL");
            if (normals != p.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second],
                    [&](glm::vec3 v, size_t index) {
                    vertices[initial_vtx + index].normal = v;
                });
            }

            // load UVs
            auto uv = p.findAttribute("TEXCOORD_0");
            if (uv != p.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
                    [&](glm::vec2 v, size_t index) {
                    vertices[initial_vtx + index].uv_x = v.x;
                    vertices[initial_vtx + index].uv_y = v.y;
                });
            }

            // load vertex colors
            auto colors = p.findAttribute("COLOR_0");
            if (colors != p.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).second],
                    [&](glm::vec4 v, size_t index) {
                    vertices[initial_vtx + index].color = v;
                });
            }

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

        newMesh->meshBuffers = renderer->UploadMesh(indices, vertices);
    }

    for (fastgltf::Node& node : gltf.nodes)
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
                [&](fastgltf::Node::TransformMatrix matrix)
                {
                    memcpy(&newNode->localTransform, matrix.data(), sizeof(matrix));
                },
                [&](fastgltf::Node::TRS transform) 
                {
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

    for (int i = 0; i < gltf.nodes.size(); i++)
    {
        fastgltf::Node& node = gltf.nodes[i];
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

    return scene;
}
