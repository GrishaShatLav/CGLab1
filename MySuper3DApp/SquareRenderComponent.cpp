#include "SquareRenderComponent.h"

SquareRenderComponent::SquareRenderComponent() : RenderComponent() {
	indeces.insert(indeces.end(), { 0, 1, 2, 1, 0, 3 });
}

SquareRenderComponent::SquareRenderComponent(std::shared_ptr<DirectX::SimpleMath::Vector4> renderOffset) :
	RenderComponent(renderOffset) {
	indeces.insert(indeces.end(), { 0, 1, 2, 1, 0, 3 });
}
