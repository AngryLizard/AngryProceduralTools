

#include "Factories/ExteriorGradient/ExteriorGradientThumbnailRenderer.h"
#include "Textures/ExteriorGradient.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"

//////////////////////////////////////////////////////////////////////////
// UPaperSpriteThumbnailRenderer

UExteriorGradientThumbnailRenderer::UExteriorGradientThumbnailRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UExteriorGradientThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UExteriorGradient* ExteriorGradient = Cast<UExteriorGradient>(Object);

	if (IsValid(ExteriorGradient) && ExteriorGradient->GetResource() != nullptr)
	{
		// Use A canvas tile item to draw
		FCanvasTileItem CanvasTile(FVector2D(X, Y), ExteriorGradient->GetResource(), FVector2D(Width, Height), FLinearColor::White);
		CanvasTile.BlendMode = SE_BLEND_Opaque;
		CanvasTile.Draw(Canvas);
	}
}
