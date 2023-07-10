#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "screensaver.h"

class Parser {
public:
  enum TokenType {
    None,    // 不明
    Str,     // 文字列
    Num,     // 数字
    Ope,     // 演算子
    Comma,   // カンマ
    StrArea, // ダブルクオーテーション
    Bracket  // カッコ
  };

  void parse(std::string file);
  void apply(Screensaver *pApp);

private:
  Screensaver *pApp = nullptr;
  std::vector<char> code = std::vector<char>(0);
  std::vector<std::pair<TokenType, std::string>> tokens;

  TokenType checkType(int idx);
  void tokenize();
  void createNode(uint32_t *beginIdx, Screensaver::Node *target);
  std::tuple<float, glm::vec3, glm::vec3, glm::vec3> createState(uint32_t *beginIdx);
  void loadCameraValues(uint32_t *beginIdx);
  float getNum(uint32_t *beginIdx);
  glm::vec3 getVec3(uint32_t *beginIdx);
};
