#include "Util/Transform.hpp"
#include "Util/TransformUtils.hpp"

#include "World/WorldObject.hpp"

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
}
