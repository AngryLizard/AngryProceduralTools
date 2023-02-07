// The Gateway of Realities: Planes of Existence.

#include "Actors/WorldPainterLayer.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/ArrowComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Actors/WorldPainterBrush.h"
#include "Components/BoxComponent.h"

#include "Engine/Canvas.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Engine/VolumeTexture.h"
#include "Editor/UnrealEd/Public/Editor.h"

#define INV_SQRTWOPI 0.398942280401f

FBrushData::FBrushData()
	: FBrushData(2, FLinearColor::Black)
{
}

FBrushData::FBrushData(int32 Num, const FLinearColor& Color)
{
	for (int32 Index = 0; Index < Num; Index++)
	{
		Slices.Emplace(Color);
	}
}

FLinearColor FBrushData::Get(int32 Index) const
{
	if (Slices.IsValidIndex(Index)) return Slices[Index];
	return FLinearColor(0.f, 0.f, 0.f, 0.f);
}

void FBrushData::AddWeighted(const FBrushData& Data, float Weight)
{
	const int32 Num = FMath::Min(Data.Slices.Num(), Slices.Num());
	for (int32 Index = 0; Index < Num; Index++)
	{
		const float Intensity = Data.Slices[Index].A * Weight;
		Slices[Index].R += Data.Slices[Index].R * Intensity;
		Slices[Index].G += Data.Slices[Index].G * Intensity;
		Slices[Index].B += Data.Slices[Index].B * Intensity;
		Slices[Index].A += Intensity;
	}
}

void FBrushData::Normalize()
{
	const int32 Num = Slices.Num();
	for (int32 Index = 0; Index < Num; Index++)
	{
		if (!FMath::IsNearlyZero(Slices[Index].A))
		{
			Slices[Index].R /= Slices[Index].A;
			Slices[Index].G /= Slices[Index].A;
			Slices[Index].B /= Slices[Index].A;
			Slices[Index].A = 1.0f;
		}
	}
}



AWorldPainterLayer::AWorldPainterLayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	RenderMaterial(nullptr)
{
	bIsEditorOnlyActor = true;

	USceneComponent* Root = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, FName(TEXT("Root")));
	Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent0"));
	BoxComponent->Mobility = EComponentMobility::Movable;
	BoxComponent->SetRelativeScale3D(FVector(100.0));
	BoxComponent->SetCanEverAffectNavigation(false);

	BoxComponent->bDrawOnlyIfSelected = true;
	BoxComponent->bUseAttachParentBound = false;
	BoxComponent->bUseEditorCompositing = true;
	BoxComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	BoxComponent->InitBoxExtent(FVector(0.5f, 0.5f, 0.5f));
	BoxComponent->SetupAttachment(Root);

	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> WPCanvasMaterial;

		FConstructorStatics()
			: WPCanvasMaterial(TEXT("/AngryProceduralTools/Resources/M_WorldPainter_Bake.M_WorldPainter_Bake"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;
	RenderMaterial = ConstructorStatics.WPCanvasMaterial.Get();
}

void AWorldPainterLayer::PostLoad()
{
	Super::PostLoad();

#if WITH_EDITOR
	Bake();
#endif // WITH_EDITOR
}

#if WITH_EDITOR

void AWorldPainterLayer::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	FProperty* ChangedProperty = PropertyChangedEvent.Property;

	if (ChangedProperty)
	{
		if (ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AWorldPainterLayer, GeneratorSeed) ||
			ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AWorldPainterLayer, VertexDisturbance) ||
			ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AWorldPainterLayer, VertexSamples))
		{
			GenerateTriangulationDone = false;
			GenerateVerticesDone = false;
			GenerateTextureDone = false;
		}

		if (ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AWorldPainterLayer, Dispersion))
		{
			GenerateVerticesDone = false;
			GenerateTextureDone = false;
		}

		if (ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AWorldPainterLayer, TargetTextures) ||
			ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AWorldPainterLayer, RenderMaterial) ||
			ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AWorldPainterLayer, Blending))
		{
			GenerateTextureDone = false;
		}
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void AWorldPainterLayer::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	FProperty* ChangedProperty = PropertyChangedEvent.Property;

	if (ChangedProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AWorldPainterLayer, Bias))
	{
		GenerateVerticesDone = false;
		GenerateTextureDone = false;
	}
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void AWorldPainterLayer::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	OnBrushUpdate(bFinished);
}

void AWorldPainterLayer::OnBrushUpdate(bool bFinished)
{
	GeneratePointsDone = false;
	GenerateVerticesDone = false;
	GenerateTextureDone = false;

	if (bFinished)
	{
		Bake();
	}
}
#endif // WITH_EDITOR



void AWorldPainterLayer::Bake()
{
	if (!GenerateTriangulationDone) GenerateTriangulation();
	if (!GeneratePointsDone) GeneratePoints();
	if (!GenerateVerticesDone) GenerateVertices();
	if (!GenerateTextureDone) GenerateTexture();
	#if WITH_EDITOR
	if (GEditor) {
		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (World) {
			GEditor->Exec(World, TEXT("r.VT.Flush"));
		}
	}
	#endif // WITH_EDITOR
}


