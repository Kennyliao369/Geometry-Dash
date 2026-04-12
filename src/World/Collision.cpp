#include "World/Collision.hpp"

#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace World::Collision {

namespace {
    constexpr float EPSILON = 0.0001f;

    struct Projection {
        float min = 0.0f;
        float max = 0.0f;
    };

    glm::vec2 safeNormalize(const glm::vec2& vector) {
        const float length = glm::length(vector);
        if (length <= EPSILON) {
            return {0.0f, 0.0f};
        }
        return vector / length;
    }

    glm::vec2 perpendicular(const glm::vec2& vector) {
        return {-vector.y, vector.x};
    }

    glm::vec2 polygonCenter(const World::PolygonGeometry& polygon) {
        glm::vec2 center{0.0f, 0.0f};

        for (const glm::vec2& vertex : polygon.vertices) {
            center += vertex;
        }

        if (polygon.vertices.empty()) {
            return center;
        }

        return center / static_cast<float>(polygon.vertices.size());
    }

    Projection projectPolygon(
        const World::PolygonGeometry& polygon,
        const glm::vec2& axis
    ) {
        Projection projection;
        projection.min = glm::dot(polygon.vertices[0], axis);
        projection.max = projection.min;

        for (std::size_t i = 1; i < polygon.vertices.size(); ++i) {
            const float value = glm::dot(polygon.vertices[i], axis);
            projection.min = std::min(projection.min, value);
            projection.max = std::max(projection.max, value);
        }

        return projection;
    }

    Projection projectCircle(
        const World::CircleGeometry& circle,
        const glm::vec2& axis
    ) {
        const float centerProjection = glm::dot(circle.center, axis);
        return {
            centerProjection - circle.radius,
            centerProjection + circle.radius
        };
    }

    bool overlapOnAxis(
        const Projection& a,
        const Projection& b,
        float& overlap
    ) {
        overlap = std::min(a.max, b.max) - std::max(a.min, b.min);
        return overlap > 0.0f;
    }

    void appendPolygonAxes(
        const World::PolygonGeometry& polygon,
        std::vector<glm::vec2>& axes
    ) {
        const std::size_t count = polygon.vertices.size();

        for (std::size_t i = 0; i < count; ++i) {
            const glm::vec2& current = polygon.vertices[i];
            const glm::vec2& next = polygon.vertices[(i + 1) % count];
            const glm::vec2 edge = next - current;
            const glm::vec2 axis = safeNormalize(perpendicular(edge));

            if (glm::length(axis) > EPSILON) {
                axes.push_back(axis);
            }
        }
    }

    glm::vec2 closestPolygonVertex(
        const World::PolygonGeometry& polygon,
        const glm::vec2& point
    ) {
        float bestDistance2 = std::numeric_limits<float>::max();
        glm::vec2 bestVertex{0.0f, 0.0f};

        for (const glm::vec2& vertex : polygon.vertices) {
            const glm::vec2 delta = vertex - point;
            const float distance2 = glm::dot(delta, delta);

            if (distance2 < bestDistance2) {
                bestDistance2 = distance2;
                bestVertex = vertex;
            }
        }

        return bestVertex;
    }

    void orientNormalFromAToB(
        CollisionResult& result,
        const glm::vec2& centerA,
        const glm::vec2& centerB
    ) {
        const glm::vec2 direction = centerB - centerA;

        if (glm::dot(result.normal, direction) < 0.0f) {
            result.normal = -result.normal;
        }
    }

    CollisionResult satPolygonVsPolygon(
        const World::PolygonGeometry& a,
        const World::PolygonGeometry& b
    ) {
        CollisionResult result;
        result.hit = true;
        result.depth = std::numeric_limits<float>::max();

        std::vector<glm::vec2> axes;
        appendPolygonAxes(a, axes);
        appendPolygonAxes(b, axes);

        for (const glm::vec2& axis : axes) {
            const Projection projectionA = projectPolygon(a, axis);
            const Projection projectionB = projectPolygon(b, axis);

            float overlap = 0.0f;
            if (!overlapOnAxis(projectionA, projectionB, overlap)) {
                return {};
            }

            if (overlap < result.depth) {
                result.depth = overlap;
                result.normal = axis;
            }
        }

        orientNormalFromAToB(result, polygonCenter(a), polygonCenter(b));
        return result;
    }

