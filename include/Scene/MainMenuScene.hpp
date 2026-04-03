#ifndef MAINMEUNSCENE_HPP
#define MAINMEUNSCENE_HPP

#include "Scene/Scene.hpp"

class MainMenuScene : public Scene {
public:
    MainMenuScene();

    virtual ~MainMenuScene() = default;

    void update(const float dt) override;

private:

};

#endif