set(INCLUDE_FILES
    App.hpp

    World/Character.hpp
    World/WorldRender.hpp
    World/WorldObject.hpp
    World/ObjectType.hpp
    World/HazardObject.hpp
    World/DecorationObject.hpp
    World/SolidObject.hpp
    World/TriggerObject.hpp

    Level/LevelData.hpp
    Level/LevelLoader.hpp

    Scene/Scene.hpp
    Scene/SceneType.hpp
    Scene/MainMenuScene.hpp
    Scene/GameplayScene.hpp
)

set(SRC_FILES
    App.cpp

    World/Character.cpp
    World/WorldRender.cpp
    World/WorldObject.cpp

    Level/LevelLoader.cpp

    Scence/MainMenuScene.cpp
    Scence/GameplayScene.cpp
)

set(TEST_FILES
)
