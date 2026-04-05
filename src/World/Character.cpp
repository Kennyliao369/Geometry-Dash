#include "World/Character.hpp"

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
            m_Velocity.y = m_JumpSpeed;
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
