#ifndef WORLDOBJECT_HPP
#define WORLDOBJECT_HPP

#include "Core/Drawable.hpp"

#include "Util/Transform.hpp"
#include "Util/Image.hpp"

#include "pch.hpp" // IWYU pragma: export
#include "World/CollisionGeometry.hpp"

#include <algorithm>
#include <memory>
#include <variant>
#include <vector>

enum class ObjectType {
    CHARACTER,
    SOLID,
    HAZARD,
    TRIGGER,
    DECORATION,
    UNKNOWN
};

namespace World {

class WorldObject {
public:
    WorldObject() = default;
    explicit WorldObject(ObjectType type) : m_Type(type) {}

    WorldObject(const WorldObject &other) = default;
    WorldObject(WorldObject &&other) = default;
    virtual ~WorldObject() = default;

    WorldObject &operator=(const WorldObject &other) = delete;

    ObjectType getType() const {
        return m_Type;
    }

    float GetZIndex() const {
        return m_ZIndex;
    }

    const std::vector<std::shared_ptr<WorldObject>>& GetChildren() const {
        return m_Children;
    }

    void SetPivot(const glm::vec2 &pivot) {
        m_Pivot = pivot;
    }

    void SetZIndex(float index) {
        m_ZIndex = index;
    }

    void SetDrawable(const std::shared_ptr<Core::Drawable>& drawable) {
        m_Drawable = drawable;
    }

    void SetVisible(bool visible) {
        m_Visible = visible;
    }

    void setPosition(glm::vec2 position) {
        m_WorldPosition = position;
    }

    void setSize(glm::vec2 size) {
        m_WorldSize = size;
    }

    void setRotation(float rotationDegrees) {
        m_Rotation = rotationDegrees;
    }

    void AddChild(const std::shared_ptr<WorldObject>& child) {
        m_Children.push_back(child);
    }

    void RemoveChild(const std::shared_ptr<WorldObject>& child) {
        m_Children.erase(
            std::remove(m_Children.begin(), m_Children.end(), child),
            m_Children.end());
    }

    glm::vec2 getPosition() const {
        return m_WorldPosition;
    }

    glm::vec2 getSize() const {
        return m_WorldSize;
    }

    float getRotation() const {
        return m_Rotation;
    }

    glm::vec2 getDrawableSize() const {
        if (!m_Drawable) {
            return {1.0f, 1.0f};
        }
        return m_Drawable->GetSize();
    }

    virtual CollisionGeometry getCollisionGeometry() const;
    virtual AABB getAABB() const;

    void Draw(const Util::Transform& renderTransform);

protected:
    ObjectType m_Type = ObjectType::UNKNOWN;
    std::shared_ptr<Core::Drawable> m_Drawable = nullptr;
    
    glm::vec2 m_WorldPosition = {0.0f, 0.0f};
    glm::vec2 m_WorldSize = {0.0f, 0.0f};
    float m_Rotation = 0.0f;
    
    float m_ZIndex = 0.0f;
    bool m_Visible = true;
    glm::vec2 m_Pivot = {0.0f, 0.0f};
    std::vector<std::shared_ptr<WorldObject>> m_Children;
};

} // namespace World

#endif
