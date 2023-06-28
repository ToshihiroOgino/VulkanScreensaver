#include "screensaver.hpp"

#include <iostream>

int main() {
  Screensaver app;

  // TODO https://vulkan-tutorial.com/Compute_Shader

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
