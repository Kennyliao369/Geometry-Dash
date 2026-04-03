#ifndef LEVELLOADER_HPP
#define LEVELLOADER_HPP

#include <string>

#include "Level/LevelData.hpp"

class LevelLoader {
public:
    static LevelData LoadFromFile(const std::string& filePath);
};

#endif