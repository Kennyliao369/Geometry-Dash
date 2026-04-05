#ifndef COLLISION_HPP
#define COLLISION_HPP

#include "World/WorldObject.hpp"

#include <glm/vec2.hpp>

#include <vector>

namespace World::Collision {

struct CircleData {
    glm::vec2 center = {0.0f, 0.0f};
    float radius = 0.0f;
};

struct PolygonData {
    std::vector<glm::vec2> vertices;
};

struct CollisionResult {
    bool hit = false;

    // normal 定義為：從 A 指向 B 的方向
    glm::vec2 normal = {0.0f, 0.0f};

    // 最小穿透深度
    float depth = 0.0f;
};

glm::vec2 rotatePoint(const glm::vec2& point, float degrees);

PolygonData buildPolygon(const WorldObject& object);
CircleData buildCircle(const WorldObject& object);

CollisionResult polygonVsPolygon(
    const PolygonData& a,
    const PolygonData& b
);

CollisionResult circleVsCircle(
    const CircleData& a,
    const CircleData& b
);

CollisionResult circleVsPolygon(
    const CircleData& circle,
    const PolygonData& polygon
);

CollisionResult objectVsObject(
    const WorldObject& a,
    const WorldObject& b
);

} // namespace World::Collision

#endif