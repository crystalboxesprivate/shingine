#include "Modules/Graphics/OpenGL/OglShaderManager.h"
#include "Utility/Graphics.h"

#include "Modules/Statics/IGraphics.h"

#include "Modules/Graphics/IShader.h"
#include "Modules/Statics/AssetManager.h"

#include "Engine/AssetTypes/IMaterial.h"

#include <glm/gtc/type_ptr.hpp>
#include "Core.h"
OglShaderManager::OglShaderManager() {}

void OglShaderManager::GetUniformId(const std::string &uniformName,
                                    int programId, int &uniformLoc) {
  uniformLoc = 0;
  std::unordered_map<std::string, int>::iterator it =
      UniformLocations[programId].find(uniformName);
  if (it == UniformLocations[programId].end()) {
    // find uniform location
    glUseProgram(programId);
    uniformLoc = glGetUniformLocation(programId, uniformName.c_str());
    UniformLocations[programId][uniformName] = uniformLoc;
    S_LOG_FUNC("Getting uniform location for %s", uniformName.c_str());
  } else
    uniformLoc = it->second;
}

int OglShaderManager::GetShaderProgramId(unsigned int shaderId) {
  // if the asset id is 0 then assign the default shader id
  if (shaderId == 0)
    shaderId = Statics::Get<IGraphics>()->DefaultShader()->AssetId();

  std::unordered_map<unsigned int, int>::iterator it =
      AssetIdToShaderProgramId.find(shaderId);
  if (it != AssetIdToShaderProgramId.end())
    return it->second;

  // compile shader source or raise the error
  IShader *shader = dynamic_cast<IShader *>(
      Statics::Get<IAssetManager>()->GetAssetOfType("Shader", shaderId));

  if (shader == nullptr)
    throw 1;

  int shaderProgramId = CreateVertexFragmentShaderProgram(shader);

  AssetIdToShaderProgramId[shaderId] = shaderProgramId;
  return shaderProgramId;
}

int OglShaderManager::CreateVertexFragmentShaderProgram(IShader *shader) {
  String vertexShaderSource, fragmentShaderSource;
  // couldn't find the vertex and fragment shader sources
  if (!shader->GetSource(EShaderType::VERTEX, vertexShaderSource))
    throw 1;
  if (!shader->GetSource(EShaderType::FRAGMENT, fragmentShaderSource))
    throw 1;

  int vertexId = CompileShaderSource(EShaderType::VERTEX, vertexShaderSource);
  int fragmentId =
      CompileShaderSource(EShaderType::FRAGMENT, fragmentShaderSource);

  int programId = glCreateProgram();

  glAttachShader(programId, vertexId);
  glAttachShader(programId, fragmentId);
  glLinkProgram(programId);

  String errorText;
  if (!DidProgramLink(programId, errorText)) {
    std::cout << "Program link error: " << errorText << std::endl;
    throw 1;
  }
  return programId;
}

int OglShaderManager::CompileShaderSource(int shaderType,
                                          const String &shaderSource) {
  int shaderTypeId = 0;
  std::string shaderTypeName = "";
  switch (EShaderType(shaderType)) {
  case EShaderType::VERTEX:
    shaderTypeId = GL_VERTEX_SHADER;
    shaderTypeName = "GL_VERTEX_SHADER";
    break;
  case EShaderType::FRAGMENT:
    shaderTypeId = GL_FRAGMENT_SHADER;
    shaderTypeName = "GL_FRAGMENT_SHADER";
    break;
  default:
    break;
  }

  int shaderId = glCreateShader(shaderTypeId);
  // compile shader source
  std::vector<String> multilineSource = shaderSource.Split('\n');
  unsigned int lineCount = static_cast<unsigned int>(multilineSource.size());
  char **arraySource = new char *[lineCount];
  memset(arraySource, 0, lineCount);

  for (size_t x = 0; x < lineCount; x++) {
    unsigned int charCount = multilineSource[x].Length();
    arraySource[x] = new char[charCount + 2];
    memset(arraySource[x], 0, charCount + 2);

    for (unsigned int y = 0; y < charCount; y++)
      arraySource[x][y] = multilineSource[x][y];

    arraySource[x][charCount] = '\n';
    arraySource[x][charCount + 1] = '\0';
  }

  glShaderSource(shaderId, lineCount, arraySource, nullptr);
  glCompileShader(shaderId);

  for (size_t x = 0; x < lineCount; x++) {
    delete[] arraySource[x];
  }
  delete[] arraySource;

  String errorText;
  if (!DidShaderCompile(shaderId, errorText)) {
    std::cout << "Shader compilation error " << shaderTypeName << " : "
              << errorText << std::endl;
    throw 1;
  }

  return shaderId;
}

bool OglShaderManager::DidShaderCompile(unsigned int shaderId,
                                        String &errorText) {
  errorText = "";
  GLint success = 0;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);
    char *logText = new char[maxLength];
    glGetShaderInfoLog(shaderId, maxLength, &maxLength, logText);
    errorText += logText;
    delete[] logText;
    return false;
  }
  return true;
}

bool OglShaderManager::DidProgramLink(unsigned int programId,
                                      String &errorText) {
  errorText = "";
  GLint success = 0;
  glGetProgramiv(programId, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &maxLength);
    char *logText = new char[maxLength];
    glGetProgramInfoLog(programId, maxLength, &maxLength, logText);
    errorText += logText;
    delete[] logText;
    return false;
  }
  return true;
}
