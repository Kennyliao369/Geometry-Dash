#include "Scene/GameplayScene.hpp"
#include "Scene/SceneType.hpp"

GameplayScene::GameplayScene()
    : Scene(SceneType::Gameplay) {

    m_WorldRoot.setViewConfig({1280.0f, 720.0f}, {16.0f, 12.0f});

    m_LevelData = LevelLoader::LoadFromFile(RESOURCE_DIR "/Level/level01.lvl");
    m_AllObjects = m_LevelData.allObjects;

    m_Player = std::make_shared<Character>(CharacterType::CUBE);
    m_Player->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Image/cube01.png"));
    m_Player->setPosition(m_LevelData.meta.playerSpawn);
    m_Player->setSize(glm::vec2(1.0f, 1.0f));

        
        

    m_WorldRoot.focus(m_Player);
    updateVisibleRange();
}

void GameplayScene::update(const float dt) {
    
    // m_Objects.insert(m_Objects.end(), spawnedObjects.begin(), spawnedObjects.end());
    // spawnObjects();
    
    
    m_Player->update(dt);
    /*
    updatePlayerPhysics(dt);
    resolveSolidCollisions();
    checkHazardCollisions();
    checkTriggerOverlaps();
    */
    updateVisibleRange();

    renderWorld();
    // m_UiRoot.Update();
}

void GameplayScene::updateVisibleRange() {
    const float focusX = m_WorldRoot.getFocusPosition().x;

    const float actualVisibleWidth =
    m_WorldRoot.getScreenSize().x / m_WorldRoot.getCellSize();

    const float leftBound =
        focusX - actualVisibleWidth * 0.5f - m_VisiblePadding.x;
    const float rightBound =
        focusX + actualVisibleWidth * 0.5f + m_VisiblePadding.y;

    const auto beginIt = std::lower_bound(
        m_AllObjects.begin(),
        m_AllObjects.end(),
        leftBound,
        [](const std::shared_ptr<World::WorldObject>& object, float x) {
            return object->getPosition().x < x;
        }
    );

    const auto endIt = std::upper_bound(
        m_AllObjects.begin(),
        m_AllObjects.end(),
        rightBound,
        [](float x, const std::shared_ptr<World::WorldObject>& object) {
            return x < object->getPosition().x;
        }
    );

    m_VisibleRange.x = static_cast<std::size_t>(
        std::distance(m_AllObjects.begin(), beginIt)
    );
    m_VisibleRange.y = static_cast<std::size_t>(
        std::distance(m_AllObjects.begin(), endIt)
    );
}


void GameplayScene::renderWorld() {
    m_WorldRoot.clear();
    m_WorldRoot.addObject(m_Player);
    for (std::size_t i = m_VisibleRange.x; i < m_VisibleRange.y; ++i) {
        m_WorldRoot.addObject(m_AllObjects[i]);
    }
    m_WorldRoot.update();
}
