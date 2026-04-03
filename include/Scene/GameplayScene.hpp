#ifndef GAMEPLAYSCENE_HPP
#define GAMEPLAYSCENE_HPP

#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"

#include "World/WorldRender.hpp"
#include "World/WorldObject.hpp"

#include "World/Character.hpp"
/*
#include "World/HazardObject.hpp"
#include "World/DecorationObject.hpp"
#include "World/SolidObject.hpp"
#include "World/TriggerObject.hpp"
*/

#include "Level/LevelLoader.hpp"

#include "Scene/Scene.hpp"

class GameplayScene : public Scene {
public:
    GameplayScene();

    virtual ~GameplayScene() = default;

    void update(const float dt) override;

private:
    void updateVisibleRange();
    void updatePlayerPhysics(float dt);
    void resolveSolidCollisions();
    void checkHazardCollisions();
    void checkTriggerOverlaps();
    void renderWorld();

private:
    // Util::Renderer m_UiRoot;

    LevelData m_LevelData;
    World::WorldRender m_WorldRoot;

    std::shared_ptr<Character> m_Player;
    std::vector<std::shared_ptr<World::WorldObject>> m_AllObjects;

    glm::uvec2 m_VisibleRange = {0.0f, 0.0f};
    glm::vec2 m_VisiblePadding = {0.0f, 0.0f};
    
    
    /*
    std::vector<std::shared_ptr<SolidObject>> m_Solids;
    std::vector<std::shared_ptr<HazardObject>> m_Hazards;
    std::vector<std::shared_ptr<TriggerObject>> m_Triggers;
    std::vector<std::shared_ptr<DecorationObject>> m_Decorations;
    */
};

#endif