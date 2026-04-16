#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

#include "World/WorldObject.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

#include <string>

enum class CharacterType {
    CUBE,
    SHIP
};

class Character : public World::WorldObject {
public:
    Character() = default;

    explicit Character(CharacterType characterType)
        : World::WorldObject(ObjectType::CHARACTER),
          m_CharacterType(characterType) {
    }

    ~Character() override = default;

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

    void setCharacterType(CharacterType characterType) {
        m_CharacterType = characterType;
    }

    void setJumpHeight(float jumpHeight) {
        m_JumpHeight = jumpHeight;
    }

    float getJumpHeight() const {
        return m_JumpHeight;
    }

    void setPreviousPosition(const glm::vec2& previousPosition) {
        m_PreviousPosition = previousPosition;
    }

    const glm::vec2& getPreviousPosition() const {
        return m_PreviousPosition;
    }

    World::CollisionGeometry getCollisionGeometry() const override {
        const glm::vec2 halfSize = m_WorldSize * 0.5f;

        World::PolygonGeometry polygon;
        polygon.vertices = {
            m_WorldPosition + glm::vec2{-halfSize.x, -halfSize.y},
            m_WorldPosition + glm::vec2{ halfSize.x, -halfSize.y},
            m_WorldPosition + glm::vec2{ halfSize.x,  0.0f}, // halfSize.y
            m_WorldPosition + glm::vec2{-halfSize.x,  0.0f}  //halfSize.y
        };

        return polygon;
    }

    void update(float dt);

private:
    void handleInput(const float dt);
    void applyPhysics(const float dt);
    void updateAnimation(const float dt);

    void snapCubeRotationToRightAngle();
    static float normalizeDegrees(float degrees);

private:
    CharacterType m_CharacterType = CharacterType::CUBE;

    glm::vec2 m_Velocity = {0.0f, 0.0f};
    glm::vec2 m_PreviousPosition = {0.0f, 0.0f};
    bool m_IsOnGround = false;

    float m_MoveSpeed = 31.0f / 3.0f;
    float m_Gravity = -85.0f;

    float m_JumpHeight = 2.2f;
    float m_ShipLiftSpeed = 18.0f;

    float m_CubeAirRotationSpeed = -400.0f;
};

#endif