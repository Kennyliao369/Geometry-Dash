#ifndef HAZARDOBJECT_HPP
#define HAZARDOBJECT_HPP

#include "World/WorldObject.hpp"

#include <glm/trigonometric.hpp>

enum class HazardType {
    SPIKE,
    ACID
};

class HazardObject : public World::WorldObject {
public:
    explicit HazardObject(HazardType hazardType)
        : World::WorldObject(ObjectType::HAZARD),
          m_HazardType(hazardType) {
    }

    ~HazardObject() override = default;

    HazardType getHazardType() const {
        return m_HazardType;
    }

private:
    HazardType m_HazardType;
};

class SpikeObject : public HazardObject {
public:
    SpikeObject()
        : HazardObject(HazardType::SPIKE) {
    }

    ~SpikeObject() override = default;

    World::CollisionGeometry getCollisionGeometry() const override {
        const glm::vec2 halfSize = getSize() * 0.5f;

        const std::vector<glm::vec2> localVertices = {
            {-halfSize.x, -halfSize.y},
            { halfSize.x, -halfSize.y},
            { 0.0f,        halfSize.y}
        };

        World::PolygonGeometry polygon;
        polygon.vertices.reserve(localVertices.size());

        const float radians = glm::radians(getRotation());
        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);

        for (const glm::vec2& localVertex : localVertices) {
            polygon.vertices.push_back(
                getPosition() + glm::vec2{
                    localVertex.x * cosine - localVertex.y * sine,
                    localVertex.x * sine   + localVertex.y * cosine
                }
            );
        }

        return polygon;
    }
};

class AcidObject : public HazardObject {
public:
    AcidObject()
        : HazardObject(HazardType::ACID) {
    }

    ~AcidObject() override = default;
};

#endif