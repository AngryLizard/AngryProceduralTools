

#pragma once

#include "CoreMinimal.h"

#include "NormalCurve.generated.h"


USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FNormalCurve
{
	GENERATED_USTRUCT_BODY()
		FNormalCurve();

	bool operator==(const FNormalCurve& Other) const;

	FVector2D Sample(float Time) const;

	UPROPERTY(EditAnywhere, Category = "Procedural")
		FVector X;

	UPROPERTY(EditAnywhere, Category = "Procedural")
		FVector Y;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FUnitCurve
{
	GENERATED_USTRUCT_BODY()
		FUnitCurve();

	bool operator==(const FUnitCurve& Other) const;

	float Sample(float Time) const;

	UPROPERTY(EditAnywhere, Category = "Procedural")
		FRichCurve CurveData;

};
