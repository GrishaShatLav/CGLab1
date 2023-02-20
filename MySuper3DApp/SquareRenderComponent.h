#pragma once
#include "RenderComponent.h"

class SquareRenderComponent : public RenderComponent {
public:
	SquareRenderComponent();
	SquareRenderComponent(std::shared_ptr<DirectX::SimpleMath::Vector4> renderOffset);
};
