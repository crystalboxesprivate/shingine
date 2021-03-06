#include <iostream>
#include <map>
#include <string>
#include <typeinfo>

class ISerialized;

#define __REGISTER_SERIALIZED_TYPE(TYPENAME)                                   \
  virtual String SerializedName();                                             \
  static SerializedRegistry<TYPENAME> RegisterClassConstructor;                \
                                                                               \
  unsigned int SerializedUniqueID;                                             \
  bool SerializedIDSet = false;                                                \
  virtual unsigned int UniqueID() { return SerializedUniqueID; }               \
  virtual void SetUniqueID(unsigned int newId) {                               \
    SerializedUniqueID = newId;                                                \
    SerializedIDSet = true;                                                    \
  }

#define REGISTER_SERIALIZED_CLASS(TYPENAME)                                    \
  String TYPENAME::SerializedName() { return #TYPENAME; }                      \
  SerializedRegistry<TYPENAME> TYPENAME::RegisterClassConstructor(             \
      #TYPENAME, typeid(TYPENAME).name());

#define SERIALIZE_CLASS(CLASSNAME)                                             \
  typedef void (CLASSNAME::*MFP)(ISerialized * &attr);                         \
  std::map<std::string, MFP> AttributeFunctionMapGet;                          \
  std::map<std::string, MFP> AttributeFunctionMapSet;                          \
  std::vector<std::string> AttributeNames;                                     \
  virtual void SetAttribute(ISerialized *&attr) {                              \
    SetAttribute(attr->SerializedName(), attr);                                \
  };                                                                           \
  virtual void SetAttribute(const String &serializedName,                      \
                            ISerialized *&attr) {                              \
    if (AttributeFunctionMapSet.find(serializedName) ==                        \
        AttributeFunctionMapSet.end())                                         \
      return;                                                                  \
    MFP fp = AttributeFunctionMapSet[serializedName];                          \
    (this->*fp)(attr);                                                         \
  };                                                                           \
  virtual void GetAttribute(ISerialized *&attr) {                              \
    if (AttributeFunctionMapGet.find(attr->SerializedName()) ==                \
        AttributeFunctionMapGet.end())                                         \
      return;                                                                  \
    MFP fp = AttributeFunctionMapGet[attr->SerializedName()];                  \
    (this->*fp)(attr);                                                         \
  };                                                                           \
  virtual void GetAllAttributes(std::vector<ISerialized *> &attributes) {      \
    for (unsigned int x = 0; x < AttributeNames.size(); x++) {                 \
      ISerialized *newAttr;                                                    \
      MFP fp = AttributeFunctionMapGet[AttributeNames[x]];                     \
      (this->*fp)(newAttr);                                                    \
      attributes.push_back(newAttr);                                           \
    }                                                                          \
  };                                                                           \
  __REGISTER_SERIALIZED_TYPE(CLASSNAME)

#define COMPONENT_CLASS(CLASSNAME)                                             \
  SERIALIZE_CLASS(CLASSNAME)                                                   \
  static SerializedRegistry<ComponentMap<CLASSNAME>>                           \
      RegisterComponentMapConstructor;

#define REGISTER_COMPONENT(CLASSNAME)                                          \
  REGISTER_SERIALIZED_CLASS(CLASSNAME)                                         \
  SerializedRegistry<ComponentMap<CLASSNAME>>                                  \
      CLASSNAME::RegisterComponentMapConstructor(                              \
          "ComponentMap<" #CLASSNAME ">",                                      \
          typeid(ComponentMap<CLASSNAME>).name());

#define ATTRIBUTE_ID(NAME)                                                     \
  unsigned int NAME = 0;                                                       \
  void Attrib_Set_##NAME(ISerialized *&attr) {                                 \
    NAME = ((TypedAttributeValue<unsigned int> *)attr)->Get();                 \
  }                                                                            \
  void Attrib_Get_##NAME(ISerialized *&attr) {                                 \
    attr = new TypedAttributeValue<unsigned int>(#NAME, "uid", NAME);          \
  }

#define ATTRIBUTE_ID_VECTOR(NAME)                                              \
  std::vector<unsigned int> NAME;                                              \
  void Attrib_Set_##NAME(ISerialized *&attr) {                                 \
    NAME = ((TypedAttribute<unsigned int> *)attr)->Get();                      \
  }                                                                            \
  void Attrib_Get_##NAME(ISerialized *&attr) {                                 \
    attr = new TypedAttribute<unsigned int>(#NAME, "uid", NAME);               \
  }

#define ATTRIBUTE_VALUE(TYPE_NAME, NAME)                                       \
  TYPE_NAME NAME;                                                              \
  void Attrib_Set_##NAME(ISerialized *&attr) {                                 \
    NAME = ((TypedAttributeValue<TYPE_NAME> *)attr)->Get();                    \
  }                                                                            \
  void Attrib_Get_##NAME(ISerialized *&attr) {                                 \
    attr = new TypedAttributeValue<TYPE_NAME>(#NAME, #TYPE_NAME, NAME);        \
  }

#define ATTRIBUTE_VECTOR(TYPE_NAME, NAME)                                      \
  std::vector<TYPE_NAME> NAME;                                                 \
  void Attrib_Set_##NAME(ISerialized *&attr) {                                 \
    TypedAttribute<TYPE_NAME> *typedAttr = (TypedAttribute<TYPE_NAME> *)attr;  \
    if (!typedAttr)                                                            \
      return;                                                                  \
    NAME = typedAttr->Get();                                                   \
  }                                                                            \
  void Attrib_Get_##NAME(ISerialized *&attr) {                                 \
    attr = new TypedAttribute<TYPE_NAME>(#NAME, #TYPE_NAME, NAME);             \
  }

