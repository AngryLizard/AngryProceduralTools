// The Gateway of Angry: Planes of Existence.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralGradient.h"

#include "Utility/NormalCurve.h"
#include "ExteriorGradient.generated.h"


USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FExteriorColumn
{
	GENERATED_USTRUCT_BODY()
		FExteriorColumn();

	UPROPERTY(EditAnywhere, Category = "Procedural", meta = (DisplayName = "Color X/Y", Color="Red"))
		FNormalCurve Color;

	UPROPERTY(EditAnywhere, Category = "Procedural", meta = (DisplayName = "Roughness Min/Max", Color = "Green"))
		FNormalCurve Roughness;

	UPROPERTY(EditAnywhere, Category = "Procedural", meta = (DisplayName = "Metal/Glow", Color = "Blue"))
		FNormalCurve Meglow;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FExteriorRow
{
	GENERATED_USTRUCT_BODY()
		FExteriorRow();

	UPROPERTY(EditAnywhere, Category = "Procedural")
		FString Comment;

	UPROPERTY(EditAnywhere, Category = "Procedural")
		FExteriorColumn Columns[4];
};

////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(HideCategories = Object, BlueprintType)
class ANGRYPROCEDURALTOOLS_API UExteriorGradient : public UProceduralGradient
{
	GENERATED_UCLASS_BODY()

public:
	virtual void GenerateGradient(int32 SizeX, int32 SizeY, TArray<FLinearColor>& Pixels) override;

	/** Colors */
	UPROPERTY(EditAnywhere, Category = "Procedural")
		TArray<FExteriorRow> Rows;

	UPROPERTY(EditAnywhere, Category = "Procedural", meta = (ClampMin = 1, ClampMax = 64))
		int32 RowThickness;

};
