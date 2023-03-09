

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveLinearColor.h"
#include "ProceduralGradient.h"

#include "Utility/NormalCurve.h"
#include "PaintGradient.generated.h"

class UPaintGradient;

////////////////////////////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FGradientFilter
{
	GENERATED_USTRUCT_BODY()
		FGradientFilter();

	FLinearColor HSVFilter(const FLinearColor& HSV, float Time) const;
	FLinearColor RGBFilter(const FLinearColor& RGB, float Time) const;

	UPROPERTY(EditAnywhere, Category = "Procedural", meta = (ClampMin = -1.0, ClampMax = 1.0))
		FUnitCurve Temperature;

	UPROPERTY(EditAnywhere, Category = "Procedural", meta = (ClampMin = -1.0, ClampMax = 1.0))
		FUnitCurve MagentaGreen;

	UPROPERTY(EditAnywhere, Category = "Procedural", meta = (ClampMin = 0.0, ClampMax = 360))
		FUnitCurve Hue;

	UPROPERTY(EditAnywhere, Category = "Procedural", meta = (ClampMin = -1.0, ClampMax = 1.0))
		FUnitCurve Saturation;

	UPROPERTY(EditAnywhere, Category = "Procedural", meta = (ClampMin = -1.0, ClampMax = 1.0))
		FUnitCurve Value;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FGradientSection
{
	GENERATED_USTRUCT_BODY()
		FGradientSection();

	UPROPERTY(EditAnywhere, Category = "Procedural")
		FGradientFilter Horizontal;

	UPROPERTY(EditAnywhere, Category = "Procedural")
		FGradientFilter Vertical;

	UPROPERTY(EditAnywhere, Category = "Procedural")
		TArray<UCurveLinearColor*> Curves;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FGradientCache
{
	GENERATED_USTRUCT_BODY()
		FGradientCache();

	UPROPERTY(EditAnywhere, Category = "Procedural")
		int32 SizeX;

	UPROPERTY(EditAnywhere, Category = "Procedural")
		int32 SizeY;

	UPROPERTY(EditAnywhere, Category = "Procedural")
		int32 Height;

	UPROPERTY(EditAnywhere, Category = "Procedural")
		int32 Offset;

	UPROPERTY(EditAnywhere, Category = "Procedural")
		TArray<FLinearColor> Pixels;

	/** Base colors */
	UPROPERTY(EditAnywhere, Category = "Procedural")
		TArray<UCurveLinearColor*> Curves;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

UCLASS(HideCategories = Object, BlueprintType)
class ANGRYPROCEDURALTOOLS_API UPaintGradient : public UProceduralGradient
{
	GENERATED_UCLASS_BODY()

public:

#if WITH_EDITOR
	void UpdateCurveSlot(UCurveBase* Curve, EPropertyChangeType::Type ChangeType);
#endif

	/**
	 * Sample gradient curves (checks cache first)
	 */
	UFUNCTION()
		void SampleGradientCurves();

	virtual void GenerateGradient(int32 SizeX, int32 SizeY, TArray<FLinearColor>& Pixels) override;

	/** Whether the pixel cache is marked dirty */
	UPROPERTY(Transient)
		bool CacheDirty;

protected:

	/** Pixel cache after curve sampler */
	UPROPERTY(Transient)
		TArray<FGradientCache> PixelCache;

public:

	/** Colors */
	UPROPERTY(EditAnywhere, Category = "Procedural")
		TArray<FGradientSection> Sections;

};
