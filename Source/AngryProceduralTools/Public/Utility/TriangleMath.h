#pragma once

#include "CoreMinimal.h"

#include "TriangleMath.generated.h"

/**
 *
 */
UCLASS()
class ANGRYPROCEDURALTOOLS_API UTriangleMath : public UObject
{
	GENERATED_BODY()
public:

	/** Get length of vector projected to [-1, 1]^2. */
	UFUNCTION(BlueprintPure, Category = "Math", meta = (Keywords = "C++"))
		static float ProjectToBox(const FVector2D& Vector);

	/** Project a segment (S - T) onto a slate from A to B with given normal. */
	UFUNCTION(BlueprintPure, Category = "Math", meta = (Keywords = "C++"))
		static bool ProjectToSlate(const FVector& Normal, const FVector& A, const FVector& B, const FVector& S, const FVector& T);

	/** Computes the circumcenter of a given triangle */
	UFUNCTION(BlueprintPure, Category = "Math", meta = (Keywords = "C++"))
		static bool ComputeCircumcenter(const FVector& A, const FVector& B, const FVector& C, FVector& Out);

	/** Computes the circumcenter of a given triangle */
	UFUNCTION(BlueprintPure, Category = "Math", meta = (Keywords = "C++"))
		static bool ComputeCircumcenter2D(const FVector2D& A, const FVector2D& B, const FVector2D& C, FVector2D& Out);
};
