#pragma once
#include <iostream>
#include "../Common.h"

class ISerialized
{
public:
    virtual ~ISerialized() {};
    virtual CString SerializedName() = 0;
    virtual CString TypeName() = 0;
};

class ISerializedClass : public ISerialized
{
public:
    virtual ~ISerializedClass() {};
    virtual unsigned int UniqueID() = 0;
    virtual void SetUniqueID(unsigned int newId) = 0;
    virtual CString TypeName() { return "SerializedClass"; };
    virtual void SetAttribute(ISerialized* &attr) = 0;
    virtual void SetAttribute(const CString& serializedName, ISerialized* &attr) = 0;
    virtual void GetAttribute(ISerialized* &attr) = 0;
    virtual void GetAllAttributes(std::vector<ISerialized*> &attributes) = 0;
};
