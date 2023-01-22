// Copyright Epic Games, Inc. All Rights Reserved.

#include "Factories/ExteriorGradient/ExteriorGradientFactory.h"
#include "Textures/ExteriorGradient.h"


#define LOCTEXT_NAMESPACE "ExteriorGradientFactory"

UExteriorGradientFactory::UExteriorGradientFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UExteriorGradient::StaticClass();
}

FText UExteriorGradientFactory::GetDisplayName() const
{
	return LOCTEXT("ExteriorGradientFactoryDescription", "Exterior gradient texture");
}

bool UExteriorGradientFactory::ConfigureProperties()
{
	return true;
}

bool UExteriorGradientFactory::CanCreateNew() const
{
	return true;
}

UObject* UExteriorGradientFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UExteriorGradient* ExteriorGradient = CastChecked<UExteriorGradient>(CreateOrOverwriteAsset(UExteriorGradient::StaticClass(), InParent, Name, Flags));

	if (ExteriorGradient == nullptr)
	{
		UE_LOG(LogTexture, Warning, TEXT("ExteriorGradient creation failed.\n"));
	}

	return ExteriorGradient;
}

#undef LOCTEXT_NAMESPACE
