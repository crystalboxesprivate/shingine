#pragma once
#include "Modules/Statics/IActiveCamera.h"
#include "Modules/Statics/IComponentManager.h"
#include "Modules/Statics/IEntityManager.h"

#include "Engine/AssetTypes/Material.h"
#include "Modules/Graphics/GraphicsUtils.h"
#include "Modules/Statics/IAssetManager.h"

#include "Engine/Components/LightComponent.h"
#include "Engine/Components/RendererComponent.h"
#include "Engine/Components/SkyLightComponent.h"
#include "Engine/Components/TransformComponent.h"
#include "Game/FirstPersonController/FirstPersonComponent.h"

void AddFirstPersonController() {
  TransformComponent *transformComponent =
      Statics::Get<IActiveCamera>()->GetTransformComponent();
  // Add first person component
  FirstPersonController::FirstPersonComponent *comp =
      Statics::Get<IComponentManager>()
          ->AddComponent<FirstPersonController::FirstPersonComponent>(
              transformComponent->EntityId());

  comp->PlayerMovementSettings->RunMultiplier = 5.f;
  comp->PlayerMovementSettings->ForwardSpeed = 13.f;
  // Won't recalculate transform matrices without this set to 1
  transformComponent->IsDynamic = 1;
}

void TestStuff() {
  IComponentManager *componentManager = Statics::Get<IComponentManager>();

  // load texture
  ISerializedClass *tex;
  ResourceLoader::LoadBitmap("Assets/Textures/uv_checker.bmp", tex);
  // add default shader
  IShader *shader = GraphicsUtils::CreateVertexFragmentShader(
      "Assets/Shaders/SimpleLighting.vert",
      "Assets/Shaders/SimpleLighting.frag");
  // add material with the lighting
  Material *mat = Statics::Get<IAssetManager>()->AddAssetOfType<Material>();
  mat->Name = "TexturePreviewMaterial";
  mat->ShaderId = shader->AssetId();
  mat->SetTexture("_MainTex", tex->UniqueID());

  // set the same lighting material to the each object on the scene
  {
    ComponentMap<RendererComponent> *renderers =
        componentManager->GetComponentMap<RendererComponent>();

    for (unsigned int x = 0; x < renderers->Count(); x++) {
      RendererComponent *r = renderers->AtIndex(x);
      r->MaterialReference = mat->UniqueID();
    }
  }

  // add skylight component if there isn't
  SkyLightComponent *skyLight =
      componentManager->GetComponentOfType<SkyLightComponent>();
  if (!skyLight)
    Statics::Get<IEntityManager>()->CreateEntity(
        {"TransformComponent", "SkyLightComponent", "ObjectMetadataComponent"});

  {
    ComponentMap<LightComponent> *lightMap =
        componentManager->GetComponentMap<LightComponent>();
  
    bool directionalLight = false;

    for (unsigned int x = 0; x < lightMap->Count(); x++) {
      LightComponent *light = lightMap->AtIndex(x);
      directionalLight = light->LightType == 2;
      if (directionalLight)
        break;
    }

    if (!directionalLight) {
      unsigned int newEntity = Statics::Get<IEntityManager>()->CreateEntity(
          {"TransformComponent", "LightComponent", "ObjectMetadataComponent"});
      LightComponent *light =
          componentManager->GetComponentOfType<LightComponent>(newEntity);

      light->LightType = 2;
      light->ShadowEnabled = 1;

      TransformComponent *xform =
          componentManager->GetComponentOfType<TransformComponent>(newEntity);
      glm::quat rotation = glm::quat(glm::vec3(
          glm::radians(50.f), glm::radians(-30.f), glm::radians(0.f)));
      xform->SetRotation(rotation);
    }
  }
}
