// Copyright Epic Games, Inc. All Rights Reserved.

#include "Factories/PaintGradient/PaintGradientThumbnailRenderer.h"
#include "Textures/PaintGradient.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"

//////////////////////////////////////////////////////////////////////////
// UPaperSpriteThumbnailRenderer

UPaintGradientThumbnailRenderer::UPaintGradientThumbnailRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPaintGradientThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	UPaintGradient* PaintGradient = Cast<UPaintGradient>(Object);

	if (IsValid(PaintGradient) && PaintGradient->GetResource() != nullptr)
	{
		// Use A canvas tile item to draw
		FCanvasTileItem CanvasTile(FVector2D(X, Y), PaintGradient->GetResource(), FVector2D(Width, Height), FLinearColor::White);
		CanvasTile.BlendMode = SE_BLEND_Opaque;
		CanvasTile.Draw(Canvas);
	}
}
