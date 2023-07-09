// 参考
// https://vulkan-tutorial.com/
// https://www.youtube.com/watch?v=8AXTNMMWBGg

#include "parser.h"
#include "screensaver.h"

#include <iostream>

int main() {
  Parser parser;
  parser.parse("./scripts/sample.txt");
  Screensaver app = Screensaver();
  parser.apply(&app);
  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
