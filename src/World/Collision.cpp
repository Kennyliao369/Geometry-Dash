#include "World/Collision.hpp"

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace World::Collision {

namespace {
    constexpr float EPSILON = 0.0001f;

    struct Projection {
        float min = 0.0f;
        float max = 0.0f;
    };

    glm::vec2 safeNormalize(const glm::vec2& v) {
        const float length = glm::length(v);
        if (length <= EPSILON) {
            return {0.0f, 0.0f};
        }
        return v / length;
    }

    glm::vec2 perpendicular(const glm::vec2& v) {
        return {-v.y, v.x};
    }

    glm::vec2 polygonCenter(const PolygonData& polygon) {
        glm::vec2 sum{0.0f, 0.0f};

        for (const auto& vertex : polygon.vertices) {
            sum += vertex;
        }

        if (polygon.vertices.empty()) {
            return sum;
        }

        return sum / static_cast<float>(polygon.vertices.size());
    }

    Projection projectPolygon(
        const PolygonData& polygon,
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
        const CircleData& circle,
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
        const PolygonData& polygon,
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
        const PolygonData& polygon,
        const glm::vec2& point
    ) {
        float bestDistance2 = std::numeric_limits<float>::max();
        glm::vec2 bestVertex{0.0f, 0.0f};

        for (const auto& vertex : polygon.vertices) {
            const float distance2 = glm::dot(vertex - point, vertex - point);
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
        const PolygonData& a,
        const PolygonData& b
    ) {
        CollisionResult result;
        result.hit = true;
        result.depth = std::numeric_limits<float>::max();

        std::vector<glm::vec2> axes;
        appendPolygonAxes(a, axes);
        appendPolygonAxes(b, axes);

        for (const auto& axis : axes) {
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
        const CircleData& circle,
        const PolygonData& polygon
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

        for (const auto& axis : axes) {
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

} // namespace

glm::vec2 rotatePoint(const glm::vec2& point, const float degrees) {
    const float radians = glm::radians(degrees);
    const float c = std::cos(radians);
    const float s = std::sin(radians);

    return {
        point.x * c - point.y * s,
        point.x * s + point.y * c
    };
}

PolygonData buildPolygon(const WorldObject& object) {
    const glm::vec2 center = object.getPosition();
    const glm::vec2 halfSize = object.getSize() * 0.5f;

    std::vector<glm::vec2> localVertices;

    switch (object.getShapeType()) {
    case ShapeType::BOX:
        localVertices = {
            {-halfSize.x, -halfSize.y},
            { halfSize.x, -halfSize.y},
            { halfSize.x,  halfSize.y},
            {-halfSize.x,  halfSize.y}
        };
        break;

    case ShapeType::TRIANGLE:
        localVertices = {
            {-halfSize.x, -halfSize.y},
            { halfSize.x, -halfSize.y},
            {0.0f,         halfSize.y}
        };
        break;

    case ShapeType::CIRCLE:
        return {};
    }

    PolygonData polygon;
    polygon.vertices.reserve(localVertices.size());

    for (const auto& localVertex : localVertices) {
        polygon.vertices.push_back(
            center + rotatePoint(localVertex, object.getRotation())
        );
    }

    return polygon;
}

CircleData buildCircle(const WorldObject& object) {
    CircleData circle;
    circle.center = object.getPosition();
    circle.radius = std::min(object.getSize().x, object.getSize().y) * 0.5f;
    return circle;
}

CollisionResult polygonVsPolygon(
    const PolygonData& a,
    const PolygonData& b
) {
    if (a.vertices.size() < 3 || b.vertices.size() < 3) {
        return {};
    }

    return satPolygonVsPolygon(a, b);
}

CollisionResult circleVsCircle(
    const CircleData& a,
    const CircleData& b
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
    } else {
        result.normal = {1.0f, 0.0f};
    }

    return result;
}

CollisionResult circleVsPolygon(
    const CircleData& circle,
    const PolygonData& polygon
) {
    if (polygon.vertices.size() < 3) {
        return {};
    }

    return satCircleVsPolygon(circle, polygon);
}

CollisionResult objectVsObject(
    const WorldObject& a,
    const WorldObject& b
) {
    const ShapeType shapeA = a.getShapeType();
    const ShapeType shapeB = b.getShapeType();

    if (shapeA == ShapeType::CIRCLE && shapeB == ShapeType::CIRCLE) {
        return circleVsCircle(buildCircle(a), buildCircle(b));
    }

    if (shapeA == ShapeType::CIRCLE && shapeB != ShapeType::CIRCLE) {
        return circleVsPolygon(buildCircle(a), buildPolygon(b));
    }

    if (shapeA != ShapeType::CIRCLE && shapeB == ShapeType::CIRCLE) {
        CollisionResult result = circleVsPolygon(buildCircle(b), buildPolygon(a));
        if (result.hit) {
            result.normal = -result.normal;
        }
        return result;
    }

    return polygonVsPolygon(buildPolygon(a), buildPolygon(b));
}

} // namespace World::Collision