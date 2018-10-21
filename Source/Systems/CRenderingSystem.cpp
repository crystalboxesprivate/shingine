#include "Systems/CRenderingSystem.h"
#include "Engine/AssetTypes/Settings/CRenderSettings.h"
#include "Modules/Graphics/COpenGLRenderer.h"


#include "Modules/Statics/CAssetManager.h"
#include "Modules/Statics/CComponentManager.h"
#include "Modules/Statics/CEntityManager.h"


void CRenderingSystem::Initialize()
{
    // get render settings
    CRenderSettings* renderSettings = dynamic_cast<CRenderSettings*> (CAssetManager::Get()->GetAssetOfType("RenderSettings"));
    if (!renderSettings)
    {
        // default render settings;
        renderSettings = new CRenderSettings();
        CAssetManager::Get()->AddInstance(renderSettings);
    }

    if (!Renderer)
    {
        Renderer = new COpenGLRender();
        Renderer->Create(renderSettings->ScreenWidth, renderSettings->ScreenHeight, renderSettings->WindowTitle);
    }
}

void CRenderingSystem::Update()
{
    CComponentManager::StringMap::iterator rendererIterator;
    CComponentManager::StringMap::iterator transformIterator;

    CComponentManager* componentManager = CComponentManager::Get();
    // iterate over renderer iterator
    componentManager->GetComponentIteratorOfType("Renderer", rendererIterator);
    componentManager->GetComponentIteratorOfType("Transform", transformIterator);
    // get camera component
    IComponent* cameraComponent = componentManager->GetComponentOfType("Camera");
    if (!cameraComponent)
    {
        unsigned int newId = 
            CEntityManager::Get()->CreateEntity({"Transform", "Camera", "ObjectMetadata"});
        cameraComponent = componentManager->GetComponentOfType("Camera", newId);
    }

    std::unordered_map<unsigned int, IComponent*>::iterator entityIterator;
    for (entityIterator = rendererIterator->second.begin(); entityIterator != rendererIterator->second.end(); entityIterator++)
    {
        unsigned int entityId = entityIterator->first;
        IComponent* transform = transformIterator->second.at(entityId);

    }
}

CRenderingSystem::~CRenderingSystem()
{
    delete Renderer;
}

// the rendering backend
// checks shader first
// compiles if not backs to default
// find mesh bind to this material id
// renders it with the matrix and shader uniforms collected from material
