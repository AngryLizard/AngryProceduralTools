#include "Actors/ProceduralActor.h"
#include "ProceduralMeshComponent/Public/ProceduralMeshComponent.h"
#include "Generators/ProceduralLibrary.h"

AProceduralActor::AProceduralActor(const FObjectInitializer& ObjectInitializer)
:	Super(ObjectInitializer),
	EnableAutoGenerate(true),
	PreviewLOD(0),
	MaxLOD(3),
	CollisionLOD(2),
	EnableCollision(true)
{
	USceneComponent* Root = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, FName(TEXT("Root")));
	SetRootComponent(Root);
	Root->SetMobility(EComponentMobility::Static);

	ProceduralMesh = ObjectInitializer.CreateDefaultSubobject<UProceduralMeshComponent>(this, FName(TEXT("ProceduralMesh")));
	ProceduralMesh->SetupAttachment(Root);
}

void AProceduralActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (EnableAutoGenerate)
	{
		Generate(PreviewLOD);
	}
}

TArray<FGenTriangleMesh> AProceduralActor::GenerateMesh_Implementation(const FTransform& Transform, int32 LOD) const
{
	return TArray<FGenTriangleMesh>();
}

bool AProceduralActor::Generate(int32 LOD)
{
	FEditorScriptExecutionGuard ScriptGuard;

	const FTransform& Base = ProceduralMesh->GetComponentTransform();
	const TArray<FGenTriangleMesh> Meshes = GenerateMesh(Base, LOD);
	if (Meshes.Num() > 0)
	{
		UProceduralLibrary::ApplyToMeshes(ProceduralMesh, Meshes, EnableCollision);
		return true;
	}
	return false;
}

void AProceduralActor::Preview()
{
	Generate(PreviewLOD);
}