    CollisionResult satCircleVsPolygon(
        const World::CircleGeometry& circle,
        const World::PolygonGeometry& polygon
    ) {
        CollisionResult result;
        result.hit = true;
        result.depth = std::numeric_limits<float>::max();

        std::vector<glm::vec2> axes;
        appendPolygonAxes(polygon, axes);

        const glm::vec2 closestVertex = closestPolygonVertex(polygon, circle.center);
        const glm::vec2 vertexAxis = safeNormalize(closestVertex - circle.center);
        if (glm::length(vertexAxis) > EPSILON) {
            axes.push_back(vertexAxis);
        }

        for (const glm::vec2& axis : axes) {
            const Projection circleProjection = projectCircle(circle, axis);
            const Projection polygonProjection = projectPolygon(polygon, axis);

            float overlap = 0.0f;
            if (!overlapOnAxis(circleProjection, polygonProjection, overlap)) {
                return {};
            }

            if (overlap < result.depth) {
                result.depth = overlap;
                result.normal = axis;
            }
        }

        orientNormalFromAToB(result, circle.center, polygonCenter(polygon));
        return result;
    }
}

bool aabbVsAabb(
    const World::AABB& a,
    const World::AABB& b
) {
    return a.min.x <= b.max.x && a.max.x >= b.min.x &&
           a.min.y <= b.max.y && a.max.y >= b.min.y;
}

CollisionResult polygonVsPolygon(
    const World::PolygonGeometry& a,
    const World::PolygonGeometry& b
) {
    if (a.vertices.size() < 3 || b.vertices.size() < 3) {
        return {};
    }

    return satPolygonVsPolygon(a, b);
}

CollisionResult circleVsCircle(
    const World::CircleGeometry& a,
    const World::CircleGeometry& b
) {
    CollisionResult result;

    const glm::vec2 delta = b.center - a.center;
    const float distance = glm::length(delta);
    const float radiusSum = a.radius + b.radius;

    if (distance >= radiusSum) {
        return result;
    }

    result.hit = true;
    result.depth = radiusSum - distance;

    if (distance > EPSILON) {
        result.normal = delta / distance;
    }
    else {
        result.normal = {1.0f, 0.0f};
    }

    return result;
}

CollisionResult circleVsPolygon(
    const World::CircleGeometry& circle,
    const World::PolygonGeometry& polygon
) {
    if (polygon.vertices.size() < 3) {
        return {};
    }

    return satCircleVsPolygon(circle, polygon);
}

CollisionResult geometryVsGeometry(
    const World::CollisionGeometry& a,
    const World::CollisionGeometry& b
) {
    if (std::holds_alternative<World::CircleGeometry>(a) &&
        std::holds_alternative<World::CircleGeometry>(b)) {
        return circleVsCircle(
            std::get<World::CircleGeometry>(a),
            std::get<World::CircleGeometry>(b)
        );
    }

    if (std::holds_alternative<World::CircleGeometry>(a) &&
        std::holds_alternative<World::PolygonGeometry>(b)) {
        return circleVsPolygon(
            std::get<World::CircleGeometry>(a),
            std::get<World::PolygonGeometry>(b)
        );
    }

    if (std::holds_alternative<World::PolygonGeometry>(a) &&
        std::holds_alternative<World::CircleGeometry>(b)) {
        CollisionResult result = circleVsPolygon(
            std::get<World::CircleGeometry>(b),
            std::get<World::PolygonGeometry>(a)
        );

        if (result.hit) {
            result.normal = -result.normal;
        }

        return result;
    }

    return polygonVsPolygon(
        std::get<World::PolygonGeometry>(a),
        std::get<World::PolygonGeometry>(b)
    );
}

} // namespace World::Collision
