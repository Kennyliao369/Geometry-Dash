#include "Util/LoadTextFile.hpp"

#include "Level/LevelLoader.hpp"
#include "World/ObjectType.hpp"
#include "World/WorldObject.hpp"

#include "World/TriggerObject.hpp"
#include "World/HazardObject.hpp"
#include "World/DecorationObject.hpp"
#include "World/SolidObject.hpp"

#include "Util/Image.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
    enum class Section {
        NONE,
        META,
        OBJECTS
    };

    std::vector<std::string> splitWhitespace(const std::string& line) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;

        while (iss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string trim(const std::string& text) {
        const std::size_t begin = text.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos) {
            return "";
        }

        const std::size_t end = text.find_last_not_of(" \t\r\n");
        return text.substr(begin, end - begin + 1);
    }

    void parseMeta(LevelData& levelData, const std::vector<std::string>& tokens) {
        if (tokens.size() < 2) {
            throw std::runtime_error("meta line requires: key value");
        }

        if (tokens[0] == "player_spawn_x") {
            levelData.meta.playerSpawn.x = std::stof(tokens[1]);
        }
        else if (tokens[0] == "player_spawn_y") {
            levelData.meta.playerSpawn.y = std::stof(tokens[1]);
        }
    }


    void parseObject(LevelData& levelData, const std::vector<std::string>& tokens) {
        if (tokens.size() < 7) {
            throw std::runtime_error(
                "object line requires: type x y width height material value"
            );
        }

        std::shared_ptr<World::WorldObject> object;
        if (tokens[0] == "block") {
            object = std::make_shared<SolidObject>(SolidType::BLOCK);
        } 
        else if (tokens[0] == "beacon") {
            object = std::make_shared<DecorationObject>(DecorationType::BEACON);
        }
        else if (tokens[0] == "ground") {
            object = std::make_shared<SolidObject>(SolidType::GROUND);
        } 
        else if (tokens[0] == "spike") {
            object = std::make_shared<HazardObject>(HazardType::SPIKE);
        }
        else if (tokens[0] == "acid") {
            object = std::make_shared<HazardObject>(HazardType::ACID);
        }
        else if (tokens[0] == "portal") {
            object = std::make_shared<TriggerObject>(TriggerType::PORTAL);
        }
        else if (tokens[0] == "pad") {
            object = std::make_shared<TriggerObject>(TriggerType::PAD);
        }
        else if (tokens[0] == "coin") {
            object = std::make_shared<TriggerObject>(TriggerType::COIN);
        }
        else {
            throw std::runtime_error("unknown object type: " + tokens[0]);
        }


        object->setPosition({
            std::stof(tokens[1]),
            std::stof(tokens[2])
        });

        object->setSize({
            std::stof(tokens[3]),
            std::stof(tokens[4])
        });

        object->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR"/Image/" + tokens[5] + ".png"));

        // object->value = tokens[6];

        if (object) {
            levelData.allObjects.push_back(object);
        }
    }
}

LevelData LevelLoader::LoadFromFile(const std::string& filePath) {
    LevelData levelData;

    const std::string content = Util::LoadTextFile(filePath);
    std::istringstream stream(content);
    std::string rawLine;

    Section currentSection = Section::NONE;

    while (std::getline(stream, rawLine)) {
        const std::string line = trim(rawLine);

        if (line.empty()) {
            continue;
        }
        if (line[0] == '#') {
            continue;
        }

        if (line == "[meta]") {
            currentSection = Section::META;
            continue;
        }

        if (line == "[objects]") {
            currentSection = Section::OBJECTS;
            continue;
        }

        const std::vector<std::string> tokens = splitWhitespace(line);

        switch (currentSection) {
        case Section::META:
            parseMeta(levelData, tokens);
            break;

        case Section::OBJECTS:
            parseObject(levelData, tokens);
            break;

        default:
            break;
        }
    }

    std::sort(
        levelData.allObjects.begin(),
        levelData.allObjects.end(),
        [](const std::shared_ptr<World::WorldObject>& a,
           const std::shared_ptr<World::WorldObject>& b) {
            return a->getPosition().x < b->getPosition().x;
        }
    );

    return levelData;
}
