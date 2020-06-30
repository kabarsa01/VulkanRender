#pragma once

#include "scene/SceneObjectComponent.h"

#include <glm/ext/vector_float3.hpp>

enum LightType
{
	LT_Directional,
	LT_Spot,
	LT_Point,
	LT_MAX
};

class LightComponent : public SceneObjectComponent
{
public:
	LightType type;
	glm::vec3 color;
	float intensity;
	float radius;
	float spotHalfAngle;

	LightComponent(std::shared_ptr<SceneObjectBase> InParent);
	virtual ~LightComponent();
};

typedef std::shared_ptr<LightComponent> LightComponentPtr;

