#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <array>
#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

VkResult CreateDebugUtilsMessengerEXT(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT *, const VkAllocationCallbacks *, VkDebugUtilsMessengerEXT *);
void DestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks *);

#pragma region structures
struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 texCoord;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
  }

  bool operator==(const Vertex &other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord;
  }
};

namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const &vertex) const {
    return ((hash<glm::vec3>()(vertex.pos) ^
             (hash<glm::vec3>()(vertex.color) << 1)) >>
            1) ^
           (hash<glm::vec2>()(vertex.texCoord) << 1);
  }
};
} // namespace std

struct UniformBufferObject {
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
};
#pragma endregion

class Screensaver {
public:
  struct Node {
  public:
    Screensaver *pApp = nullptr;

    bool isContainTexture = false;
    bool isContainModel = false;

    std::string texturePath = "";
    std::string modelPath = "";

    std::vector<Node *> children;

    // glm::vec3 position = glm::vec3(0, 0, 0);
    // glm::vec3 rotationDegrees = glm::vec3(0, 0, 0);
    // glm::vec3 scale = glm::vec3(1, 1, 1);

    uint32_t currentState = 0;
    float cycle = 0;
    std::vector<float> timeline;
    // vector<time, pos, rot, scale>
    std::vector<std::tuple<float, glm::vec3, glm::vec3, glm::vec3>> states;

    struct Texture {
      VkImage image;
      VkDeviceMemory memory;
      VkImageLayout layout;
      VkImageView view;
      uint32_t texWidth, texHeight;
      uint32_t mipLevels;
      VkSampler sampler;

      VkDescriptorSet descriptorSet;
    } texture;

    struct Model {
      std::vector<Vertex> vertices;
      std::vector<uint32_t> indices;
      VkBuffer vertexBuffer;
      VkDeviceMemory vertexBufferMemory;
      VkBuffer indexBuffer;
      VkDeviceMemory indexBufferMemory;
    } model;

    Node() { init(); }
    Node(Screensaver *pApp) {
      this->pApp = pApp;
      init();
    }

    // 追加したノードのポインタを返す
    Node *addChild();
    void addState(std::tuple<float, glm::vec3, glm::vec3, glm::vec3> state);

    void assignResourcePath(std::string &texturePath, std::string &modelPath);

    void render(VkCommandBuffer commandBuffer, glm::mat4 transform);
    void loadTexture();
    void loadModel();

    void destroyTexture();
    void destroyModel();

  private:
    void init();
  };

  Node *rootNode;
  uint32_t numOfTextures = 0;

  glm::vec3 cameraPosition = glm::vec3(5, 5, 5);
  glm::vec3 lookAt = glm::vec3(0, 0, 0);
  glm::vec3 angularVelocity = glm::vec3(0, 0, 0);
  float fovY = 60.0f;

  void run();
  Screensaver() {
    rootNode = new Node(this);
    // std::string roomTexture = "textures/checkerboard.jpg", roomModel = "models/cube_room.obj";
    // rootNode->assignResourcePath(roomTexture, roomModel);
  }

private:
#pragma region members

  GLFWwindow *window;

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkSurfaceKHR surface;

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
  VkDevice device;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  VkExtent2D swapChainExtent;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;

  VkRenderPass renderPass;
  VkDescriptorSetLayout uniformDescriptorSetLayout;
  VkDescriptorSetLayout textureDescriptorSetLayout;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  VkCommandPool commandPool;

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  VkImage colorImage;
  VkDeviceMemory colorImageMemory;
  VkImageView colorImageView;

  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void *> uniformBuffersMapped;

  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  uint32_t currentFrame = 0;

  bool framebufferResized = false;
#pragma endregion

#pragma region base
  void initWindow();
  void initVulkan();
  void mainLoop();
  void cleanup();
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void createLogicalDevice();
  void pickPhysicalDevice();
  void createSwapChain();
  void createImageViews();
  void createRenderPass();
  void createDescriptorSetLayout();
  void createGraphicsPipeline();
  void createFramebuffers();
  void createCommandPool();
  void createColorResources();
  void createDepthResources();
  void createUniformBuffers();
  void createDescriptorPool();
  void createDescriptorSets();
  void createCommandBuffers();
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void createSyncObjects();
  void drawFrame();
#pragma endregion

#pragma region utils
  void cleanupSwapChain();
  void recreateSwapChain();
  VkSampleCountFlagBits getMaxUsableSampleCount();
  void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
  void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
  VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
  void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
  VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
  VkFormat findDepthFormat();
  bool hasStencilComponent(VkFormat format);
  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  VkShaderModule createShaderModule(const std::vector<char> &code);
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  void updateUniformBuffer(uint32_t currentImage);
  bool isDeviceSuitable(VkPhysicalDevice device);
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();
#pragma endregion

#pragma region static
  static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto app = reinterpret_cast<Screensaver *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
  }
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
  }
#pragma endregion
};
