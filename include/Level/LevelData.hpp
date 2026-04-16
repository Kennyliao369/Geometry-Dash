#ifndef LEVELDATA_HPP
#define LEVELDATA_HPP

#include <memory>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include "Util/Color.hpp"

#include "World/WorldObject.hpp"

struct LevelBgmData {
    std::string path = "";
    int volume = 100;   // 0 ~ 128
};

struct MetaData {
    LevelBgmData bgm;
    glm::vec2 playerSpawn = {0.0f, 0.0f};
    Util::Color backgroundColor = Util::Color::FromRGB(58, 53, 254);
};

struct LevelData {
    MetaData meta;
    std::vector<std::shared_ptr<World::WorldObject>> allObjects;
};

#endif