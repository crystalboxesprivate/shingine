#pragma once
#include <unordered_map>
#include <typeinfo>
#include "Utility/Data/Serialization.h"
#include "IComponentManager.h"

class IComponent;

class ComponentManager : public IComponentManager, public ISerializedClass
{
public:
    typedef std::unordered_map<unsigned int, IComponent*> IdMap;
    typedef std::unordered_map<std::string, IdMap> StringMap;

    SERIALIZE_CLASS(ComponentManager)
    ComponentManager();
    virtual ~ComponentManager() {}

    virtual void AddComponent(IComponent* component);
    virtual IComponent* AddComponent(String type, unsigned int entityId = 0);
    virtual void UpdateComponentEntityId(IComponent* component);
    virtual IComponent* GetComponentOfType(String typeName, unsigned int componentId = 0);
    
    virtual void GetAllComponents(std::vector<IComponent*> &components);
    virtual bool GetComponentIteratorOfType(String typeName, StringMap::iterator &iterator);
private:
    StringMap Components;
};
