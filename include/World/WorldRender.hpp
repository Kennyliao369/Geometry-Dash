#ifndef WORLDRENDER_HPP
#define WORLDRENDER_HPP

#include <algorithm>
#include <memory>
#include <vector>

#include <glm/vec2.hpp>

#include "Util/Transform.hpp"
#include "World/WorldObject.hpp"

namespace World {

class WorldRender {
public:
    WorldRender() = default;
    WorldRender(const glm::vec2& screenSize, const glm::vec2& visibleWorldSize);

    void addObject(const std::shared_ptr<WorldObject>& worldObject);
    void addObjects(const std::vector<std::shared_ptr<WorldObject>>& worldObjects);

    void addObjects(
        std::vector<std::shared_ptr<WorldObject>>::const_iterator begin,
        std::vector<std::shared_ptr<WorldObject>>::const_iterator end
    );

    void clear();

    void focus(glm::vec2 position);
    void focus(const std::shared_ptr<WorldObject> target);
    void clearTarget();

    glm::vec2 getFocusPosition();

    void setScreenSize(const glm::vec2& screenSize);
    void setVisibleWorldSize(const glm::vec2& visibleWorldSize);
    void setViewConfig(const glm::vec2& screenSize, const glm::vec2& visibleWorldSize);

    glm::vec2 getScreenSize() const;
    glm::vec2 getVisibleWorldSize() const;
    float getCellSize() const;

    void update();

private:
    struct StackInfo {
        std::shared_ptr<WorldObject> m_WorldObject;
        Util::Transform m_ParentTransform;
    };

    void updateFocusPosition();
    Util::Transform projectObjectToScreen(const StackInfo& stackInfo) const;
    void updateCellSize();

private:
    glm::vec2 m_ScreenSize = {1280.0f, 720.0f};
    glm::vec2 m_VisibleWorldSize = {16.0f, 12.0f};
    float m_CellSize = 64.0f;

    std::vector<StackInfo> m_ObjectsStack;

    std::weak_ptr<WorldObject> m_FocusTarget;
    glm::vec2 m_FocusPosition = {0.0f, 0.0f};
};

}

#endif