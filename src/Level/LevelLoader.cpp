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
    // tool
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
    
    std::string buildResourcePath(const std::string& relativePath) {
        if (relativePath.empty()) {
            return "";
        }
        return std::string(RESOURCE_DIR) + "/" + relativePath;
    }
    
    Util::Color parseColorObject(const json& node, const std::string& key) {
        if (!node.contains(key) || !node.at(key).is_object()) {
            throw std::runtime_error("missing object field: " + key);
        }
        const json& colorNode = node.at(key);

        if (!colorNode.contains("r") || !colorNode.at("r").is_number_integer()) {
            throw std::runtime_error(key + ".r must be integer");
        }
        if (!colorNode.contains("g") || !colorNode.at("g").is_number_integer()) {
            throw std::runtime_error(key + ".g must be integer");
        }
        if (!colorNode.contains("b") || !colorNode.at("b").is_number_integer()) {
            throw std::runtime_error(key + ".b must be integer");
        }

        const int r = std::clamp(colorNode.at("r").get<int>(), 0, 255);
        const int g = std::clamp(colorNode.at("g").get<int>(), 0, 255);
        const int b = std::clamp(colorNode.at("b").get<int>(), 0, 255);
        const int a = std::clamp(colorNode.value("a", 255), 0, 255);

        Util::Color color = Util::Color::FromRGB(r, g, b);
        color.a = a;
        return color;
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
    
    // character
    CharacterType parseCharacterTypeText(const std::string& text) {
        if (text == "cube") {
            return CharacterType::CUBE;
        }
        if (text == "ship") {
            return CharacterType::SHIP;
        }

        throw std::runtime_error("unknown character type: " + text);
    }

    // portal
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

    // backgroundColor
    Util::Color parseBackgroundColorTarget(const json& parameter) {
        return parseColorObject(parameter, "targetColor");
    }
    
    float parseBackgroundColorDuration(const json& parameter) {
        if (!parameter.contains("duration")) { return 0.0f; }

        if (!parameter.at("duration").is_number()) {
            throw std::runtime_error("parameter.duration must be a number");
        }

        return std::max(0.0f, parameter.at("duration").get<float>());
    }

    // createObject
    struct ObjectCreateInfo {
        std::string type;
        std::string material;
        glm::vec2 position;
        glm::vec2 size;
        float rotation = 0.0f;
        int repeat = 1;
        json parameter;
    };
    
    ObjectCreateInfo parseObjectCreateInfo(const json& objectJson) {
        if (!objectJson.contains("type") || !objectJson.at("type").is_string()) {
            throw std::runtime_error("object requires string field: type");
        }
        if (!objectJson.contains("material") || !objectJson.at("material").is_string()) {
            throw std::runtime_error("object requires string field: material");
        }

        ObjectCreateInfo info;
        info.type = objectJson.at("type").get<std::string>();
        info.material = objectJson.at("material").get<std::string>();
        info.position = parseVec2(objectJson, "position");
        info.size = parseVec2(objectJson, "size");
        info.rotation = objectJson.value("rotation", 0.0f);
        info.repeat = objectJson.value("repeat", 1);
        info.parameter = parseParameterObject(objectJson);

        if (info.repeat <= 0) {
            throw std::runtime_error("repeat must be greater than 0");
        }

        return info;
    }
    

    
    std::shared_ptr<World::WorldObject> createObject(const ObjectCreateInfo& info) {
        std::shared_ptr<World::WorldObject> object;

        if (info.type == "block") {
            object = std::make_shared<BlockObject>();
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Solid/" + info.material + ".png")
            );
        }
        else if (info.type == "ground") {
            object = std::make_shared<GroundObject>();
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Solid/" + info.material + ".png")
            );
        }
        else if (info.type == "spike") {
            object = std::make_shared<SpikeObject>();
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Hazard/" + info.material + ".png")
            );
        }
        else if (info.type == "acid") {
            object = std::make_shared<AcidObject>();
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Hazard/" + info.material + ".png")
            );
        }
        else if (info.type == "beacon") {
            object = std::make_shared<DecorationObject>(DecorationType::BEACON);
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Decoration/" + info.material + ".png")
            );
        }
        else if (info.type == "portal") {
            auto portal = std::make_shared<PortalObject>();
            portal->setTargetCharacterType(
                parsePortalTargetCharacterType(info.parameter)
            );
            portal->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Trigger/" + info.material + ".png")
            );
            object = portal;
        }
        else if (info.type == "pad") {
            object = std::make_shared<PadObject>();
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Trigger/" + info.material + ".png")
            );
        }
        else if (info.type == "coin") {
            object = std::make_shared<CoinObject>();
            object->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Trigger/" + info.material + ".png")
            );
        }
        else if (info.type == "backgroundColor") {
            auto backgroundColorTrigger = std::make_shared<BackgroundColorObject>();
            backgroundColorTrigger->setTargetColor(
                parseBackgroundColorTarget(info.parameter)
            );
            backgroundColorTrigger->setDuration(
                parseBackgroundColorDuration(info.parameter)
            );
            backgroundColorTrigger->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Trigger/" + info.material + ".png")
            );
            object = backgroundColorTrigger;
        }
        else {
            throw std::runtime_error("unknown object type: " + info.type);
        }

        object->setPosition(info.position);
        object->setSize(info.size);
        object->setRotation(info.rotation);

        return object;
    }
    
    std::vector<std::shared_ptr<World::WorldObject>> createObjects(const json& objectJson) {
        ObjectCreateInfo info = parseObjectCreateInfo(objectJson);

        std::vector<std::shared_ptr<World::WorldObject>> objects;
        objects.reserve(static_cast<std::size_t>(info.repeat));

        for (int i = 0; i < info.repeat; ++i) {
            ObjectCreateInfo repeatedInfo = info;
            repeatedInfo.position.x += info.size.x * static_cast<float>(i);
            objects.push_back(createObject(repeatedInfo));
        }

        return objects;
    }
    
    void readMeta(LevelData& levelData, const json& root) {
        // meta
        if (!root.contains("meta") || !root.at("meta").is_object()) {
            throw std::runtime_error("level json requires object field: meta");
        }
        const json& meta = root.at("meta");
        
        // playerSpawn
        if (!meta.contains("playerSpawn") || !meta.at("playerSpawn").is_object()) {
            throw std::runtime_error("meta requires object field: playerSpawn");
        }
        levelData.meta.playerSpawn = parseVec2(meta, "playerSpawn");
     
        // bgm
        if (!meta.contains("bgm") || !meta.at("bgm").is_object()) {
            throw std::runtime_error("meta requires object field: bgm");
        }
        const json& bgm = meta.at("bgm");
        if (!bgm.contains("path") || !bgm.at("path").is_string()) {
            throw std::runtime_error("meta requires object: bgm.path");
        }
        levelData.meta.bgm.path = buildResourcePath(bgm.at("path").get<std::string>());
        if (!bgm.contains("volume") || !bgm.at("volume").is_number_integer()) {
            throw std::runtime_error("meta requires object: bgm.volume");
        }
        levelData.meta.bgm.volume = std::clamp(bgm.at("volume").get<int>(), 0, 128);

        // background
        if (!meta.contains("backgroundColor") || !meta.at("backgroundColor").is_object()) {
            throw std::runtime_error("meta requires object field: backgroundColor");
        }
        levelData.meta.backgroundColor = parseColorObject(meta, "backgroundColor");
    
    
    }

    void readObjects(LevelData& levelData, const json& root) {
        if (!root.contains("objects") || !root.at("objects").is_array()) {
            throw std::runtime_error("level json requires array field: objects");
        }

        for (const auto& objectJson : root.at("objects")) {
            auto objects = createObjects(objectJson);
            levelData.allObjects.insert(
                levelData.allObjects.end(),
                objects.begin(),
                objects.end()
            );
        }
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
