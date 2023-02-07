// The Gateway of Angry: Planes of Existence.

#pragma once

#include "ProceduralLibrary.h"
#include "FillDelaunayLibrary.generated.h"


//////////////////////////////////////////// STRUCT /////////////////////////////////////////////////


USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FFillSurfaceParams
{
	GENERATED_USTRUCT_BODY()
		FFillSurfaceParams();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		float CurveThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		bool Delaunay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		float Thickness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector Extrude;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FFillMaterialParams
{
	GENERATED_USTRUCT_BODY()
		FFillMaterialParams();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FProceduralMaterialParams Top;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FProceduralMaterialParams Rim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FProceduralMaterialParams Bottom;
};

/**
 *
 */
UCLASS()
class ANGRYPROCEDURALTOOLS_API UFillDelaunayLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		////////////////////////////////////////////////////////////////////////////////////////////////////
public:

	/** Generate skew mesh */
	UFUNCTION(BlueprintPure, Category = "Procedural Mesh", Meta = (Keywords = "C++"))
		static void GenerateFill(
			USplineComponent* Spline,
			const FTransform& Transform,
			FFillSurfaceParams Surface,
			FFillMaterialParams Material,

			TArray<FGenTriangleMesh>& Meshes);
};


