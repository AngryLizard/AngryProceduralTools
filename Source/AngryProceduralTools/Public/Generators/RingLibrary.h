// The Gateway of Angry: Planes of Existence.

#pragma once

#include "Components/ArrowComponent.h"

#include "ProceduralLibrary.h"
#include "RingLibrary.generated.h"

//////////////////////////////////////////// STRUCT /////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FRingShapeParams
{
	GENERATED_USTRUCT_BODY()
		FRingShapeParams();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh", meta = (ClampMin = 0))
		int32 Segments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh", meta = (ClampMin = 0))
		float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh", meta = (ClampMin = 0))
		float Girth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		float Principal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		float Residual;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FRingMaterialParams
{
	GENERATED_USTRUCT_BODY()
		FRingMaterialParams();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		bool ProjectUV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FProceduralMaterialParams Material;

};

/**
 *
 */
UCLASS()
class ANGRYPROCEDURALTOOLS_API URingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	////////////////////////////////////////////////////////////////////////////////////////////////////
public:

	/** Generate ring mesh */
	UFUNCTION(BlueprintPure, Category = "Procedural Mesh", Meta = (Keywords = "C++"))
		static void GenerateRing(
			UArrowComponent* Direction,
			const FTransform& Transform,
			FRingShapeParams Shape,
			FRingMaterialParams Material,

			TArray<FGenTriangleMesh>& Meshes);

};

