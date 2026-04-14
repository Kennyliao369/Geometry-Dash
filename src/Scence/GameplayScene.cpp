#include "Scene/GameplayScene.hpp"

#include "Util/Logger.hpp"
#include "World/Collision.hpp"
#include "World/TriggerObject.hpp"

namespace {
    enum class SolidHitType {
        LANDING,
        SIDE,
        CEILING
    };

    World::AABB mergeAabbs(const World::AABB& a, const World::AABB& b) {
        World::AABB merged;
        merged.min = {
            std::min(a.min.x, b.min.x),
            std::min(a.min.y, b.min.y)
        };
        merged.max = {
            std::max(a.max.x, b.max.x),
            std::max(a.max.y, b.max.y)
        };
        return merged;
    }

    // 計算玩家與物件在垂直方向上的最小重疊深度。
    // 數值越小，代表越接近「只是在上/下邊緣碰到」的情況。
    float calculateVerticalOverlapDepth(
        const World::AABB& playerAabb,
        const World::AABB& objectAabb
    ) {
        const float overlapToObjectTop = objectAabb.max.y - playerAabb.min.y;
        const float overlapToObjectBottom = playerAabb.max.y - objectAabb.min.y;
        return std::min(overlapToObjectTop, overlapToObjectBottom);
    }

    // 由碰撞法線推算表面斜度：
    // 0 度接近平地，90 度接近垂直牆面。
    float calculateSlopeDegrees(const glm::vec2& normal) {
        const float length = glm::length(normal);
        if (length <= 0.0001f) {
            return 90.0f;
        }

        const glm::vec2 normalizedNormal = normal / length;
        return glm::degrees(
            std::atan2(std::abs(normalizedNormal.x), std::abs(normalizedNormal.y))
        );
    }

