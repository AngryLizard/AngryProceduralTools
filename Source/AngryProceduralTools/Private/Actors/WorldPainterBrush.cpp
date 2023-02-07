// The Gateway of Realities: Planes of Existence.

#include "Actors/WorldPainterBrush.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/ArrowComponent.h"
#include "UObject/ConstructorHelpers.h"

AWorldPainterBrush::AWorldPainterBrush(const FObjectInitializer& ObjectInitializer)
:	Super(ObjectInitializer),
	Size(0.1f)
{
	bIsEditorOnlyActor = true;

#if WITH_EDITORONLY_DATA
	DynamicMaterial = nullptr;

	UWorldPainterNotifyRootComponent* Root = CreateEditorOnlyDefaultSubobject<UWorldPainterNotifyRootComponent>(TEXT("Root"));
	Root->Brush = this;
	SetRootComponent(Root);

	PreviewComponent = CreateEditorOnlyDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UStaticMesh> WPBrushMeshObject;

			FConstructorStatics()
				: WPBrushMeshObject(TEXT("/AngryProceduralTools/Resources/SM_WorldPainter_BrushMesh.SM_WorldPainter_BrushMesh"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		PreviewComponent->SetRelativeScale3D_Direct(FVector(1.f, 1.f, 1.f));
		PreviewComponent->SetStaticMesh(ConstructorStatics.WPBrushMeshObject.Get());
		PreviewComponent->SetupAttachment(RootComponent);
	}

#endif // WITH_EDITORONLY_DATA

	PrimaryActorTick.bCanEverTick = false;
	bAllowTickBeforeBeginPlay = true;
	bReplicates = false;
	NetUpdateFrequency = 10.0f;
	SetHidden(true);
	SetReplicatingMovement(false);
	SetCanBeDamaged(false);
}

void AWorldPainterBrush::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITORONLY_DATA
	UMaterialInterface* Material = GetPreviewComponent()->GetMaterial(0);
	if (IsValid(Material) && !IsValid(DynamicMaterial))
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
	}

	UpdateBillboardMaterial();
#endif
}

void AWorldPainterBrush::Bake()
{
	AWorldPainterLayer* PainterLayer = GetPainterLayer();
	if (PainterLayer)
	{
		PainterLayer->Bake();
	}
}

AWorldPainterLayer* AWorldPainterBrush::GetPainterLayer() const
{
	return Cast<AWorldPainterLayer>(GetAttachParentActor());
}

#if WITH_EDITOR

void AWorldPainterBrush::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	FProperty* ChangedProperty = PropertyChangedEvent.Property;

	if (ChangedProperty)
	{
		if (ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AWorldPainterBrush, Size))
		{
			UpdateBillboardMaterial();
			AWorldPainterLayer* PainterLayer = GetPainterLayer();
			if (PainterLayer)
			{
				PainterLayer->OnBrushUpdate(true);
			}
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AWorldPainterBrush::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	FProperty* ChangedProperty = PropertyChangedEvent.Property;

	if (ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(FBrushData, Slices))
	{
		UpdateBillboardMaterial();
		AWorldPainterLayer* PainterLayer = GetPainterLayer();
		if (PainterLayer)
		{
			PainterLayer->OnBrushUpdate(true);
		}
	}
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void AWorldPainterBrush::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	PreviewComponent->SetRelativeScale3D_Direct(FVector(1.f, 1.f, 1.f));
	PreviewComponent->SetWorldRotation(FQuat::Identity);

	AWorldPainterLayer* PainterLayer = GetPainterLayer();
	if (PainterLayer)
	{
		PainterLayer->OnBrushUpdate(bFinished);
	}
}

#endif // WITH_EDITOR

#if WITH_EDITORONLY_DATA
void AWorldPainterBrush::UpdateBillboardMaterial()
{
	if (IsValid(DynamicMaterial))
	{
		const int32 Num = Brush.Slices.Num();
		for (int32 Index = 0; Index < Num; Index++)
		{
			DynamicMaterial->SetVectorParameterValue(*FString::Printf(TEXT("Color%d"), Index), Brush.Slices[Index]);
		}

		AWorldPainterLayer* PainterLayer = GetPainterLayer();
		if (IsValid(PainterLayer))
		{
			DynamicMaterial->SetScalarParameterValue("CanvasID", PainterLayer->MaterialID);
			DynamicMaterial->SetScalarParameterValue("SliceCount", PainterLayer->Bias.Slices.Num());
		}

		GetPreviewComponent()->SetMaterial(0, DynamicMaterial);
	}
}

/** Returns PreviewComponent subobject **/
UStaticMeshComponent* AWorldPainterBrush::GetPreviewComponent() const { return PreviewComponent; }
#endif

void UWorldPainterNotifyRootComponent::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();

#if WITH_EDITOR
	if (Brush.IsValid())
	{
		Brush->Bake();
	}
#endif // WITH_EDITOR
}