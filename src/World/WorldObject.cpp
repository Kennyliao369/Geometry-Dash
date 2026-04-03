#include "Util/Transform.hpp"
#include "Util/TransformUtils.hpp"

#include "World/WorldObject.hpp"

namespace World {

void WorldObject::Draw() {
    if (!m_Visible || m_Drawable == nullptr) {
        return;
    }

    auto data = Util::ConvertToUniformBufferData(
        m_ImageTransform, m_Drawable->GetSize(), m_ZIndex);
    data.m_Model = glm::translate(
        data.m_Model, glm::vec3{m_Pivot / m_Drawable->GetSize(), 0} * -1.0F);

    m_Drawable->Draw(data);
}

void WorldObject::setImageSize(glm::vec2 newSize) {
    if (!m_Drawable) { return; }
    const glm::vec2 oldSize = m_Drawable->GetSize();
    glm::vec2 scale   = newSize / oldSize;
    m_ImageTransform.scale = scale;
}
}
