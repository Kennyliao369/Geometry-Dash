#include "World/WorldRender.hpp"

#include <queue>
#include <algorithm>
#include <cmath>

namespace {
    float toFrameIndependentFactor(const float factorAt60Fps, const float dt) {
        const float factor = std::clamp(factorAt60Fps, 0.0f, 1.0f);

        if (factor >= 1.0f) {
            return 1.0f;
        }

        return 1.0f - std::pow(1.0f - factor, dt * 60.0f);
    }
}

namespace World {

WorldRender::WorldRender(const glm::vec2& screenSize, const glm::vec2& visibleWorldSize)
    : m_ScreenSize(screenSize),
      m_VisibleWorldSize(visibleWorldSize) {
    updateCellSize();
}


void WorldRender::addObject(const std::shared_ptr<World::WorldObject>& worldObject) {
    m_ObjectsStack.push_back(StackInfo{worldObject, Util::Transform{}});
}

void WorldRender::addObjects(const std::vector<std::shared_ptr<World::WorldObject>>& worldObjects) {
    for (const auto& worldObject : worldObjects) {
        m_ObjectsStack.push_back(StackInfo{worldObject, Util::Transform{}});
    }
}
void WorldRender::addObjects(
        std::vector<std::shared_ptr<WorldObject>>::const_iterator begin,
        std::vector<std::shared_ptr<WorldObject>>::const_iterator end
    ) {
    for (auto it = begin; it != end; ++it) {
        m_ObjectsStack.push_back(StackInfo{*it, Util::Transform{}});
    }
}
void WorldRender::clear() {
    m_ObjectsStack.clear();
}

// void WorldRender::removeObjects(std::shared_ptr<WorldObject> worldObject) {}

void WorldRender::focus(glm::vec2 position) {
    m_FocusTarget.reset();
    m_FocusPosition = position;
}

void WorldRender::focus(const std::shared_ptr<WorldObject> target) {
    m_FocusTarget = target;

    if (target) {
        m_FocusPosition = target->getPosition() + getTargetTrackingOffset();
    }
}

void WorldRender::updateFocusPosition(const float dt) {
    auto target = m_FocusTarget.lock();
    if (!target) {
        return;
    }

    const glm::vec2 targetPosition = target->getPosition();
    const glm::vec2 targetTrackingPosition =
        targetPosition + getTargetTrackingOffset();
    glm::vec2 desiredFocusPosition = m_FocusPosition;

    // X 軸：直接跟玩家
    desiredFocusPosition.x = targetTrackingPosition.x;

    // Y 軸：dead zone
    const float deadZoneHalfHeight =
        m_VisibleWorldSize.y * m_VerticalDeadZoneRatio;

    const float deltaY = targetPosition.y - m_FocusPosition.y;

    if (deltaY > deadZoneHalfHeight) {
        desiredFocusPosition.y = targetPosition.y - deadZoneHalfHeight;
    }
    else if (deltaY < -deadZoneHalfHeight) {
        desiredFocusPosition.y = targetPosition.y + deadZoneHalfHeight;
    }
    // 若在 dead zone 內，就保持 desiredFocusPosition.y 不變

    const float followFactorX =
        toFrameIndependentFactor(m_FocusSmoothing.x, dt);
    const float followFactorY =
        toFrameIndependentFactor(m_FocusSmoothing.y, dt);

    m_FocusPosition.x +=
        (desiredFocusPosition.x - m_FocusPosition.x) * followFactorX;
    m_FocusPosition.y +=
        (desiredFocusPosition.y - m_FocusPosition.y) * followFactorY;
}

void WorldRender::setFocusSmoothing(const glm::vec2& smoothing) {
    m_FocusSmoothing.x = std::clamp(smoothing.x, 0.0f, 1.0f);
    m_FocusSmoothing.y = std::clamp(smoothing.y, 0.0f, 1.0f);
}

void WorldRender::setVerticalDeadZoneRatio(const float ratio) {
    m_VerticalDeadZoneRatio = std::clamp(ratio, 0.0f, 0.5f);
}

void WorldRender::updateFocus(float dt) {
    updateFocusPosition(dt);
}

void WorldRender::clearTarget() {
    m_FocusTarget.reset();
}

glm::vec2 WorldRender::getFocusPosition() {
    return m_FocusPosition;
}
Util::Transform WorldRender::projectObjectToScreen(const StackInfo& stackInfo) const {
    const auto& object = stackInfo.m_WorldObject;

    Util::Transform localTransform{};
    localTransform.translation =
        (object->getPosition() - m_FocusPosition) * m_CellSize;

    const glm::vec2 desiredImageSize = object->getSize() * m_CellSize;
    const glm::vec2 drawableSize = object->getDrawableSize();

    localTransform.scale = {
        drawableSize.x != 0.0f ? desiredImageSize.x / drawableSize.x : 1.0f,
        drawableSize.y != 0.0f ? desiredImageSize.y / drawableSize.y : 1.0f
    };

    localTransform.rotation = glm::radians(object->getRotation());

    Util::Transform finalTransform = localTransform;

    // 先做一個 MVP 版合成：
    finalTransform.translation += stackInfo.m_ParentTransform.translation;
    finalTransform.rotation += stackInfo.m_ParentTransform.rotation;
    finalTransform.scale *= stackInfo.m_ParentTransform.scale;

    return finalTransform;
}


void WorldRender::setScreenSize(const glm::vec2& screenSize) {
    m_ScreenSize = screenSize;
    updateCellSize();
}

void WorldRender::setVisibleWorldSize(const glm::vec2& visibleWorldSize) {
    m_VisibleWorldSize = visibleWorldSize;
    updateCellSize();
}

void WorldRender::setViewConfig(const glm::vec2& screenSize, const glm::vec2& visibleWorldSize) {
    m_ScreenSize = screenSize;
    m_VisibleWorldSize = visibleWorldSize;
    updateCellSize();
}

glm::vec2 WorldRender::getScreenSize() const {
    return m_ScreenSize;
}

glm::vec2 WorldRender::getVisibleWorldSize() const {
    return m_VisibleWorldSize;
}

float WorldRender::getCellSize() const {
    return m_CellSize;
}

void WorldRender::updateCellSize() {
    if (m_VisibleWorldSize.x <= 0.0f || m_VisibleWorldSize.y <= 0.0f) {
        m_CellSize = 1.0f;
        return;
    }

    const float cellSizeX = m_ScreenSize.x / m_VisibleWorldSize.x;
    const float cellSizeY = m_ScreenSize.y / m_VisibleWorldSize.y;

    m_CellSize = std::min(cellSizeX, cellSizeY);
}

void WorldRender::setTargetAnchorRatio(const glm::vec2& anchorRatio) {
    m_TargetAnchorRatio.x = std::clamp(anchorRatio.x, 0.0f, 1.0f);
    m_TargetAnchorRatio.y = std::clamp(anchorRatio.y, 0.0f, 1.0f);
}

glm::vec2 WorldRender::getTargetAnchorRatio() const {
    return m_TargetAnchorRatio;
}

glm::vec2 WorldRender::getTargetTrackingOffset() const {
    return {
        (0.5f - m_TargetAnchorRatio.x) * m_VisibleWorldSize.x,
        (0.5f - m_TargetAnchorRatio.y) * m_VisibleWorldSize.y
    };
}

void WorldRender::update() {
    auto compareFunction = [](const StackInfo& a, const StackInfo& b) {
        return a.m_WorldObject->GetZIndex() > b.m_WorldObject->GetZIndex();
    };

    std::priority_queue<StackInfo, std::vector<StackInfo>, decltype(compareFunction)>
        renderQueue(compareFunction);

    while (!m_ObjectsStack.empty()) {
        auto curr = m_ObjectsStack.back();
        m_ObjectsStack.pop_back();

        renderQueue.push(curr);

        const Util::Transform currRenderTransform = projectObjectToScreen(curr);

        for (const auto& child : curr.m_WorldObject->GetChildren()) {
            m_ObjectsStack.push_back(StackInfo{child, currRenderTransform});
        }
    }

    while (!renderQueue.empty()) {
        auto curr = renderQueue.top();
        renderQueue.pop();

        const Util::Transform renderTransform = projectObjectToScreen(curr);
        curr.m_WorldObject->Draw(renderTransform);
    }
}
}
