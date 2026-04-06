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

    void updateFocus(float dt);

    void setFocusSmoothing(const glm::vec2& smoothing);
    void setVerticalDeadZoneRatio(float ratio);

    void setTargetAnchorRatio(const glm::vec2& anchorRatio);
    glm::vec2 getTargetAnchorRatio() const;

    void update();

private:
    struct StackInfo {
        std::shared_ptr<WorldObject> m_WorldObject;
        Util::Transform m_ParentTransform;
    };

    void updateFocusPosition();
    Util::Transform projectObjectToScreen(const StackInfo& stackInfo) const;
    void updateCellSize();
    void updateFocusPosition(float dt);

    glm::vec2 getTargetTrackingOffset() const;
    
private:
    glm::vec2 m_ScreenSize = {1280.0f, 720.0f};
    glm::vec2 m_VisibleWorldSize = {16.0f, 12.0f};
    float m_CellSize = 64.0f;

    std::vector<StackInfo> m_ObjectsStack;

    std::weak_ptr<WorldObject> m_FocusTarget;
    glm::vec2 m_FocusPosition = {100.0f, 100.0f};

    glm::vec2 m_FocusSmoothing = {1.0f, 0.3f};
    // x = 1.0f 代表幾乎直接跟
    // y = 0.3f 代表平滑追焦

    float m_VerticalDeadZoneRatio = 0.2f;
    // 注意：這裡是「中心上下各 20%」
    // 如果你想要總共只有 20%，就改成 0.1f


    glm::vec2 m_TargetAnchorRatio = {0.2f, 0.5f};
};

}

#endif