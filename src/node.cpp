#include "screensaver.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <chrono>

Screensaver::Node *Screensaver::Node::addChild() {
  Node *node = new Node(this->pApp);
  this->children.push_back(node);
  return node;
}

void Screensaver::Node::init() {
  states = std::vector<std::tuple<float, glm::vec3, glm::vec3, glm::vec3>>(0);
  // glm::vec3 position = glm::vec3(0, 0, 0);
  // glm::vec3 rotationDegrees = glm::vec3(0, 0, 0);
  // glm::vec3 scale = glm::vec3(1, 1, 1);
  // states.push_back(std::make_tuple(0, position, rotationDegrees, scale));
}

void Screensaver::Node::addState(std::tuple<float, glm::vec3, glm::vec3, glm::vec3> state) {
  float time = std::get<0>(state);
  if (states.empty()) {
    time = 0;
  } else {
    if (time < 0.01) {
      time = 0.01;
    }
    time = time + cycle;
  }
  glm::vec3 position = std::get<1>(state);
  glm::vec3 rotationDegrees = std::get<2>(state);
  glm::vec3 scale = std::get<3>(state);
  cycle += time;
  states.push_back(std::make_tuple(time, position, rotationDegrees, scale));
}

void Screensaver::Node::assignResourcePath(std::string &texturePath, std::string &modelPath) {
  if (enableValidationLayers) {
    std::cout << "texture:" << texturePath << ", model:" << modelPath << std::endl;
  }
  if (texturePath.length() > 0) {
    this->pApp->numOfTextures += 1;
  }
  this->texturePath = texturePath;
  this->modelPath = modelPath;
}

void Screensaver::Node::render(VkCommandBuffer commandBuffer, glm::mat4 transform) {
  if (isContainModel && isContainTexture) {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pApp->pipelineLayout, 1, 1, &texture.descriptorSet, 0, nullptr);

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    if (cycle > 0) {
      time = std::fmod(time, cycle);
    }
    int currentState = (int)time % states.size();
    int nextState = (currentState + 1) % states.size();
    glm::vec3 pos, rot, scale;
    pos = std::get<1>(states.at(currentState));
    rot = std::get<2>(states.at(currentState));
    scale = std::get<3>(states.at(currentState));
    if (states.size() > 1) {
      float t1, t2;
      glm::vec3 p2, r2, s2;
      t1 = std::get<0>(states.at(currentState));
      t2 = std::get<0>(states.at(nextState));
      p2 = std::get<1>(states.at(nextState));
      r2 = std::get<2>(states.at(nextState));
      s2 = std::get<3>(states.at(nextState));

      float progress = std::abs((time - t1) / (t2 - t1));
      pos += ((p2 - pos) * progress);
      rot += +((r2 - rot) * progress);
      scale += +((s2 - scale) * progress);
    }

    transform = glm::translate(transform, pos);
    transform = glm::rotate(transform, glm::radians(rot.x), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(rot.y), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(rot.z), glm::vec3(0, 0, 1));
    transform = glm::scale(transform, scale);
    vkCmdPushConstants(commandBuffer, pApp->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);

    // https://www.youtube.com/watch?v=8AXTNMMWBGg
    VkBuffer vBuffers[] = {model.vertexBuffer};
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model.indices.size()), 1, 0, 0, 0);
  }

  for (auto child : children) {
    child->render(commandBuffer, transform);
  }
}

