#ifndef TRIGGEROBJECT_HPP
#define TRIGGEROBJECT_HPP

#include "World/WorldObject.hpp"

enum class TriggerType {
    PORTAL,
    PAD,
    COIN
};

class TriggerObject : public World::WorldObject {
public:
    TriggerObject(TriggerType triggerType) 
        :   World::WorldObject(ObjectType::TRIGGER),
            m_TriggerType(triggerType) {
        

    }

    virtual ~TriggerObject() = default;

private:
    TriggerType m_TriggerType;

};

#endif