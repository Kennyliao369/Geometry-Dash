#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

#include "WorldObject.hpp"

#include "Util/Input.hpp" // !!!
#include "Util/Keycode.hpp" //!!!

#include <string>

enum class CharacterType {
    CUBE
};

class Character : public World::WorldObject {
public:
    Character() = default;

    Character(CharacterType characterType)
        : characterType(characterType) {
        
        
    }
    /*
    Character(const std::string& imagePath, const glm::vec2& worldPosition = {0.0f, 0.0f}, const glm::vec2& worldSize = {1.0f, 1.0f})
        : WorldObject(ObjectType::CHARACTER,
        std::make_shared<Util::Image>(imagePath),
        worldPosition, worldSize, 0) {}
    */

    virtual ~Character() = default;

    void move(glm::vec2 delta) {
        m_WorldPosition += delta;
    }

    void setImage(const std::string& imagePath) {
        m_Drawable = std::make_shared<Util::Image>(imagePath);
    }

    void update(const float dt);

    
private:
    CharacterType characterType = CharacterType::CUBE;
    glm::vec2 m_Velocity = {0.0f, 0.0f};
    bool m_IsOnGround = false;
    float m_Gravity = -1.0f;
    float m_JumpSpeed = 12.0f;
    float m_MoveSpeed = 1.0f;


};

#endif