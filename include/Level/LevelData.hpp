#ifndef LEVELDATA_HPP
#define LEVELDATA_HPP

#include <memory>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

#include "World/ObjectType.hpp"
#include "World/WorldObject.hpp"

struct MetaData {
    glm::vec2 playerSpawn = {0.0f, 0.0f};
};

struct LevelData {
    MetaData meta;
    std::vector<std::shared_ptr<World::WorldObject>> allObjects;
};

#endif