void AWorldPainterLayer::GenerateTriangulation()
{
	FRandomStream Random(GeneratorSeed);

	TArray<FVector2D> Samples;
	Samples.Reserve(VertexSamples + 4);
	for (int32 SampleX = -1; SampleX <= VertexSamples; SampleX++)
	{
		for (int32 SampleY = -1; SampleY <= VertexSamples; SampleY++)
		{
			const float X = FMath::Clamp((((float)SampleX) + 0.5f + Random.FRandRange(-0.5f, 0.5f) * VertexDisturbance) / VertexSamples, 0.0f, 1.0f);
			const float Y = FMath::Clamp((((float)SampleY) + 0.5f + Random.FRandRange(-0.5f, 0.5f) * VertexDisturbance) / VertexSamples, 0.0f, 1.0f);
			Samples.Emplace(FVector2D(X, Y));
		}
	}

	// Create triangulation
	Triangulation.QHull(Samples, -1);
	Triangulation.FixTriangles(-1);

	GenerateTriangulationDone = true;
}

void AWorldPainterLayer::GeneratePoints()
{
	Points.Empty();

	FTransform Transform = BoxComponent->GetComponentTransform();

	TArray<AActor*> Actors;
	GetAttachedActors(Actors, true);
	for (AActor* Actor : Actors)
	{
		AWorldPainterBrush* Brush = Cast<AWorldPainterBrush>(Actor);
		if (IsValid(Brush))
		{
			const FVector Local = Transform.InverseTransformPosition(Brush->GetActorLocation()) / FVector::Max(BoxComponent->GetUnscaledBoxExtent(), FVector(KINDA_SMALL_NUMBER));

			FBrushPoint Point;
			Point.Variance = Brush->Size;
			Point.Position = FVector2D(Local + FVector::OneVector) / 2;
			Point.Data = Brush->Brush;
			Points.Emplace(Point);
		}
	}

	GeneratePointsDone = true;
}

void AWorldPainterLayer::GenerateVertices()
{
	const int32 PointCount = Points.Num();
	const int32 SliceNum = Bias.Slices.Num();
	const int32 VertexCount = Triangulation.Points.Num();

	Vertices.SetNum(VertexCount);

	const float BiasWeight = FMath::Exp(-Dispersion);
	for (int32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
	{
		const FVector2D& Origin = Triangulation.Points[VertexIndex];

		FBrushData& Vertex = Vertices[VertexIndex];
		Vertex = FBrushData(SliceNum, FLinearColor(0.f, 0.f, 0.f, 0.f));
		Vertex.AddWeighted(Bias, BiasWeight);
		for (int32 PointIndex = 0; PointIndex < PointCount; PointIndex++)
		{
			const FBrushPoint& Point = Points[PointIndex];

			const float DS = (Point.Position - Origin).SizeSquared();
			const float Weight = INV_SQRTWOPI / Point.Variance * FMath::Exp(DS / (Point.Variance * Point.Variance * -2));
			Vertex.AddWeighted(Point.Data, Weight);
		}
		Vertex.Normalize();
	}

	GenerateVerticesDone = true;
}

void AWorldPainterLayer::GenerateTexture()
{
	const int32 TriangleCount = Triangulation.Triangles.Num();
	if (IsValid(RenderMaterial))
	{
		TArray<FCanvasUVTri> Tris;
		Tris.SetNum(TriangleCount);

		const int32 SliceNum = Bias.Slices.Num();
		for (int32 Slice = 0; Slice < SliceNum; Slice++)
		{
			if (TargetTextures.IsValidIndex(Slice))
			{
				UTextureRenderTarget2D* TargetTexture = TargetTextures[Slice];
				if (IsValid(TargetTexture))
				{
					UCanvas* Canvas;
					FVector2D Size;
					FDrawToRenderTargetContext Context;
					UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, TargetTexture, Canvas, Size, Context);

					for (int32 TriangleIndex = 0; TriangleIndex < TriangleCount; TriangleIndex++)
					{
						const FGenTriangle& Triangle = Triangulation.Triangles[TriangleIndex];

						FCanvasUVTri& Tri = Tris[TriangleIndex];

						const FVector2D& P0 = Triangulation.Points[Triangle.Verts[0]];
						const FVector2D& P1 = Triangulation.Points[Triangle.Verts[1]];
						const FVector2D& P2 = Triangulation.Points[Triangle.Verts[2]];

						const FLinearColor& V0 = Vertices[Triangle.Verts[0]].Get(Slice);
						const FLinearColor& V1 = Vertices[Triangle.Verts[1]].Get(Slice);
						const FLinearColor& V2 = Vertices[Triangle.Verts[2]].Get(Slice);

						const FLinearColor Mean = (V0 + V1 + V2) / 3;

						Tri.V0_Pos = P0 * Size;
						Tri.V0_UV = P0;
						Tri.V0_Color = FMath::Lerp(Mean, V0, Blending);

						Tri.V1_Pos = P1 * Size;
						Tri.V1_UV = P1;
						Tri.V1_Color = FMath::Lerp(Mean, V1, Blending);

						Tri.V2_Pos = P2 * Size;
						Tri.V2_UV = P2;
						Tri.V2_Color = FMath::Lerp(Mean, V2, Blending);
					}

					Canvas->K2_DrawMaterialTriangle(RenderMaterial, Tris);
					UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
				}
			}
		}
	}
	GenerateTextureDone = true;
}
