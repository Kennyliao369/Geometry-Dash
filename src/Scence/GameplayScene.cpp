#include "Scene/GameplayScene.hpp"
#include "Scene/SceneType.hpp"

#include "Util/Logger.hpp"
#include "World/Collision.hpp"


namespace {
    enum class SolidHitType {
        LANDING,
        SIDE,
        CEILING
    };

    SolidHitType classifySolidHit(const World::Collision::CollisionResult& result) {
        const float absX = std::abs(result.normal.x);
        const float absY = std::abs(result.normal.y);

        constexpr float kVerticalBias = 1.0f;

        if (absY > absX * kVerticalBias) {
            if (result.normal.y < 2.5f) {
                return SolidHitType::LANDING;
            }
            return SolidHitType::CEILING;
        }

        return SolidHitType::SIDE;
    }

    bool shouldDieFromSolidHit(CharacterType characterType, SolidHitType hitType) {
        switch (characterType) {
        case CharacterType::CUBE:
            return hitType == SolidHitType::SIDE ||
                   hitType == SolidHitType::CEILING;

        case CharacterType::SHIP:
            return hitType == SolidHitType::SIDE;

        default:
            return false;
        }
    }
}

GameplayScene::GameplayScene()
    : Scene(SceneType::Gameplay) {

    m_WorldRoot.setViewConfig({1280.0f, 720.0f}, {16.0f, 12.0f});

    m_LevelData = LevelLoader::LoadFromFile(RESOURCE_DIR "/Level/level01.json");
    m_AllObjects = m_LevelData.allObjects;

    m_Player = std::make_shared<Character>(CharacterType::CUBE);
    m_Player->SetDrawable(std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Role/cube_00.png"));
    m_Player->setPosition(m_LevelData.meta.playerSpawn);
    m_Player->setSize(glm::vec2(1.0f, 1.0f));
    m_Player->setShapeType(ShapeType::BOX);

    m_WorldRoot.setFocusSmoothing({1.0f, 0.3f});
    m_WorldRoot.setTargetAnchorRatio({0.1f, 0.2f});
    m_WorldRoot.setVerticalDeadZoneRatio(0.2f);
    m_WorldRoot.focus(m_Player);
    updateVisibleRange();


    m_BackgroundRoot.setScreenSize(m_WorldRoot.getScreenSize());
    m_BackgroundRoot.setCellSize(m_WorldRoot.getCellSize());
    m_BackgroundRoot.setColor(Util::Color::FromRGB(100, 50, 40));

    // 單層背景
    m_BackgroundRoot.setImage(
    std::make_shared<Util::Image>(RESOURCE_DIR"/Image/Background/background.png"),
    0.10f
);
}

void GameplayScene::update(const float dt) {
    
    // m_Objects.insert(m_Objects.end(), spawnedObjects.begin(), spawnedObjects.end());
    // spawnObjects();
    
    // m_Player->setOnGround(false);
    m_Player->update(dt);
    
    // updatePlayerPhysics(dt);
    resolveSolidCollisions(dt);
    checkHazardCollisions();
    checkTriggerOverlaps();
    
    m_WorldRoot.updateFocus(dt);
    updateVisibleRange();

    renderWorld();
    // m_UiRoot.Update();
}


void GameplayScene::resolveSolidCollisions(const float dt) {
    const glm::vec2 desiredPosition = m_Player->getPosition();
    glm::vec2 playerVelocity = m_Player->getVelocity();

    // 由 Character::update(dt) 反推本幀起點
    glm::vec2 playerPosition = desiredPosition - playerVelocity * dt;
    const glm::vec2 moveDelta = playerVelocity * dt;

    bool groundedThisFrame = false;
    constexpr float kSkin = 0.001f;

    const glm::vec2 playerHalfSize = m_Player->getSize() * 0.5f;

    // ---------- Y phase ----------
    if (moveDelta.y != 0.0f) {
        playerPosition.y += moveDelta.y;

        m_Player->setPosition(playerPosition);
        m_Player->setVelocity(playerVelocity);

        for (std::size_t i = m_VisibleRange.x; i < m_VisibleRange.y; ++i) {
            const auto& object = m_AllObjects[i];

            if (!object) {
                continue;
            }

            if (object->getType() != ObjectType::SOLID) {
                continue;
            }

            const auto result = World::Collision::objectVsObject(*m_Player, *object);

            if (!result.hit) {
                continue;
            }

            const glm::vec2 solidCenter = object->getPosition();
            const glm::vec2 solidHalfSize = object->getSize() * 0.5f;

            if (moveDelta.y < 0.0f) {
                // 往下掉，直接視為 LANDING
                playerPosition.y = solidCenter.y + solidHalfSize.y + playerHalfSize.y + kSkin;

                if (playerVelocity.y < 0.0f) {
                    playerVelocity.y = 0.0f;
                }

                groundedThisFrame = true;
            }
            else if (moveDelta.y > 0.0f) {
                // 往上撞，直接視為 CEILING
                if (shouldDieFromSolidHit(m_Player->getCharacterType(), SolidHitType::CEILING)) {
                    m_Player->setPosition(m_LevelData.meta.playerSpawn);
                    m_Player->setVelocity({0.0f, 0.0f});
                    m_Player->setOnGround(false);

                    LOG_DEBUG("Player died from ceiling collision");
                    return;
                }

                playerPosition.y = solidCenter.y - solidHalfSize.y - playerHalfSize.y - kSkin;

                if (playerVelocity.y > 0.0f) {
                    playerVelocity.y = 0.0f;
                }
            }

            // 同步修正後狀態，避免同幀後續物件還用舊位置
            m_Player->setPosition(playerPosition);
            m_Player->setVelocity(playerVelocity);
        }
    }

    // ---------- X phase ----------
    if (moveDelta.x != 0.0f) {
        playerPosition.x += moveDelta.x;

        m_Player->setPosition(playerPosition);
        m_Player->setVelocity(playerVelocity);

        for (std::size_t i = m_VisibleRange.x; i < m_VisibleRange.y; ++i) {
            const auto& object = m_AllObjects[i];

            if (!object) {
                continue;
            }

            if (object->getType() != ObjectType::SOLID) {
                continue;
            }

            const auto result = World::Collision::objectVsObject(*m_Player, *object);

            if (!result.hit) {
                continue;
            }

            const SolidHitType hitType = classifySolidHit(result);

            // X 階段如果碰撞角度仍然明顯像 SIDE，就死亡
            // 如果角度比較像 LANDING / CEILING，代表可能只是踩到下一塊地板的邊角，先放過
            if (hitType == SolidHitType::SIDE &&
                shouldDieFromSolidHit(m_Player->getCharacterType(), SolidHitType::SIDE)) {
                m_Player->setPosition(m_LevelData.meta.playerSpawn);
                m_Player->setVelocity({0.0f, 0.0f});
                m_Player->setOnGround(false);

                LOG_DEBUG("Player died from side collision");
                return;
            }
        }
    }

    m_Player->setPosition(playerPosition);
    m_Player->setVelocity(playerVelocity);
    m_Player->setOnGround(groundedThisFrame);
}


void GameplayScene::checkHazardCollisions() {
    for (std::size_t i = m_VisibleRange.x; i < m_VisibleRange.y; ++i) {
        const auto& object = m_AllObjects[i];

        if (!object) {
            continue;
        }

        if (object->getType() != ObjectType::HAZARD) {
            continue;
        }

        const auto result = World::Collision::objectVsObject(*m_Player, *object);

        if (!result.hit) {
            continue;
        }

        m_Player->setPosition(m_LevelData.meta.playerSpawn);
        m_Player->setOnGround(false);

        LOG_DEBUG("Player hit hazard");
        return;
    }
}

void GameplayScene::checkTriggerOverlaps() {
    for (std::size_t i = m_VisibleRange.x; i < m_VisibleRange.y; ++i) {
        const auto& object = m_AllObjects[i];

        if (!object) {
            continue;
        }

        if (object->getType() != ObjectType::TRIGGER) {
            continue;
        }

        const auto result = World::Collision::objectVsObject(*m_Player, *object);

        if (!result.hit) {
            continue;
        }

        LOG_DEBUG("Player overlapped trigger");
    }
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
    m_BackgroundRoot.update(m_WorldRoot.getFocusPosition());

    m_WorldRoot.clear();
    m_WorldRoot.addObject(m_Player);
    for (std::size_t i = m_VisibleRange.x; i < m_VisibleRange.y; ++i) {
        m_WorldRoot.addObject(m_AllObjects[i]);
    }
    m_WorldRoot.update();
}


// 碰撞範圍 與 顯示範圍 可能要分開