#include "parser.h"

#include "utils.h"

void Parser::parse(std::string file) {
  code = readFile(file);
  tokenize();

  // int n = 0;
  // for (auto p : tokens) {
  //   std::cout << n++ << " : " << p.first << "=" << p.second << std::endl;
  // }
}

void Parser::apply(Screensaver *pApp) {
  this->pApp = pApp;
  uint32_t idx = 0;
  while (idx < tokens.size()) {
    std::string str = tokens.at(idx).second;
    if (str == "node") {
      createNode(&idx, pApp->rootNode);
    } else if (str == "camera") {
      loadCameraValues(&idx);
    } else {
      idx++;
    }
  }
}

void Parser::createNode(uint32_t *beginIdx, Screensaver::Node *parentNode) {
  Screensaver::Node *pNode = parentNode->addChild();
  uint32_t idx = *beginIdx + 2;
  uint32_t stateCount = 0;
  while (tokens.at(idx).second != "}") {
    std::string str = tokens.at(idx).second;
    if (str == "node") {
      createNode(&idx, pNode);
    } else if (str == "resource") {
      pNode->assignResourcePath(tokens.at(idx + 3).second, tokens.at(idx + 5).second);
      idx += 8;
    } else if (str == "state") {
      auto state = createState(&idx);
      stateCount++;
      pNode->addState(state);
    } else if (str == "camera") {
      loadCameraValues(&idx);
    } else {
      idx++;
    }
  }
  if (stateCount == 0) {
    pNode->addState(std::make_tuple(1, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  }
  // }を飛ばす
  *beginIdx = idx + 1;
}

std::tuple<float, glm::vec3, glm::vec3, glm::vec3> Parser::createState(uint32_t *beginIdx) {
  uint32_t idx = *beginIdx + 2;
  float time = 1;
  glm::vec3 pos, rot, scale;
  pos = glm::vec3(0, 0, 0);
  rot = glm::vec3(0, 0, 0);
  scale = glm::vec3(1, 1, 1);
  while (tokens.at(idx).second != "}") {
    std::string str = tokens.at(idx).second;
    if (str == "time") {
      idx += 2;
      time = getNum(&idx);
      idx++;
    } else if (str == "camera") {
      loadCameraValues(&idx);
    } else {
      // それ以外の場合にはvec3を読む
      idx += 2;
      auto vec = getVec3(&idx);
      // )と;を飛ばして次のStrへ行く
      idx += 2;
      if (str == "position") {
        pos = vec;
      } else if (str == "scale") {
        scale = vec;
      } else if (str == "rotation") {
        rot = vec;
      }
    }
  }
  // }を飛ばす
  *beginIdx = idx + 1;
  return std::make_tuple(time, pos, rot, scale);
}

void Parser::loadCameraValues(uint32_t *beginIdx) {
  uint32_t idx = *beginIdx + 2;
  while (tokens.at(idx).second != "}") {
    std::string str = tokens.at(idx).second;
    if (str == "fov") {
      idx += 2;
      pApp->fovY = getNum(&idx);
      idx++;
    } else if (str == "camera") {
      loadCameraValues(&idx);
    } else {
      // それ以外の場合にはvec3を読む
      idx += 2;
      auto vec = getVec3(&idx);
      // )と;を飛ばして次のStrへ行く
      idx += 2;
      if (str == "position") {
        pApp->cameraPosition = vec;
      } else if (str == "lookAt") {
        pApp->lookAt = vec;
      } else if (str == "angularVelocity") {
        pApp->angularVelocity = vec;
      }
    }
  }
  // }を飛ばす
  *beginIdx = idx + 1;
}

float Parser::getNum(uint32_t *beginIdx) {
  bool minus = false;
  uint32_t idx = *beginIdx;
  if (tokens.at(idx).second == "-") {
    minus = true;
    idx++;
  } else if (tokens.at(idx).second == "+") {
    minus = false;
    idx++;
  }
  float result = std::stof(tokens.at(idx).second);
  if (minus) {
    result *= -1;
  }
  // std::cout << "(stof)idx:" << idx << ",val:" << result << std::endl;
  idx++;
  *beginIdx = idx;
  return result;
}

glm::vec3 Parser::getVec3(uint32_t *beginIdx) {
  (*beginIdx)++;
  glm::vec3 result = glm::vec3(0);
  result.x = getNum(beginIdx);
  (*beginIdx)++;
  result.y = getNum(beginIdx);
  (*beginIdx)++;
  result.z = getNum(beginIdx);
  return result;
}

Parser::TokenType Parser::checkType(int idx) {
  char ch = code.at(idx);
  if (ch == ',') {
    return TokenType::Comma;
  }
  if (ch == '\"') {
    return TokenType::StrArea;
  }
  if (ch == '(' || ch == ')' ||
      ch == '{' || ch == '}') {
    return TokenType::Bracket;
  }
  if (ch == '-' ||
      ch == '=' ||
      ch == ';') {
    return TokenType::Ope;
  }
  if (('0' <= ch && ch <= '9') || ch == '.') {
    return TokenType::Num;
  }
  if (('A' <= ch && ch <= 'z') || ch == '_' || ch == '/') {
    return TokenType::Str;
  }
  return TokenType::None;
}

void Parser::tokenize() {
  size_t idx = 0;
  while (idx < code.size()) {
    TokenType t = checkType(idx);
    if (t == TokenType::None) {
      idx++;
      continue;
    }

    if (t == TokenType::StrArea) {
      std::string str = "";
      idx++;
      while (idx < code.size()) {
        if (checkType(idx) == TokenType::StrArea) {
          break;
        }
        if (checkType(idx) != TokenType::None) {
          str += code.at(idx);
        }
        idx++;
      }
      tokens.push_back(std::make_pair(StrArea, str));
      idx++;
    } else if (t == TokenType::Ope ||
               t == TokenType::Bracket ||
               t == TokenType::Comma) {
      std::string str = "";
      std::string sOpe = "";
      sOpe += code.at(idx);
      tokens.push_back(std::make_pair(t, sOpe));
      idx++;
    } else {
      std::string str = "";
      str += code.at(idx);
      idx++;
      while (idx < code.size()) {
        if (checkType(idx) == TokenType::None) {
          idx++;
          continue;
        }
        if (t != checkType(idx)) {
          break;
        }
        str += code.at(idx);
        idx++;
      }
      tokens.push_back(std::make_pair(t, str));
    }
  }
}
