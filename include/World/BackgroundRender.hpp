#ifndef BACKGROUNDRENDER_HPP
#define BACKGROUNDRENDER_HPP

#include <memory>
#include <vector>

#include <glm/vec2.hpp>

#include "Util/Color.hpp"
#include "Util/Image.hpp"

namespace World {

class BackgroundRender {
public:
    BackgroundRender() = default;

    void setScreenSize(const glm::vec2& screenSize);
    void setCellSize(float cellSize);

    void setColor(const Util::Color& color);

    void setImage(const std::shared_ptr<Util::Image>& image, float parallaxFactor = 0.0f);
    void addImage(const std::shared_ptr<Util::Image>& image, float parallaxFactor);

    void clearImages();

    void update(const glm::vec2& focusPosition);

private:
    struct Layer {
        std::shared_ptr<Util::Image> image;
        float parallaxFactor = 0.0f;
    };

    static float normalizeColorComponent(float value);
    static float positiveModulo(float value, float divisor);

    void sortLayers();
    void drawBackgroundColor() const;
    void drawLayer(const Layer& layer, std::size_t layerIndex, const glm::vec2& focusPosition) const;

private:
    glm::vec2 m_ScreenSize = {1280.0f, 720.0f};
    float m_CellSize = 64.0f;

    Util::Color m_BackgroundColor = Util::Color::FromRGB(20, 20, 28);

    std::vector<Layer> m_Layers;
};

} // namespace World

#endif