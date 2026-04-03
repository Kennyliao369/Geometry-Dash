#include "World/WorldRender.hpp"

#include <queue>
#include <algorithm>

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
}

void WorldRender::updateFocusPosition() {
    if (auto target = m_FocusTarget.lock()) {
        m_FocusPosition = target->getPosition();
    }
}

void WorldRender::clearTarget() {
    m_FocusTarget.reset();
}

glm::vec2 WorldRender::getFocusPosition() {
    return m_FocusPosition;
}

void WorldRender::projectObjectToScreen(StackInfo& worldObject) {
    const glm::vec2 screenPosition = (worldObject.m_WorldObject->getPosition() - m_FocusPosition ) * m_CellSize;
    worldObject.m_WorldObject->setImageTranslation(screenPosition);

    const glm::vec2 imageSize = worldObject.m_WorldObject->getSize() * m_CellSize;
    worldObject.m_WorldObject->setImageSize(imageSize);
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

void WorldRender::update() {
    updateFocusPosition();

    auto compareFunction = [](const StackInfo &a, const StackInfo &b) {
        return a.m_WorldObject->GetZIndex() > b.m_WorldObject->GetZIndex();
    };
    std::priority_queue<StackInfo, std::vector<StackInfo>,
                        decltype(compareFunction)>
        renderQueue(compareFunction);
    
    while (!m_ObjectsStack.empty()) {
        auto curr = m_ObjectsStack.back();
        m_ObjectsStack.pop_back();
        renderQueue.push(curr);

        for (const auto &child : curr.m_WorldObject->GetChildren()) {
            m_ObjectsStack.push_back(
                StackInfo{child, curr.m_WorldObject->getImageTransform()});
        }
    }

    // draw all in render queue by order
    while (!renderQueue.empty()) {
        auto curr = renderQueue.top();
        renderQueue.pop();

        projectObjectToScreen(curr);
        curr.m_WorldObject->Draw();
    }
}
}
