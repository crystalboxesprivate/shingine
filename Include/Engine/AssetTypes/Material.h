#pragma once
#include "IMaterial.h"
#include "Utility/Data/Serialization.h"
#include "Asset.h"

class Material : public Asset, public IMaterial, public IObject {
public:
  SERIALIZE_CLASS(Material)
  Material();
  virtual ~Material(){};

  ATTRIBUTE_VALUE(String, Name)
  ATTRIBUTE_ID(ShaderId)

  ATTRIBUTE_VECTOR(String, FloatUniformNames)
  ATTRIBUTE_VECTOR(float, FloatUniformValues)

  ATTRIBUTE_VECTOR(String, VectorUniformNames)
  ATTRIBUTE_VECTOR(float, VectorUniformValues)

  ATTRIBUTE_VECTOR(String, TextureUniformNames)
  ATTRIBUTE_ID_VECTOR(TextureUniformValues)
  ATTRIBUTE_VECTOR(String, ExternalTexturePaths)

  virtual void GetFloatUniforms(std::vector<std::string> &names,
                                std::vector<float> &values);
  virtual void GetVectorUniforms(std::vector<std::string> &names,
                                 std::vector<glm::vec4> &values);
  virtual void GetTextureUniforms(std::vector<std::string> &names,
                                  std::vector<unsigned int> &values);

  virtual void SetFloat(const String &name, float value);
  virtual void SetVector(const String &name, const glm::vec4 &value);
  virtual void SetTexture(const String &name, unsigned int textureId);
};
