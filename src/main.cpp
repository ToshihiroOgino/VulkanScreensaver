#include <iostream>

#include "parser.h"
#include "screensaver.h"

int main(int argc, char *argv[]) {
  Parser parser;
  Screensaver app = Screensaver();
  if (argc > 1) {
    std::string srcPath = std::string(argv[1]);
    std::cout << "loading:" << srcPath << std::endl;
    parser.parse(srcPath);
  } else {
    std::cout << "can't run without script" << std::endl;
    return EXIT_FAILURE;
  }
  parser.apply(&app);

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
