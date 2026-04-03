#ifndef SCENE_HPP
#define SCENE_HPP

#include "Scene/SceneType.hpp"

class Scene {
public:
    Scene(SceneType type)
        : type(type) {
    };

    virtual ~Scene() = default;

    virtual void update(const float dt) = 0;

private:
    SceneType type = SceneType::None;

};

#endif