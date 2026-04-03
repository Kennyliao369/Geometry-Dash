#ifndef DECORATIONOBJECT_HPP
#define DECORATIONOBJECT_HPP

#include "World/WorldObject.hpp"

enum class DecorationType {
    BEACON,
};

class DecorationObject : public World::WorldObject {
public:
    DecorationObject(DecorationType decorationType) 
        :   World::WorldObject(ObjectType::DECORATION),
            m_DecorationType(decorationType) {
        
    }


    virtual ~DecorationObject() = default;

private:
    DecorationType m_DecorationType;

};

#endif