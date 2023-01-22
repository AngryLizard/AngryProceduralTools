// The Gateway of Angry: Planes of Existence.

#pragma once

#include "ProceduralLibrary.h"
#include "SkewLibrary.generated.h"

//////////////////////////////////////////// STRUCTS /////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FSkewParams
{
	GENERATED_USTRUCT_BODY()
		FSkewParams();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector Scale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector Offset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector2D XSkew;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector2D YSkew;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector2D ZSkew;
};

/**
 *
 */
UCLASS()
class ANGRYPROCEDURALTOOLS_API USkewLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		////////////////////////////////////////////////////////////////////////////////////////////////////
public:

	/** Generate skew mesh */
	UFUNCTION(BlueprintPure, Category = "Procedural Mesh", Meta = (Keywords = "C++"))
		static void GenerateSkew(
			int32 LOD,
			UStaticMesh* StaticMesh,
			FSkewParams Skew,

			TArray<FTriangleMesh>& Meshes);

};

