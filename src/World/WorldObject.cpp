#include "Util/Transform.hpp"
#include "Util/TransformUtils.hpp"

#include "World/WorldObject.hpp"

#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

#include <cmath>

namespace World {

void WorldObject::Draw(const Util::Transform& renderTransform) {
    if (!m_Visible || m_Drawable == nullptr) {
        return;
    }

    auto data = Util::ConvertToUniformBufferData(
        renderTransform, m_Drawable->GetSize(), m_ZIndex);

    data.m_Model = glm::translate(
        data.m_Model,
        glm::vec3{m_Pivot / m_Drawable->GetSize(), 0.0f} * -1.0f
    );

    m_Drawable->Draw(data);
}


CollisionGeometry WorldObject::getCollisionGeometry() const {
    const glm::vec2 halfSize = m_WorldSize * 0.5f;
    const std::vector<glm::vec2> localVertices = {
        {-halfSize.x, -halfSize.y},
        { halfSize.x, -halfSize.y},
        { halfSize.x,  halfSize.y},
        {-halfSize.x,  halfSize.y}
    };

    PolygonGeometry polygon;
    polygon.vertices.reserve(localVertices.size());

    const float radians = glm::radians(m_Rotation);
    const float cosine = std::cos(radians);
    const float sine = std::sin(radians);
    for (const glm::vec2& localVertex : localVertices) {
        polygon.vertices.push_back(
            m_WorldPosition + glm::vec2{
                localVertex.x * cosine - localVertex.y * sine,
                localVertex.x * sine   + localVertex.y * cosine
            }
        );
    }

    return polygon;
}

AABB WorldObject::getAABB() const {
    const CollisionGeometry geometry = getCollisionGeometry();

    return std::visit(
        [](const auto& shape) -> AABB {
            using ShapeType = std::decay_t<decltype(shape)>;

            if constexpr (std::is_same_v<ShapeType, PolygonGeometry>) {
                glm::vec2 minPoint = shape.vertices[0];
                glm::vec2 maxPoint = shape.vertices[0];

                for (const glm::vec2& vertex : shape.vertices) {
                    minPoint.x = std::min(minPoint.x, vertex.x);
                    minPoint.y = std::min(minPoint.y, vertex.y);
                    maxPoint.x = std::max(maxPoint.x, vertex.x);
                    maxPoint.y = std::max(maxPoint.y, vertex.y);
                }

                return {minPoint, maxPoint};
            }
            else {
                return {
                    shape.center - glm::vec2(shape.radius, shape.radius),
                    shape.center + glm::vec2(shape.radius, shape.radius)
                };
            }
        },
        geometry
    );
}

} // namespace World
