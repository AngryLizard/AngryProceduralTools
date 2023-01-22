// The Gateway of Angry: Planes of Existence.

#pragma once

#include "ProceduralLibrary.h"
#include "RidgeFillSplineLibrary.generated.h"

//////////////////////////////////////////// STRUCT /////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FRidgeCurvePoint
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, Category = "Procedural Mesh")
		float Ratio = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Procedural Mesh")
		float Value = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Procedural Mesh")
		float Slope = 0.0f;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FRidgeSplinePoint
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, Category = "Procedural Mesh")
		int32 LeftIndex = -1;

	UPROPERTY(EditAnywhere, Category = "Procedural Mesh")
		int32 RightIndex = -1;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FRidgeSegmentPoint
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, Category = "Procedural Mesh")
		float LeftDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Procedural Mesh")
		float RightDistance = 0.0f;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

UENUM(BlueprintType)
enum class ERidgeFillSplineType : uint8
{
	/** Spread edge loops evenly along splines */
	Spread,
	/** Edge loops bridge spline points with same index */
	Match,
	/** Edge loops bridge spline points closest to each other based on distance/angle */
	Dynamic
};


USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FRidgeSurfaceParams
{
	GENERATED_USTRUCT_BODY()
		FRidgeSurfaceParams();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FRuntimeFloatCurve Curve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh", meta = (ClampMin = 0))
		int32 FillerSplines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh", meta = (ClampMin = 0))
		int32 FillerSegments;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		float FillerHeight;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FRidgeMaterialParams
{
	GENERATED_USTRUCT_BODY()
		FRidgeMaterialParams();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		bool ProjectUV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		float UnwrapLane;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FProceduralMaterialParams Material;

};

/**
 *
 */
UCLASS()
class ANGRYPROCEDURALTOOLS_API URidgeFillSplineLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	////////////////////////////////////////////////////////////////////////////////////////////////////
public:

	/** Generate skew mesh */
	UFUNCTION(BlueprintPure, Category = "Procedural Mesh", Meta = (Keywords = "C++"))
		static void GenerateRidge(
			USplineComponent* Left,
			USplineComponent* Right,
			const FTransform& Transform,
			FRidgeSurfaceParams Surface,
			FRidgeMaterialParams Material,
			ERidgeFillSplineType Type,

			TArray<FTriangleMesh>& Meshes);

};

