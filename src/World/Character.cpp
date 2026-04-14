#include "World/Character.hpp"

#include <algorithm>
#include <cmath>

#include <glm/trigonometric.hpp>

namespace {
    constexpr float kShipTurnSpeedDegreesPerSecond = 130.0f;
    constexpr float kShipMaxRotationDegrees = 45.0f;
}

void Character::update(const float dt) {
    m_PreviousPosition = m_WorldPosition;

    handleInput(dt);
    applyPhysics(dt);
    m_WorldPosition += m_Velocity * dt;
}

void Character::handleInput(const float dt) {
    switch (m_CharacterType) {
    case CharacterType::CUBE: {
        m_Velocity.x = m_MoveSpeed;

        if (Util::Input::IsKeyPressed(Util::Keycode::SPACE) && m_IsOnGround) {
            const float gravityMagnitude = std::abs(m_Gravity);
            const float jumpVelocity = std::sqrt(2.0f * gravityMagnitude * m_JumpHeight);

            m_Velocity.y = jumpVelocity;
            m_IsOnGround = false;
        }
        break;
    }

    case CharacterType::SHIP: {
        const bool pressingJump = Util::Input::IsKeyPressed(Util::Keycode::SPACE);

        float shipRotation = getRotation();
        shipRotation +=
            (pressingJump ? kShipTurnSpeedDegreesPerSecond : -kShipTurnSpeedDegreesPerSecond) * dt;
        shipRotation = std::clamp(
            shipRotation,
            -kShipMaxRotationDegrees,
            kShipMaxRotationDegrees
        );

        setRotation(shipRotation);

        // Ship 的 X 速度固定，Y 由目前機頭角度決定。
        // tan(theta) = y / x，因此 ySpeed = xSpeed * tan(theta)。
        m_Velocity.x = m_MoveSpeed;
        m_Velocity.y = std::tan(glm::radians(shipRotation)) * m_MoveSpeed;
        m_IsOnGround = false;
        break;
    }

    default:
        break;
    }
}

void Character::applyPhysics(const float dt) {
    switch (m_CharacterType) {
    case CharacterType::CUBE:
        m_Velocity.y += m_Gravity * dt;
        break;

    case CharacterType::SHIP:
        // Ship 模式的 Y 位移由 rotation 推導，不再額外套重力。
        break;

    default:
        break;
    }
}