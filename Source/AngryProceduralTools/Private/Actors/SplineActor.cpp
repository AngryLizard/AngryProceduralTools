#include "Actors/SplineActor.h"
#include "Libraries/DiscreteMathLibrary.h"

FSplineSegment::FSplineSegment()
: StaticMesh(nullptr),
Axis(ESplineMeshAxis::Type::X),
From(0),
To(-1)
{
}


ASplineActor::ASplineActor(const FObjectInitializer& ObjectInitializer)
:	Super(ObjectInitializer)
{
	USceneComponent* Root = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, FName(TEXT("Root")));
	SetRootComponent(Root);
	Root->SetMobility(EComponentMobility::Static);

	Spline = ObjectInitializer.CreateDefaultSubobject<USplineComponent>(this, FName(TEXT("Spline")));
	Spline->SetupAttachment(Root);


	FSplineSegment Segment;
	Segment.StaticMesh = nullptr;
	Segment.From = 0;
	Segment.To = INDEX_NONE;
	Segments.Emplace(Segment);
}

void ASplineActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	GenerateInstances();
	OnSplineGenerated();
}

void ASplineActor::GenerateInstances()
{
	const int32 NumSegments = Spline->GetNumberOfSplineSegments();

	int32 SegmentIndex = 0;
	for (const FSplineSegment& Segment : Segments)
	{
		if (!IsValid(Segment.StaticMesh))
		{
			continue;
		}

		const int32 From = UDiscreteMathLibrary::PosMod(Segment.From, NumSegments);
		const int32 To = UDiscreteMathLibrary::PosMod(Segment.To, NumSegments);

		// Go along range
		int32 SplineIndex = From;
		while (true)
		{
			// Append new meshes if none have been cached yet
			if (SegmentIndex >= SplineMeshes.Num())
			{
				SplineMeshes.Emplace(nullptr);
			}

			// Update SplineMesh, create if null
			USplineMeshComponent*& Mesh = SplineMeshes[SegmentIndex++];
			if (Mesh == nullptr)
			{
				Mesh = NewObject<USplineMeshComponent>(this);
				Mesh->SetupAttachment(GetRootComponent());
				Mesh->RegisterComponent();
			}
			Mesh->BodyInstance = BodyInstance;
			Mesh->SetCullDistance(Spline->CachedMaxDrawDistance);

			Mesh->SetForwardAxis(Segment.Axis, false);

			Mesh->SetStaticMesh(Segment.StaticMesh);

			Mesh->EmptyOverrideMaterials();
			const int32 MaterialNum = Segment.Materials.Num();
			for (int32 MaterialIndex = 0; MaterialIndex < MaterialNum; MaterialIndex++)
			{
				Mesh->SetMaterial(MaterialIndex, Segment.Materials[MaterialIndex]);
			}

			Mesh->SetStartAndEnd(
				Spline->GetLocationAtSplinePoint(SplineIndex, ESplineCoordinateSpace::Local),
				Spline->GetTangentAtSplinePoint(SplineIndex, ESplineCoordinateSpace::Local),
				Spline->GetLocationAtSplinePoint(SplineIndex + 1, ESplineCoordinateSpace::Local),
				Spline->GetTangentAtSplinePoint(SplineIndex + 1, ESplineCoordinateSpace::Local), false);

			Mesh->SetStartScale(FVector2D(Spline->GetScaleAtSplinePoint(SplineIndex)), false);
			Mesh->SetEndScale(FVector2D(Spline->GetScaleAtSplinePoint(SplineIndex+1)), false);

			Mesh->SetStartRoll(Spline->GetRollAtSplinePoint(SplineIndex, ESplineCoordinateSpace::Local), false);
			Mesh->SetEndRoll(Spline->GetRollAtSplinePoint(SplineIndex + 1, ESplineCoordinateSpace::Local), false);

			Mesh->UpdateMesh();


			if (SplineIndex == To)
			{
				break;
			}
			SplineIndex = (SplineIndex + 1) % NumSegments;
		}
	}


	// Remove old
	const int32 NumMeshes = SplineMeshes.Num();
	for (int32 Index = SegmentIndex; Index < NumMeshes; Index++)
	{
		SplineMeshes[Index]->DestroyComponent();
	}
	SplineMeshes.SetNum(SegmentIndex);

}

int32 ASplineActor::CountSegments() const
{
	const int32 NumSegments = Spline->GetNumberOfSplineSegments();

	int32 Count = 0;
	for (const FSplineSegment& Segment : Segments)
	{
		const int32 From = UDiscreteMathLibrary::PosMod(Segment.From, NumSegments);
		const int32 To = UDiscreteMathLibrary::PosMod(Segment.To, NumSegments);
	}
	return Count;
}


void ASplineActor::Preview()
{
	GenerateInstances();
	OnSplineGenerated();
}
