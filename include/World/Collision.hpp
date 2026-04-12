#ifndef COLLISION_HPP
#define COLLISION_HPP

#include "World/CollisionGeometry.hpp"

#include <glm/vec2.hpp>

namespace World::Collision {

struct CollisionResult {
    bool hit = false;
    glm::vec2 normal = {0.0f, 0.0f};
    float depth = 0.0f;
};

bool aabbVsAabb(
    const World::AABB& a,
    const World::AABB& b
);

CollisionResult polygonVsPolygon(
    const World::PolygonGeometry& a,
    const World::PolygonGeometry& b
);

CollisionResult circleVsCircle(
    const World::CircleGeometry& a,
    const World::CircleGeometry& b
);

CollisionResult circleVsPolygon(
    const World::CircleGeometry& circle,
    const World::PolygonGeometry& polygon
);

CollisionResult geometryVsGeometry(
    const World::CollisionGeometry& a,
    const World::CollisionGeometry& b
);

} // namespace World::Collision

#endif
