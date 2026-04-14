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
#include <vector>

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

    json parseParameterObject(const json& node) {
        if (!node.contains("parameter")) {
            return json::object();
        }

        if (!node.at("parameter").is_object()) {
            throw std::runtime_error("parameter must be an object");
        }

        return node.at("parameter");
    }

    CharacterType parseCharacterTypeText(const std::string& text) {
        if (text == "cube") {
            return CharacterType::CUBE;
        }
        if (text == "ship") {
            return CharacterType::SHIP;
        }

        throw std::runtime_error("unknown character type: " + text);
    }

    CharacterType parsePortalTargetCharacterType(const json& parameter) {
        if (!parameter.contains("targetCharacterType")) {
            return CharacterType::CUBE;
        }

        if (!parameter.at("targetCharacterType").is_string()) {
            throw std::runtime_error("parameter.targetCharacterType must be a string");
        }

        return parseCharacterTypeText(
            parameter.at("targetCharacterType").get<std::string>()
        );
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

    std::shared_ptr<World::WorldObject> createObject(
        const std::string& type,
        const std::string& material,
        const glm::vec2& position,
        const glm::vec2& size,
        const float rotation,
        const json& parameter
    ) {
        std::shared_ptr<World::WorldObject> object;

        if (type == "block") {
            object = std::make_shared<BlockObject>();
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Solid/" + material + ".png")
            );
        }
        else if (type == "ground") {
            object = std::make_shared<GroundObject>();
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Solid/" + material + ".png")
            );
        }
        else if (type == "spike") {
            object = std::make_shared<SpikeObject>();
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Hazard/" + material + ".png")
            );
        }
        else if (type == "acid") {
            object = std::make_shared<AcidObject>();
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Hazard/" + material + ".png")
            );
        }
        else if (type == "beacon") {
            object = std::make_shared<DecorationObject>(DecorationType::BEACON);
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Decoration/" + material + ".png")
            );
        }
        else if (type == "portal") {
            auto portal = std::make_shared<PortalObject>();
            portal->setTargetCharacterType(
                parsePortalTargetCharacterType(parameter)
            );
            portal->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Trigger/" + material + ".png")
            );
            object = portal;
        }
        else if (type == "pad") {
            object = std::make_shared<PadObject>();
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Trigger/" + material + ".png")
            );
        }
        else if (type == "coin") {
            object = std::make_shared<CoinObject>();
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

    std::vector<std::shared_ptr<World::WorldObject>> createObjectsFromJson(const json& objectJson) {
        if (!objectJson.contains("type") || !objectJson.at("type").is_string()) {
            throw std::runtime_error("object requires string field: type");
        }
        if (!objectJson.contains("material") || !objectJson.at("material").is_string()) {
            throw std::runtime_error("object requires string field: material");
        }

        const std::string type = objectJson.at("type").get<std::string>();
        const std::string material = objectJson.at("material").get<std::string>();
        const glm::vec2 basePosition = parseVec2(objectJson, "position");
        const glm::vec2 size = parseVec2(objectJson, "size");
        const float rotation = objectJson.value("rotation", 0.0f);
        const int repeat = objectJson.value("repeat", 1);
        const json parameter = parseParameterObject(objectJson);

        if (repeat <= 0) {
            throw std::runtime_error("repeat must be greater than 0");
        }

        std::vector<std::shared_ptr<World::WorldObject>> objects;
        objects.reserve(static_cast<std::size_t>(repeat));

        for (int i = 0; i < repeat; ++i) {
            glm::vec2 repeatedPosition = basePosition;
            repeatedPosition.x += size.x * static_cast<float>(i);

            objects.push_back(
                createObject(type, material, repeatedPosition, size, rotation, parameter)
            );
        }

        return objects;
    }

    void readObjects(LevelData& levelData, const json& root) {
        if (!root.contains("objects") || !root.at("objects").is_array()) {
            throw std::runtime_error("level json requires array field: objects");
        }

        for (const auto& objectJson : root.at("objects")) {
            auto objects = createObjectsFromJson(objectJson);
            levelData.allObjects.insert(
                levelData.allObjects.end(),
                objects.begin(),
                objects.end()
            );
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
