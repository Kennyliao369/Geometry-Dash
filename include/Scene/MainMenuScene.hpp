#ifndef MAINMENUSCENE_HPP
#define MAINMENUSCENE_HPP

#include "Scene/Scene.hpp"

class MainMenuScene : public Scene {
public:
    MainMenuScene();

    virtual ~MainMenuScene() = default;

    void update(const float dt) override;

private:

};

#endif