#define ___GET_ATTR_VEC_BOY(NAME)                                              \
  void Attrib_Set_##NAME(ISerialized *&attr) {                                 \
    TypedAttribute<float> *typedAttr = (TypedAttribute<float> *)attr;          \
    if (!typedAttr)                                                            \
      return;                                                                  \
    std::vector<float> raw = typedAttr->Get();

#define ATTRIBUTE_GLM_VEC3_ARRAY(NAME)                                         \
  std::vector<glm::vec3> NAME;                                                 \
  ___GET_ATTR_VEC_BOY(NAME)                                                    \
  NAME = std::vector<glm::vec3>();                                             \
  float vectorSize = raw.size() / 3;                                           \
  for (unsigned int x = 0; x < vectorSize; x++) {                              \
    unsigned int index = x * 3;                                                \
    NAME.push_back(glm::vec3(raw[index + 0], raw[index + 1], raw[index + 2])); \
  }                                                                            \
  }                                                                            \
  void Attrib_Get_##NAME(ISerialized *&attr) {                                 \
    std::vector<float> outVec;                                                 \
    for (unsigned int x = 0; x < NAME.size(); x++) {                           \
      outVec.push_back(NAME[x].x);                                             \
      outVec.push_back(NAME[x].y);                                             \
      outVec.push_back(NAME[x].z);                                             \
    }                                                                          \
    attr = new TypedAttribute<float>(#NAME, "float", outVec);                  \
  }

#define ATTRIBUTE_GLM_VEC3(NAME)                                               \
  glm::vec3 NAME;                                                              \
  ___GET_ATTR_VEC_BOY(NAME)                                                    \
  NAME = glm::vec3(raw[0], raw[1], raw[2]);                                    \
  }                                                                            \
  void Attrib_Get_##NAME(ISerialized *&attr) {                                 \
    std::vector<float> packedAttr = {NAME.x, NAME.y, NAME.z};                  \
    attr = new TypedAttribute<float>(#NAME, "float", packedAttr);              \
  }

#define ATTRIBUTE_GLM_QUAT(NAME)                                               \
  glm::quat NAME;                                                              \
  ___GET_ATTR_VEC_BOY(NAME)                                                    \
  NAME = glm::quat();                                                          \
  NAME.x = raw[0];                                                             \
  NAME.y = raw[1];                                                             \
  NAME.z = raw[2];                                                             \
  NAME.w = raw[3];                                                             \
  }                                                                            \
  void Attrib_Get_##NAME(ISerialized *&attr) {                                 \
    std::vector<float> packedAttr = {NAME.x, NAME.y, NAME.z, NAME.w};          \
    attr = new TypedAttribute<float>(#NAME, "float", packedAttr);              \
  }

#define ATTRIBUTE_CLASS(CLASSNAME, NAME)                                       \
  CLASSNAME *NAME = nullptr;                                                   \
  void Attrib_Set_##NAME(ISerialized *&attr) {                                 \
    NAME = dynamic_cast<CLASSNAME *>(attr);                                    \
  }                                                                            \
  void Attrib_Get_##NAME(ISerialized *&attr) {                                 \
    if (NAME == nullptr) {                                                     \
      attr = SerializedFactory::CreateInstance(#CLASSNAME);                    \
      NAME = dynamic_cast<CLASSNAME *>(attr);                                  \
      return;                                                                  \
    }                                                                          \
    attr = NAME;                                                               \
  }
// create an empty IDataNode
// create C
#define ATTRIBUTE_CLASS_VECTOR(CLASSNAME, NAME)                                \
  std::vector<CLASSNAME *> NAME;                                               \
  void Attrib_Set_##NAME(ISerialized *&attr) {                                 \
    CAttributeClassVector *typedAttr =                                         \
        dynamic_cast<CAttributeClassVector *>(attr);                           \
    if (!typedAttr)                                                            \
      return;                                                                  \
    std::vector<ISerialized *> data = typedAttr->Get();                        \
    NAME.clear();                                                              \
    for (size_t x = 0; x < data.size(); x++) {                                 \
      CLASSNAME *myObj = dynamic_cast<CLASSNAME *>(data[x]);                   \
      if (!myObj)                                                              \
        continue;                                                              \
      NAME.push_back(myObj);                                                   \
    }                                                                          \
  }                                                                            \
  void Attrib_Get_##NAME(ISerialized *&attr) {                                 \
    std::vector<ISerialized *> data;                                           \
    for (size_t x = 0; x < NAME.size(); x++) {                                 \
      ISerialized *myObj = dynamic_cast<ISerialized *>(NAME[x]);               \
      data.push_back(myObj);                                                   \
    }                                                                          \
    attr = new CAttributeClassVector(#NAME, data);                             \
  }

#define ATTRIBUTE_REGISTER(CLASSNAME, NAME)                                    \
  AttributeNames.push_back(#NAME);                                             \
  AttributeFunctionMapGet.insert(                                              \
      std::make_pair(#NAME, &CLASSNAME::Attrib_Get_##NAME));                   \
  AttributeFunctionMapSet.insert(                                              \
      std::make_pair(#NAME, &CLASSNAME::Attrib_Set_##NAME));
