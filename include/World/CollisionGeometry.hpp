#ifndef WORLD_COLLISIONGEOMETRY_HPP
#define WORLD_COLLISIONGEOMETRY_HPP

#include <glm/vec2.hpp>

#include <variant>
#include <vector>

namespace World {

struct AABB {
    glm::vec2 min = {0.0f, 0.0f};
    glm::vec2 max = {0.0f, 0.0f};
};

struct CircleGeometry {
    glm::vec2 center = {0.0f, 0.0f};
    float radius = 0.0f;
};

struct PolygonGeometry {
    std::vector<glm::vec2> vertices;
};

using CollisionGeometry = std::variant<PolygonGeometry, CircleGeometry>;

} // namespace World

#endif