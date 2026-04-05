#include "Level/LevelLoader.hpp"

#include "World/WorldObject.hpp"
#include "World/TriggerObject.hpp"
#include "World/HazardObject.hpp"
#include "World/DecorationObject.hpp"
#include "World/SolidObject.hpp"

#include "Util/Image.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

namespace {
    json loadJsonFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open level file: " + filePath);
        }

        return json::parse(file);
    }

    glm::vec2 parseVec2(const json& node, const std::string& key) {
        if (!node.contains(key) || !node.at(key).is_object()) {
            throw std::runtime_error("missing object field: " + key);
        }

        const auto& vec = node.at(key);

        if (!vec.contains("x") || !vec.contains("y")) {
            throw std::runtime_error("vec2 field requires x and y: " + key);
        }

        return {
            vec.at("x").get<float>(),
            vec.at("y").get<float>()
        };
    }

    void readMeta(LevelData& levelData, const json& root) {
        if (!root.contains("meta") || !root.at("meta").is_object()) {
            throw std::runtime_error("level json requires object field: meta");
        }

        const json& meta = root.at("meta");

        if (!meta.contains("playerSpawn") || !meta.at("playerSpawn").is_object()) {
            throw std::runtime_error("meta requires object field: playerSpawn");
        }

        levelData.meta.playerSpawn = parseVec2(meta, "playerSpawn");
    }

    std::shared_ptr<World::WorldObject> createObjectFromJson(const json& objectJson) {
        if (!objectJson.contains("type") || !objectJson.at("type").is_string()) {
            throw std::runtime_error("object requires string field: type");
        }
        if (!objectJson.contains("material") || !objectJson.at("material").is_string()) {
            throw std::runtime_error("object requires string field: material");
        }

        const std::string type = objectJson.at("type").get<std::string>();
        const std::string material = objectJson.at("material").get<std::string>();
        const glm::vec2 position = parseVec2(objectJson, "position");
        const glm::vec2 size = parseVec2(objectJson, "size");
        const float rotation = objectJson.value("rotation", 0.0f);

        std::shared_ptr<World::WorldObject> object;

        if (type == "block") {
            object = std::make_shared<SolidObject>(SolidType::BLOCK);
            object->setShapeType(ShapeType::BOX);
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Solid/" + material + ".png")
            );
        }
        else if (type == "ground") {
            object = std::make_shared<SolidObject>(SolidType::GROUND);
            object->setShapeType(ShapeType::BOX);
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Solid/" + material + ".png")
            );
        }
        else if (type == "spike") {
            object = std::make_shared<HazardObject>(HazardType::SPIKE);
            object->setShapeType(ShapeType::TRIANGLE);
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Hazard/" + material + ".png")
            );
        }
        else if (type == "acid") {
            object = std::make_shared<HazardObject>(HazardType::ACID);
            object->setShapeType(ShapeType::BOX);
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Hazard/" + material + ".png")
            );
        }
        else if (type == "beacon") {
            object = std::make_shared<DecorationObject>(DecorationType::BEACON);
            object->setShapeType(ShapeType::UNKNOWN);
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Decoration/" + material + ".png")
            );
        }
        else if (type == "portal") {
            object = std::make_shared<TriggerObject>(TriggerType::PORTAL);
            object->setShapeType(ShapeType::BOX);
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Trigger/" + material + ".png")
            );
        }
        else if (type == "pad") {
            object = std::make_shared<TriggerObject>(TriggerType::PAD);
            object->setShapeType(ShapeType::BOX);
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Trigger/" + material + ".png")
            );
        }
        else if (type == "coin") {
            object = std::make_shared<TriggerObject>(TriggerType::COIN);
            object->setShapeType(ShapeType::CIRCLE);
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Trigger/" + material + ".png")
            );
        }
        else {
            throw std::runtime_error("unknown object type: " + type);
        }

        object->setPosition(position);
        object->setSize(size);
        object->setRotation(rotation);

        return object;
    }

    void readObjects(LevelData& levelData, const json& root) {
        if (!root.contains("objects") || !root.at("objects").is_array()) {
            throw std::runtime_error("level json requires array field: objects");
        }

        for (const auto& objectJson : root.at("objects")) {
            levelData.allObjects.push_back(createObjectFromJson(objectJson));
        }
    }

    void sortObjectsByX(LevelData& levelData) {
        std::sort(
            levelData.allObjects.begin(),
            levelData.allObjects.end(),
            [](const std::shared_ptr<World::WorldObject>& a,
               const std::shared_ptr<World::WorldObject>& b) {
                return a->getPosition().x < b->getPosition().x;
            }
        );
    }
}

LevelData LevelLoader::LoadFromFile(const std::string& filePath) {
    LevelData levelData;

    const json root = loadJsonFile(filePath);

    readMeta(levelData, root);
    readObjects(levelData, root);
    sortObjectsByX(levelData);

    return levelData;
}
