#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

#include "WorldObject.hpp"

#include "Util/Input.hpp" // !!!
#include "Util/Keycode.hpp" //!!!

#include <string>

enum class CharacterType {
    CUBE,
    SHIP
};

class Character : public World::WorldObject {
public:
    Character() = default;

    Character(CharacterType characterType)
        :   World::WorldObject(ObjectType::CHARACTER),
            m_CharacterType(characterType) {
        
        
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

    void setOnGround(bool onGround) {
        m_IsOnGround = onGround;
    }

    bool isOnGround() const {
        return m_IsOnGround;
    }

    const glm::vec2& getVelocity() const {
        return m_Velocity;
    }

    void setVelocity(const glm::vec2& velocity) {
        m_Velocity = velocity;
    }

    void setVelocityX(float x) {
        m_Velocity.x = x;
    }

    void setVelocityY(float y) {
        m_Velocity.y = y;
    }

    CharacterType getCharacterType() const {
        return m_CharacterType;
    }

    void update(const float dt);

private:
    void handleInput();

    void applyPhysics(float dt);

private:
    CharacterType m_CharacterType = CharacterType::CUBE;

    glm::vec2 m_Velocity = {0.0f, 0.0f};
    bool m_IsOnGround = false;

    float m_MoveSpeed = 10.66666f;
    float m_Gravity = -80.0f;
    float m_JumpSpeed = 18.0f;
    float m_ShipLiftSpeed = 18.0f;

};

#endif