#include "World/Character.hpp"

#include <cmath>

void Character::update(const float dt) {
    handleInput();
    applyPhysics(dt);
    m_WorldPosition += m_Velocity * dt;
}

void Character::handleInput() {
    switch (m_CharacterType) {
    case CharacterType::CUBE:
        m_Velocity.x = m_MoveSpeed;

        if (Util::Input::IsKeyPressed(Util::Keycode::SPACE) && m_IsOnGround) {
            const float gravityMagnitude = std::abs(m_Gravity);
            const float jumpVelocity = std::sqrt(2.0f * gravityMagnitude * m_JumpHeight);

            m_Velocity.y = jumpVelocity;
            m_IsOnGround = false;
        }
        break;

    case CharacterType::SHIP:
        m_Velocity.x = m_MoveSpeed;
        if (Util::Input::IsKeyPressed(Util::Keycode::SPACE)) {
            m_Velocity.y = m_ShipLiftSpeed;
        }
        break;

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
        m_Velocity.y += m_Gravity * 0.6f * dt;
        break;

    default:
        break;
    }
}