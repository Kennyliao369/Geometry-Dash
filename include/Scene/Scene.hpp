#ifndef SCENE_HPP
#define SCENE_HPP

enum class SceneType {
    None,
    MainMenu,
    Gameplay,
    CharacterSelect,
    LevelSelect
};

class Scene {
public:
    Scene(SceneType type)
        : m_Type(type) {
    };

    virtual ~Scene() = default;

    SceneType getType() const {
        return m_Type;
    }

    virtual void update(const float dt) = 0;

private:
    SceneType m_Type = SceneType::None;

};

#endif