    // 先用上下最小重疊深度做容錯，再用法線斜度判斷這次碰撞
    // 比較像上下接觸還是左右碰撞。
    SolidHitType classifySolidHit(
        const World::Collision::CollisionResult& collisionResult,
        const World::AABB& playerAabb,
        const World::AABB& objectAabb,
        const glm::vec2& stepDelta
    ) {
        constexpr float kVerticalContactTolerance = 0.5f;
        constexpr float kMaxAcceptableSlopeDegrees = 45.0f;
        constexpr float kSeparationEpsilon = 0.0005f;

        const float verticalOverlapDepth =
            calculateVerticalOverlapDepth(playerAabb, objectAabb);
        const float slopeDegrees = calculateSlopeDegrees(collisionResult.normal);

        
        // 只要上下重疊很淺，或表面斜度不超過 45 度，
        // 就把這次命中視為上下接觸。
        const bool shouldTreatAsVertical =
            verticalOverlapDepth < kVerticalContactTolerance ||
            slopeDegrees <= kMaxAcceptableSlopeDegrees;

        if (shouldTreatAsVertical) {
            // 用推出向量的 y 方向與本步移動方向，
            // 判斷這次是落地還是頂頭。
            const glm::vec2 separation =
                -collisionResult.normal * (collisionResult.depth + kSeparationEpsilon);

            if (separation.y > 0.0f || stepDelta.y < 0.0f) {
                return SolidHitType::LANDING;
            }

            return SolidHitType::CEILING;

        } else {
            return SolidHitType::SIDE;
        }
        
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

    void applyPlayerCharacterType(const std::shared_ptr<Character>& player, const CharacterType newType) {
        if (!player) {
            return;
        }

        if (player->getCharacterType() == newType) {
            return;
        }

        player->setCharacterType(newType);
        player->setRotation(0.0f);

        glm::vec2 newVelocity = player->getVelocity();
        newVelocity.y = 0.0f;
        player->setVelocity(newVelocity);
        player->setOnGround(false);

        switch (newType) {
        case CharacterType::CUBE:
            player->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Role/cube_00.png")
            );
            player->setSize(glm::vec2{1.0f, 1.0f});
            break;

        case CharacterType::SHIP:
            player->SetDrawable(
                std::make_shared<Util::Image>(RESOURCE_DIR "/Image/Role/ship_00.png")
            );
            player->setSize(glm::vec2{1.0f, 0.75f});
            break;

        default:
            break;
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

    m_WorldRoot.setFocusSmoothing({1.0f, 0.3f});
    m_WorldRoot.setTargetAnchorRatio({0.1f, 0.2f});
    m_WorldRoot.setVerticalDeadZoneRatio(0.2f);
    m_WorldRoot.focus(m_Player);
    updateVisibleRange();

    m_BackgroundRoot.setScreenSize(m_WorldRoot.getScreenSize());
    m_BackgroundRoot.setCellSize(m_WorldRoot.getCellSize());
    m_BackgroundRoot.setColor(Util::Color::FromRGB(58, 53, 254));

    // 單層背景
    m_BackgroundRoot.setImage(
    std::make_shared<Util::Image>(RESOURCE_DIR"/Image/Background/background.png"),
    0.10f
);
}

void GameplayScene::update(const float dt) {
    /// Debug mode !!!
    
    constexpr float moveSpeed = 60.0f;
    glm::vec2 position = m_Player->getPosition();

    if (Util::Input::IsKeyPressed(Util::Keycode::LEFT)) {
        position.x -= moveSpeed * dt;
    }
    if (Util::Input::IsKeyPressed(Util::Keycode::RIGHT)) {
        position.x += moveSpeed * dt;
    }
    if (Util::Input::IsKeyPressed(Util::Keycode::UP)) {
        position.y += moveSpeed * dt;
    }
    if (Util::Input::IsKeyPressed(Util::Keycode::DOWN)) {
        position.y -= moveSpeed * dt;
    }

    m_Player->setPosition(position);
    m_Player->setVelocity({0.0f, 0.0f});
    m_Player->setOnGround(false);
    
    // Debug mode !!!

    /*
    m_Player->update(dt);
    resolveSolidCollisions(dt);
    checkHazardCollisions();
    checkTriggerOverlaps();
    */

    m_WorldRoot.updateFocus(dt);
    updateVisibleRange();

    renderWorld();
}

std::vector<std::shared_ptr<World::WorldObject>> GameplayScene::getSolidCollisionCandidates(
    const World::AABB& playerSweptAabb
) const {
    std::vector<std::shared_ptr<World::WorldObject>> solidCollisionCandidates;

    solidCollisionCandidates.reserve(
        static_cast<std::size_t>(m_VisibleRange.y - m_VisibleRange.x)
    );

    // 目前先只用顯示範圍內的物件做候選，再用 swept AABB 粗篩一次。
    for (std::size_t objectIndex = static_cast<std::size_t>(m_VisibleRange.x);
         objectIndex < static_cast<std::size_t>(m_VisibleRange.y);
         ++objectIndex) {

        const auto& object = m_AllObjects[objectIndex];

        if (object->getType() != ObjectType::SOLID) {
            continue;
        }

        if (!World::Collision::aabbVsAabb(playerSweptAabb, object->getAABB())) {
            continue;
        }

        solidCollisionCandidates.push_back(object);
    }

    return solidCollisionCandidates;
}

// 使用swept AABB找出目前場景中可能發生碰撞的物件，並存起來。

// 每次移動 {0.1WorldPosition}，每次移動都檢測看看是否有撞到候選物件。
// 如果有撞到候選物件 就去判斷是什麼類型的碰撞（上、下或是側邊）
// 如何判斷類型碰撞：當兩者AABB的上下的重合深度 < 0.1 && 推出角度 > 45度：上下撞
//                不然就是：左右撞
                  
// 類型碰撞被拿去判斷是否死亡

// 如何推出（禁止左右推）：如果 0 < 推出角度 < 90 代表物體是斜的：可以藉由推出方向與推出深度，反推要上或下，推出多遠才能能剛好落地
//                     其他 代表物體是垂直或水平：這時直接使用AABB看上下嵌入深度，朝向y軸速度的反向推出嵌入深度

// 如何處理碰撞並矯正使用者位置後，矯正過的位置可能會有二次碰撞：可能需要拿新的位置做再次碰撞檢測
void GameplayScene::resolveSolidCollisions(const float dt) {
    constexpr float kCollisionStepLength = 0.1f;
    constexpr float kSeparationEpsilon = 0.0005f;

    const glm::vec2 startPosition = m_Player->getPreviousPosition();
    const glm::vec2 targetPosition = m_Player->getPosition();
    const glm::vec2 frameDelta = targetPosition - startPosition;

    // 先計算玩家起點 / 終點的 AABB，供 swept AABB 粗篩用。
    m_Player->setPosition(targetPosition);
    const World::AABB playerTargetAabb = m_Player->getAABB();
    m_Player->setPosition(startPosition);
    const World::AABB playerStartAabb = m_Player->getAABB();
    
    const World::AABB playerSweptAabb = mergeAabbs(playerStartAabb, playerTargetAabb);
    
    // 目前候選清單先從顯示範圍抓，再用 swept AABB 篩掉不可能碰撞的 solid。
    const std::vector<std::shared_ptr<World::WorldObject>> solidCollisionCandidates = getSolidCollisionCandidates(playerSweptAabb);

    // 每次矯正完後，新一輪的矯正迴圈從這裡開始

    glm::vec2 resolvedPosition = startPosition;
    glm::vec2 resolvedVelocity = m_Player->getVelocity();
    bool groundedThisFrame = false;

    const float movementDistance = glm::length(frameDelta);
    const int stepCount = std::max(1, static_cast<int>(std::ceil(movementDistance / kCollisionStepLength)));
    glm::vec2 stepDelta = frameDelta / static_cast<float>(stepCount);

    for (int stepIndex = 0; stepIndex < stepCount; ++stepIndex) {
        // 每次最多只前進 0.1 world position。
        resolvedPosition += stepDelta;
        m_Player->setPosition(resolvedPosition);

        for (const auto& object : solidCollisionCandidates) {
            // 第二層先做目前位置的 AABB 粗篩，再做精判。
            const World::AABB playerAabb = m_Player->getAABB();
            const World::AABB objectAabb = object->getAABB();
            if (!World::Collision::aabbVsAabb(playerAabb, objectAabb)) {
                continue;
            }

            // 第三層做精判。
            const auto collisionResult = World::Collision::geometryVsGeometry(
                m_Player->getCollisionGeometry(),
                object->getCollisionGeometry()
            );
            if (!collisionResult.hit) {
                continue;
            }

            // 取得碰撞方向
            const SolidHitType hitType = classifySolidHit(
                collisionResult,
                playerAabb,
                objectAabb,
                stepDelta
            );


            // 判斷此方向玩家會不會死亡
            if (shouldDieFromSolidHit(m_Player->getCharacterType(), hitType)) {
                m_Player->setPosition(m_LevelData.meta.playerSpawn);
                m_Player->setPreviousPosition(m_LevelData.meta.playerSpawn);
                m_Player->setVelocity({0.0f, 0.0f});
                m_Player->setOnGround(false);

                LOG_DEBUG("Player died from solid collision");
                return;
            }


            // ！！！
            // 側撞 / 頂頭是否死亡，由角色型態決定。
            if (shouldDieFromSolidHit(m_Player->getCharacterType(), hitType)) {
                m_Player->setPosition(m_LevelData.meta.playerSpawn);
                m_Player->setPreviousPosition(m_LevelData.meta.playerSpawn);
                m_Player->setVelocity({0.0f, 0.0f});
                m_Player->setOnGround(false);

                LOG_DEBUG("Player died from solid collision");
                return;
            }

            // 只允許修正 y，不修正 x，避免水平節奏被碰撞矯正打亂。
            const float slopeDegrees = calculateSlopeDegrees(collisionResult.normal);
            const glm::vec2 separation =
                -collisionResult.normal * (collisionResult.depth + kSeparationEpsilon);

            float pushY = 0.0f;

            if (slopeDegrees > 0.0f && slopeDegrees < 90.0f) {
                // 斜面：先沿分離方向取 y 分量，讓角色能順著斜面落回去。
                pushY = separation.y;
            }
            else {
                // 水平 / 垂直面：直接使用 AABB 的上下重疊深度做 y 推回。
                const float overlapToObjectTop = objectAabb.max.y - playerAabb.min.y;
                const float overlapToObjectBottom = playerAabb.max.y - objectAabb.min.y;

                if (hitType == SolidHitType::LANDING) {
                    pushY = overlapToObjectTop + kSeparationEpsilon;
                }
                else {
                    pushY = -(overlapToObjectBottom + kSeparationEpsilon);
                }
            }

            resolvedPosition.y += pushY;
            m_Player->setPosition(resolvedPosition);

            if (hitType == SolidHitType::LANDING) {
                groundedThisFrame = true;

                if (resolvedVelocity.y < 0.0f) {
                    resolvedVelocity.y = 0.0f;
                }

                // 已經落地後，本幀後續步驟不再繼續往下掉，
                // 但仍然保留 x 位移繼續前進。
                stepDelta.y = 0.0f;
            }
            else if (hitType == SolidHitType::CEILING) {
                if (resolvedVelocity.y > 0.0f) {
                    resolvedVelocity.y = 0.0f;
                }

                // 頂頭後，本幀後續步驟不再繼續往上推。
                stepDelta.y = 0.0f;
            }
            // ！！！


            // 碰撞形態判定
            // calculateVerticalOverlapDepth 的函數先直接寫在這裡就好
            // classifySolidHit 的函數先直接寫在這裡就好
            
            // 推出
            // 設定priviousPosition為矯正過的位置，然後再次開始往targetPosition前進，直到 x 相同。
            // 為什麼可以？由於每次矯正只會矯正 y 軸，所以 x 軸會繼續移動，直到 x 移到到 target


        }
    }
    m_Player->setPosition(resolvedPosition);
    m_Player->setVelocity(resolvedVelocity);
    m_Player->setOnGround(groundedThisFrame);
}


void GameplayScene::checkHazardCollisions() {
    const World::AABB playerAabb = m_Player->getAABB();
    const World::CollisionGeometry playerGeometry = m_Player->getCollisionGeometry();

    for (std::size_t i = static_cast<std::size_t>(m_VisibleRange.x);
         i < static_cast<std::size_t>(m_VisibleRange.y);
         ++i) {
        const auto& object = m_AllObjects[i];

        if (!object) {
            continue;
        }

        if (object->getType() != ObjectType::HAZARD) {
            continue;
        }

        if (!World::Collision::aabbVsAabb(playerAabb, object->getAABB())) {
            continue;
        }

        const auto result = World::Collision::geometryVsGeometry(
            playerGeometry,
            object->getCollisionGeometry()
        );

        if (!result.hit) {
            continue;
        }

        m_Player->setPosition(m_LevelData.meta.playerSpawn);
        m_Player->setPreviousPosition(m_LevelData.meta.playerSpawn);
        m_Player->setVelocity({0.0f, 0.0f});
        m_Player->setOnGround(false);

        LOG_DEBUG("Player hit hazard");
        return;
    }
}

void GameplayScene::checkTriggerOverlaps() {
    const World::AABB playerAabb = m_Player->getAABB();
    const World::CollisionGeometry playerGeometry = m_Player->getCollisionGeometry();

    for (std::size_t i = static_cast<std::size_t>(m_VisibleRange.x);
         i < static_cast<std::size_t>(m_VisibleRange.y);
         ++i) {
        const auto& object = m_AllObjects[i];

        if (!object) {
            continue;
        }

        if (object->getType() != ObjectType::TRIGGER) {
            continue;
        }

        if (!World::Collision::aabbVsAabb(playerAabb, object->getAABB())) {
            continue;
        }

        const auto result = World::Collision::geometryVsGeometry(
            playerGeometry,
            object->getCollisionGeometry()
        );

        if (!result.hit) {
            continue;
        }
        const auto triggerObject = std::dynamic_pointer_cast<TriggerObject>(object);
        if (!triggerObject) {
            continue;
        }

        if (triggerObject->getTriggerType() == TriggerType::PORTAL) {
            const auto portalObject = std::dynamic_pointer_cast<PortalObject>(object);
            if (!portalObject) {
                continue;
            }

            applyPlayerCharacterType(m_Player, portalObject->getTargetCharacterType());
            LOG_DEBUG("Player entered portal");
            return;
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