#pragma once

#include <glm/glm.hpp>

#include <map>
#include <string>
#include <vector>

#include "screensaver.h"

using std::string;
using std::vector;

enum TokenType {
  None,    // 不明
  Str,     // 文字列
  Num,     // 数字
  Ope,     // 演算子
  Comma,   // カンマ
  StrArea, // ダブルクオーテーション
  Bracket  // カッコ
};

class Parser {
public:
  void parse(string file);
  void apply(Screensaver *pApp);

private:
  Screensaver *pApp = nullptr;
  vector<char> code = vector<char>(0);
  vector<std::pair<TokenType, string>> tokens = vector<std::pair<TokenType, string>>(0);
  std::map<string, Screensaver::Node *> nodes = std::map<string, Screensaver::Node *>();

  TokenType checkType(int idx);
  void tokenize();
  uint32_t init(uint32_t beginIdx);
  void createNode(uint32_t *beginIdx, Screensaver::Node *target);
  std::tuple<float, glm::vec3, glm::vec3, glm::vec3> createState(uint32_t *beginIdx);
  void loadCameraValues(uint32_t *beginIdx);
  float getNum(uint32_t *beginIdx);
  glm::vec3 getVec3(uint32_t *beginIdx);
};
