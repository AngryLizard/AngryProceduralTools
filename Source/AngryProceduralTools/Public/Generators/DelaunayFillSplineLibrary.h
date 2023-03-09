

#pragma once

#include "ProceduralLibrary.h"
#include "DelaunayFillSplineLibrary.generated.h"


//////////////////////////////////////////// STRUCTS /////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FDelaunaySurfaceParams
{
	GENERATED_USTRUCT_BODY()
		FDelaunaySurfaceParams();

	float SampleCurve(int32 Index, float Time) const;
	float SampleCurves(float X, float Y) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		TArray<FRuntimeFloatCurve> Curves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		float FillerMaxSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		float FillerHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		bool Delaunay;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FDelaunayMaterialParams
{
	GENERATED_USTRUCT_BODY()
		FDelaunayMaterialParams();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		bool ProjectUV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FProceduralMaterialParams Material;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FDelaunayInstanceParams
{
	GENERATED_USTRUCT_BODY()
		FDelaunayInstanceParams();

	FVector SampleAlignment(FRandomStream& Random);
	float SampleRotation(FRandomStream& Random);

	// Size of the instance in each dimension
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector InstanceExtend;
	
	// All possible up-vector directions the instances can spawn in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		TArray<FVector> InstanceAlignments;

	// All possible rotations around Z-Axis in radians, interval form
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		TArray<FVector2D> InstanceRotations;

	// Whether to place the instances on the mean of all connecting vertices or according to the voronoi points.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		bool PlaceOnMean;

	// Whether to take neighbouring neighbour's principal axis into consideration or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		bool RotateAlongPrincipal;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FDelaunayHoleParams
{
	GENERATED_USTRUCT_BODY()
		FDelaunayHoleParams();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		float Radius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		TArray<FTransform> Transforms;
};

/**
 *
 */
UCLASS()
class ANGRYPROCEDURALTOOLS_API UDelaunayFillSplineLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		////////////////////////////////////////////////////////////////////////////////////////////////////
public:

		/** This fills the space between two splines using delaunay triangulation.
		* For every triangle an instance (output as transforms) can be generated.
		* These instances are rotated according to the instance options.
		*/
		UFUNCTION(BlueprintPure, Category = "Procedural Mesh", Meta = (Keywords = "C++"))
			static void GenerateDelaunay(
				USplineComponent* Left, 
				USplineComponent* Right,
				const FTransform& Transform,
				FDelaunaySurfaceParams Surface,
				FDelaunayMaterialParams Material,
				FDelaunayInstanceParams Instances,
				FDelaunayHoleParams Holes,
				
				TArray<FGenTriangleMesh>& Meshes,
				TArray<FTransform>& Transforms);

};