void Screensaver::Node::loadTexture() {
  if (texturePath.length() > 0) {
    // create texture image
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    texture.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {
      throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    pApp->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(pApp->device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(pApp->device, stagingBufferMemory);

    stbi_image_free(pixels);

    pApp->createImage(texWidth, texHeight, texture.mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture.image, texture.memory);

    pApp->transitionImageLayout(texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture.mipLevels);
    pApp->copyBufferToImage(stagingBuffer, texture.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    vkDestroyBuffer(pApp->device, stagingBuffer, nullptr);
    vkFreeMemory(pApp->device, stagingBufferMemory, nullptr);

    pApp->generateMipmaps(texture.image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, texture.mipLevels);

    // create texture image view
    texture.view = pApp->createImageView(texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, texture.mipLevels);

    // create sampler
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(pApp->physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(texture.mipLevels);
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(pApp->device, &samplerInfo, nullptr, &texture.sampler) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture sampler!");
    }

    // init descriptor create info

    // create descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pApp->descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &pApp->textureDescriptorSetLayout;

    if (vkAllocateDescriptorSets(pApp->device, &allocInfo, &texture.descriptorSet) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkDescriptorImageInfo imageInfo;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.view;
    imageInfo.sampler = texture.sampler;

    VkWriteDescriptorSet descriptorWrites{};
    descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites.dstSet = texture.descriptorSet;
    descriptorWrites.dstBinding = 0;
    descriptorWrites.dstArrayElement = 0;
    descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites.descriptorCount = 1;
    descriptorWrites.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(pApp->device, 1, &descriptorWrites, 0, nullptr);

    isContainTexture = true;
  }

  for (auto child : children) {
    child->loadTexture();
  }
}
void Screensaver::Node::loadModel() {
  if (modelPath.length() > 0) {
    // load model
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
      throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto &shape : shapes) {
      for (const auto &index : shape.mesh.indices) {
        Vertex vertex{};

        vertex.pos = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2]};

        vertex.texCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

        vertex.color = {1.0f, 1.0f, 1.0f};

        if (uniqueVertices.count(vertex) == 0) {
          uniqueVertices[vertex] = static_cast<uint32_t>(model.vertices.size());
          model.vertices.push_back(vertex);
        }

        model.indices.push_back(uniqueVertices[vertex]);
      }
    }

    // create vertex buffer
    VkDeviceSize bufferSize = sizeof(model.vertices[0]) * model.vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    pApp->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(pApp->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, model.vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(pApp->device, stagingBufferMemory);

    pApp->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, model.vertexBuffer, model.vertexBufferMemory);

    pApp->copyBuffer(stagingBuffer, model.vertexBuffer, bufferSize);

    vkDestroyBuffer(pApp->device, stagingBuffer, nullptr);
    vkFreeMemory(pApp->device, stagingBufferMemory, nullptr);

    // create index buffer
    bufferSize = sizeof(model.indices[0]) * model.indices.size();

    pApp->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    data = nullptr;
    vkMapMemory(pApp->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, model.indices.data(), (size_t)bufferSize);
    vkUnmapMemory(pApp->device, stagingBufferMemory);

    pApp->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, model.indexBuffer, model.indexBufferMemory);

    pApp->copyBuffer(stagingBuffer, model.indexBuffer, bufferSize);

    vkDestroyBuffer(pApp->device, stagingBuffer, nullptr);
    vkFreeMemory(pApp->device, stagingBufferMemory, nullptr);

    isContainModel = true;
  }
  for (auto child : children) {
    child->loadModel();
  }
}

void Screensaver::Node::destroyTexture() {
  if (isContainTexture) {
    vkDestroySampler(pApp->device, texture.sampler, nullptr);
    vkDestroyImageView(pApp->device, texture.view, nullptr);
    vkDestroyImage(pApp->device, texture.image, nullptr);
    vkFreeMemory(pApp->device, texture.memory, nullptr);
  }
  isContainTexture = false;
  for (auto child : children) {
    child->destroyTexture();
  }
}
void Screensaver::Node::destroyModel() {
  if (isContainModel) {
    vkDestroyBuffer(pApp->device, model.vertexBuffer, nullptr);
    vkFreeMemory(pApp->device, model.vertexBufferMemory, nullptr);
    vkDestroyBuffer(pApp->device, model.indexBuffer, nullptr);
    vkFreeMemory(pApp->device, model.indexBufferMemory, nullptr);
  }
  isContainModel = false;
  for (auto child : children) {
    child->destroyModel();
  }
}
