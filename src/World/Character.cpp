#include "World/Character.hpp"

void Character::update(const float dt) {
    (void) dt;

    
    if (Util::Input::IsKeyPressed(Util::Keycode::UP)) // !!!
        move({0.0f, 0.1f});
    if (Util::Input::IsKeyPressed(Util::Keycode::DOWN)) // !!!
        move({0.0f, -0.1f});
    if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) // !!!
        move({0.1f, 0.0f});
    if (Util::Input::IsKeyPressed(Util::Keycode::LEFT)) // !!!
        move({-0.1f, 0.0f});
    
}
