#pragma once
#include "ComponentSetup.h"

#define POINT_LIGHT_TYPE 0
#define SPOT_LIGHT_TYPE 1
#define DIRECTIONAL_LIGHT_TYPE 2

class LightComponent : public Component
{
public:
    SERIALIZE_CLASS(LightComponent)
    
    LightComponent();
    virtual ~LightComponent();

    ATTRIBUTE_VECTOR(float, Color)
    ATTRIBUTE_VALUE(float, Exposure)
    ATTRIBUTE_VALUE(float, Intensity)

    // 0 point 1 spot 2 directional
    ATTRIBUTE_VALUE(unsigned char, LightType)
    // attenuation
    ATTRIBUTE_VALUE(float, Constant)
    ATTRIBUTE_VALUE(float, Linear)
    ATTRIBUTE_VALUE(float, Quadratic)

    ATTRIBUTE_VALUE(float, CutOff)

    ATTRIBUTE_VALUE(unsigned char, ShadowEnabled)
};