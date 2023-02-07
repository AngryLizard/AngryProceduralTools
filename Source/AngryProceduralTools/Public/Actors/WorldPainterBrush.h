// The Gateway of Realities: Planes of Existence.

#pragma once

#include "CoreMinimal.h"
#include "Components/BillboardComponent.h"
#include "WorldPainterLayer.h"

#include "GameFramework/Actor.h"
#include "WorldPainterBrush.generated.h"

//////////////////////////////////////////// DECL /////////////////////////////////////////////////


/**
* AWorldPainterBrush marks a region inside a texture region volume
*/
UCLASS(ClassGroup = (Custom), Blueprintable, meta = (BlueprintSpawnableComponent))
class ANGRYPROCEDURALTOOLS_API AWorldPainterBrush : public AActor
{
	GENERATED_BODY()

public:

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////// ENGINE ////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////

	AWorldPainterBrush(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	void OnConstruction(const FTransform& Transform) override;

	////////////////////////////////////////////////////////////////////////////////////////////////////
public:

	/** Size of this brush (distance variance) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Painter Brush", Meta = (ClampMin = 0, ClampMax = 1))
		float Size;

	/** Brush color to be applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Painter Texture", meta = (ShowOnlyInnerProperties))
		FBrushData Brush;

	/** Bake owning painter canvas */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Painter Brush", Meta = (Keywords = "C++"))
		void Bake();

	////////////////////////////////////////////////////////////////////////////////////////////////////
protected:

	/** Get layer this brush paints to (attach parent) */
	UFUNCTION()
		AWorldPainterLayer* GetPainterLayer() const;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif // WITH_EDITOR

private:

#if WITH_EDITORONLY_DATA
private:
	/** Billboard Component displayed in editor */
	UPROPERTY()
		class UStaticMeshComponent* PreviewComponent;

	////////////////////////////////////////////////////////////////////////////////////////////////////
protected:

	/** Data of this brush */
	UPROPERTY()
		UMaterialInstanceDynamic* DynamicMaterial;

	/** Update billboard material */
	void UpdateBillboardMaterial();

public:
	/** Returns PreviewComponent subobject **/
	class UStaticMeshComponent* GetPreviewComponent() const;
#endif
};

UCLASS()
class ANGRYPROCEDURALTOOLS_API UWorldPainterNotifyRootComponent : public USceneComponent
{
	GENERATED_BODY()

		friend class AWorldPainterBrush;

	virtual void OnAttachmentChanged() override;
private:
	TWeakObjectPtr<AWorldPainterBrush> Brush;
};
