#include "World/BackgroundRender.hpp"

#include "Util/Transform.hpp"
#include "Util/TransformUtils.hpp"

#include <algorithm>
#include <cmath>

namespace World {

void BackgroundRender::setScreenSize(const glm::vec2& screenSize) {
    m_ScreenSize = screenSize;
}

void BackgroundRender::setCellSize(const float cellSize) {
    m_CellSize = cellSize;
}

void BackgroundRender::setColor(const Util::Color& color) {
    m_BackgroundColor = color;
}

void BackgroundRender::setImage(
    const std::shared_ptr<Util::Image>& image,
    const float parallaxFactor
) {
    m_Layers.clear();
    addImage(image, parallaxFactor);
}

void BackgroundRender::addImage(
    const std::shared_ptr<Util::Image>& image,
    const float parallaxFactor
) {
    if (!image) {
        return;
    }

    m_Layers.push_back(Layer{
        .image = image,
        .parallaxFactor = parallaxFactor
    });

    sortLayers();
}

void BackgroundRender::clearImages() {
    m_Layers.clear();
}

float BackgroundRender::normalizeColorComponent(const float value) {
    return std::clamp(value, 0.0f, 255.0f) / 255.0f;
}

float BackgroundRender::positiveModulo(const float value, const float divisor) {
    const float mod = std::fmod(value, divisor);
    return mod < 0.0f ? mod + divisor : mod;
}

void BackgroundRender::sortLayers() {
    // parallaxFactor 越小看起來越遠，先畫在後面
    std::stable_sort(
        m_Layers.begin(),
        m_Layers.end(),
        [](const Layer& a, const Layer& b) {
            return a.parallaxFactor < b.parallaxFactor;
        }
    );
}

void BackgroundRender::drawBackgroundColor() const {
    glClearColor(
        normalizeColorComponent(m_BackgroundColor.r),
        normalizeColorComponent(m_BackgroundColor.g),
        normalizeColorComponent(m_BackgroundColor.b),
        normalizeColorComponent(m_BackgroundColor.a)
    );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void BackgroundRender::drawLayer(
    const Layer& layer,
    const std::size_t layerIndex,
    const glm::vec2& focusPosition
) const {
    if (!layer.image) {
        return;
    }

    const glm::vec2 tileSize = layer.image->GetSize();
    if (tileSize.x <= 0.0f || tileSize.y <= 0.0f) {
        return;
    }

    // focusPosition 是 world unit，先轉成像素偏移，再乘上 parallax
    const glm::vec2 scrollPixels =
        focusPosition * m_CellSize * layer.parallaxFactor;

    const float offsetX = positiveModulo(scrollPixels.x, tileSize.x);
    const float offsetY = positiveModulo(scrollPixels.y, tileSize.y);

    const float halfScreenW = m_ScreenSize.x * 0.5f;
    const float halfScreenH = m_ScreenSize.y * 0.5f;

    // 多補一圈，避免畫面邊緣留空
    const float startLeft   = -halfScreenW - offsetX - tileSize.x;
    const float startBottom = -halfScreenH - offsetY - tileSize.y;
    const float endRight    =  halfScreenW + tileSize.x;
    const float endTop      =  halfScreenH + tileSize.y;

    const float zIndex = -10.0f + static_cast<float>(layerIndex) * 0.01f;

    for (float left = startLeft; left < endRight; left += tileSize.x) {
        for (float bottom = startBottom; bottom < endTop; bottom += tileSize.y) {
            Util::Transform renderTransform{};
            renderTransform.translation = {
                left + tileSize.x * 0.5f,
                bottom + tileSize.y * 0.5f
            };
            renderTransform.scale = {1.0f, 1.0f};
            renderTransform.rotation = 0.0f;

            auto data = Util::ConvertToUniformBufferData(
                renderTransform,
                tileSize,
                zIndex
            );

            layer.image->Draw(data);
        }
    }
}

void BackgroundRender::update(const glm::vec2& focusPosition) {
    drawBackgroundColor();

    for (std::size_t i = 0; i < m_Layers.size(); ++i) {
        drawLayer(m_Layers[i], i, focusPosition);
    }
}

} // namespace World