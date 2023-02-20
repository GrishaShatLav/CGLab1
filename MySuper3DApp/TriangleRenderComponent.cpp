#include "TriangleRenderComponent.h"

TriangleRenderComponent::TriangleRenderComponent() : RenderComponent() {
	indeces.insert(indeces.end(), { 0, 1, 2 });
}

TriangleRenderComponent::TriangleRenderComponent(std::shared_ptr<DirectX::SimpleMath::Vector4> renderOffset) :
	RenderComponent(renderOffset) {
	indeces.insert(indeces.end(), { 0, 1, 2 });
}
