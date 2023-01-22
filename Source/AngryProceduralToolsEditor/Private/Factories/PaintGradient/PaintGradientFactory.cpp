// Copyright Epic Games, Inc. All Rights Reserved.

#include "Factories/PaintGradient/PaintGradientFactory.h"
#include "Textures/PaintGradient.h"

#define LOCTEXT_NAMESPACE "PaintGradientFactory"

UPaintGradientFactory::UPaintGradientFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UPaintGradient::StaticClass();
}

FText UPaintGradientFactory::GetDisplayName() const
{
	return LOCTEXT("PaintGradientFactoryDescription", "Paint Gradient Texture");
}

bool UPaintGradientFactory::ConfigureProperties()
{
	return true;
}

bool UPaintGradientFactory::CanCreateNew() const
{
	return true;
}

UObject* UPaintGradientFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UPaintGradient* PaintGradient = CastChecked<UPaintGradient>(CreateOrOverwriteAsset(UPaintGradient::StaticClass(), InParent, Name, Flags));

	if (PaintGradient == nullptr)
	{
		UE_LOG(LogTexture, Warning, TEXT("PaintGradient creation failed.\n"));
	}

	return PaintGradient;
}

#undef LOCTEXT_NAMESPACE
