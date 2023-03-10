

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

#include "GameFramework/Actor.h"
#include "SplineActor.generated.h"

//////////////////////////////////////////// DECL /////////////////////////////////////////////////

class USplineMeshComponent;
class USplineComponent;
class UStaticMesh;
class UMaterialInterface;

//////////////////////////////////////////// STRUCTS /////////////////////////////////////////////////


USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FSplineSegment
{
	GENERATED_USTRUCT_BODY()
		FSplineSegment();

	UPROPERTY(EditAnywhere, Category = "Spline Mesh")
		UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Spline Mesh", Meta = (ToolTip = "Material overrides."))
		TArray<UMaterialInterface*> Materials;

	UPROPERTY(EditAnywhere, Category = "Spline Mesh")
		TEnumAsByte<ESplineMeshAxis::Type> Axis;

	UPROPERTY(EditAnywhere, Category = "Spline Mesh")
		int32 From;

	UPROPERTY(EditAnywhere, Category = "Spline Mesh")
		int32 To;
};

/**
* SplineActor creates spline meshes
*/
UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class ANGRYPROCEDURALTOOLS_API ASplineActor : public AActor
{
	GENERATED_BODY()

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////// ENGINE ////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	ASplineActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void OnConstruction(const FTransform& Transform) override;

	//////////////////////////////////////////// IMPLEMENTABLES ////////////////////////////////////////

	/** Called every simulation tick when attached */
	UFUNCTION(BlueprintImplementableEvent, Category = "Spline Mesh", Meta = (Keywords = "C++"))
		void OnSplineGenerated();

	////////////////////////////////////////////// COMPONENTS //////////////////////////////////////////////////////
private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline Mesh", meta = (AllowPrivateAccess = "true"))
		USplineComponent* Spline;

	////////////////////////////////////////////////////////////////////////////////////////////////////
public:

	FORCEINLINE USplineComponent* GetSpline() const { return Spline; }

	////////////////////////////////////////////////////////////////////////////////////////////////////
public:

	/** Physics scene information for this component, holds a single rigid body with multiple shapes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spline Mesh", meta = (ShowOnlyInnerProperties, SkipUCSModifiedProperties))
		FBodyInstance BodyInstance;

	/** Generate spline mesh instances */
	UFUNCTION(BlueprintCallable, Category = "Procedural Mesh", Meta = (Keywords = "C++"))
		virtual void GenerateInstances();

	/** Meshes to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline Mesh")
		TArray<FSplineSegment> Segments;

	////////////////////////////////////////////////////////////////////////////////////////////////////
protected:

	/** Currently generated spline meshes */
	UPROPERTY(VisibleAnywhere, Category = "Spline Mesh")
		TArray<USplineMeshComponent*> SplineMeshes;

	/** Count how many segments are defined */
	UFUNCTION(BlueprintCallable, Category = "Spline Mesh", Meta = (Keywords = "C++"))
		int32 CountSegments() const;

	/** Generate procedural mesh */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Spline Mesh", Meta = (Keywords = "C++"))
		virtual void Preview();

};
