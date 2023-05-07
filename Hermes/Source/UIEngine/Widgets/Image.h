#pragma once

#include "Core/Core.h"
#include "RenderingEngine/Texture.h"
#include "UIEngine/Widgets/Widget.h"

namespace Hermes::UI
{
	class HERMES_API Image : public Widget
	{
	public:
		static std::shared_ptr<Image> Create(AssetHandle<Texture2D> InTexture);

		const Texture2D* GetTexture()  const;
		void SetTexture(AssetHandle<Texture2D> NewTexture);

	private:
		AssetHandle<Texture2D> Texture;

		explicit Image(AssetHandle<Texture2D> InTexture);

		virtual Vec2 ComputeMinimumSize() const override;

		virtual void Draw(DrawingContext& Context) const override;
	};
}