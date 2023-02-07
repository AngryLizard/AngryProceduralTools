// The Gateway of Realities: Planes of Existence.

#pragma once

#include "CoreMinimal.h"
#include "Components/BillboardComponent.h"
#include "Utility/Triangulation.h"

#include "GameFramework/Actor.h"
#include "WorldPainterLayer.generated.h"

//////////////////////////////////////////// DECL /////////////////////////////////////////////////

class UProceduralMeshComponent;
class UWorldPainterTexture;
class UTextureRenderTarget2D;
class UBoxComponent;

//////////////////////////////////////////// STRUCTS /////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct FBrushData
{
	GENERATED_USTRUCT_BODY()

	FBrushData();
	FBrushData(int32 Num, const FLinearColor& Color);
	FLinearColor Get(int32 Index) const;
	void AddWeighted(const FBrushData& Data, float Weight);
	void Normalize();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ShowOnlyInnerProperties))
		TArray<FLinearColor> Slices;
};

USTRUCT(BlueprintType)
struct FBrushPoint
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Variance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector2D Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FBrushData Data;
};


/**
* AWorldPainterLayer
*/
UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class ANGRYPROCEDURALTOOLS_API AWorldPainterLayer : public AActor
{
	GENERATED_BODY()

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////// ENGINE ////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	AWorldPainterLayer(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void PostLoad() override;

	////////////////////////////////////////////////////////////////////////////////////////////////////
private:

	/** Bounding box for the painting bounds */
	UPROPERTY(EditAnywhere, Category = "World Painter Texture")
		TObjectPtr<UBoxComponent> BoxComponent;

	////////////////////////////////////////////////////////////////////////////////////////////////////
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Painter Texture")
		TArray<UTextureRenderTarget2D*> TargetTextures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Painter Texture")
		UMaterialInterface* RenderMaterial;

	/** Base color to be applied where there is no brush */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Painter Texture", meta = (ShowOnlyInnerProperties))
		FBrushData Bias;

	/** How much brush colors disperse into bias value */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Painter Texture", Meta = (ClampMin = 0, ClampMax = 20))
		float Dispersion = 5.0f;

	/** How much blending there is between triangles */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Painter Texture", Meta = (ClampMin = 0, ClampMax = 1))
		float Blending = 1.0f;

	/** Seed for the randome vertex generator */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Painter Texture")
		int32 GeneratorSeed = 69;

	/** Amount of disturbance for each vertex */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Painter Texture", Meta = (ClampMin = 0, ClampMax = 1))
		float VertexDisturbance = 0.5f;

	/** Number of vertices to draw per side */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Painter Texture", Meta = (ClampMin = 0, ClampMax = 256))
		int32 VertexSamples = 32;

	/** ID forwarded to the billboard material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Painter Texture", Meta = (ClampMin = 0, ClampMax = 256))
		int32 MaterialID = 0;

	/** Draw contained brushes to Target Texture */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Painter Texture", Meta = (Keywords = "C++"))
		void Bake();


#if WITH_EDITOR
	/** Called when a brush updated (currently unused) */
	void OnBrushUpdate(bool bFinished);

	////////////////////////////////////////////////////////////////////////////////////////////////////
protected:

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif // WITH_EDITOR

	////////////////////////////////////////////////////////////////////////////////////////////////////
private:

	// These procedures were split-up in case we ever want to update this at runtime (only some of the last few steps need to be repeated on brush/setting changes)
	bool GenerateTriangulationDone = false;
	void GenerateTriangulation();

	bool GeneratePointsDone = false;
	void GeneratePoints();

	bool GenerateVerticesDone = false;
	void GenerateVertices();

	bool GenerateTextureDone = false;
	void GenerateTexture();

	FTriangulation2D Triangulation;
	TArray<FBrushPoint> Points;
	TArray<FBrushData> Vertices;
};