#pragma once
#include <unordered_map>
#include <string>
#include "Solver.h"

class String;
class ISystem;

class Solver_Impl
{
public:
    typedef std::unordered_map<std::string, ISystem*> SystemsMap;
    Solver_Impl();
    void AddSystem(const String& systemTypeName);
    void SetActive(bool active, const String& systemTypeName);
    void InitializeSystems();
    bool Simulate();
    bool IsValid();
private:
    bool Valid = true;
    SystemsMap Systems;
};