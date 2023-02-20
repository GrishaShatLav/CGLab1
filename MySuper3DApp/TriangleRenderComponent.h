#pragma once
#include "RenderComponent.h"

class RenderComponent;

class TriangleRenderComponent : public RenderComponent {
public:
	TriangleRenderComponent();
	TriangleRenderComponent(std::shared_ptr<DirectX::SimpleMath::Vector4> renderOffset);
};
