#include "Generators/ProceduralLibrary.h"
#include "ProceduralMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"

FProceduralMaterialParams::FProceduralMaterialParams()
:	NormalisedUV(false),
	TextureSize(1024.0f, 1024.0f),
	TexelDensity(512.0f, 512.0f),
	UVOffset(FVector2D::ZeroVector),
	UVScale(FVector2D(1.0f)),
	Material(nullptr),
	VertexColor(FLinearColor::White)
{
}

FVector2D FProceduralMaterialParams::Transform(const FVector2D& UV, const FVector2D& Bounds) const
{
	if (UVScale.GetAbs().GetMin() < SMALL_NUMBER) return FVector2D();
	if (NormalisedUV)
	{
		return (UV + UVOffset) * UVScale;
	}

	const FVector2D Spread = FVector2D::Max(Bounds, FVector2D(SMALL_NUMBER));
	const FVector2D Ratio = TexelDensity * Spread / FVector2D::Max(TextureSize, FVector2D(SMALL_NUMBER));
	return (UV * 0.01f * Ratio + UVOffset) * UVScale;
}

FVector2D FProceduralMaterialParams::Inverse(const FVector2D& UV, const FVector2D& Bounds) const
{
	if (UVScale.GetAbs().GetMin() < SMALL_NUMBER) return FVector2D();
	if (NormalisedUV)
	{
		return (UV / UVScale) - UVOffset;
	}

	const FVector2D Spread = FVector2D::Max(Bounds, FVector2D(SMALL_NUMBER));
	const FVector2D Ratio = TexelDensity * Spread / FVector2D::Max(TextureSize, FVector2D(SMALL_NUMBER));
	return (UV / UVScale - UVOffset) / Ratio / 0.01f;
}

FProceduralStaticMesh::FProceduralStaticMesh()
:	Weight(1.0f),
	Offset(FVector::ZeroVector),
	StaticMesh(nullptr),
	Axis(ESplineMeshAxis::X)
{
}

FProceduralSplineMesh::FProceduralSplineMesh()
:	MinLength(0.0f),
	MaxLength(0.0f),
	CanBeTail(true),
	CanBeSegment(true),
	CanBeHead(true)
{
}

FProceduralPostMesh::FProceduralPostMesh()
:	MinAngle(0.0f),
	MaxAngle(180.0f)
{
}

FProceduralSplineMeshArray::FProceduralSplineMeshArray()
:	PointAligned(false),
	PostDistances(0.0f),
	Offset(FVector::ZeroVector)
{
}

FProceduralMeshContainer::FProceduralMeshContainer()
{
}


FVector2D UProceduralLibrary::ProjectUV(const FVector& Location, const FVector& Normal, const FVector2D& Bounds)
{
	// Use local vertex location for projection
	FVector XAxis = FVector::ForwardVector;
	FVector YAxis = FVector::RightVector;
	FVector ZAxis = Normal;
	FVector::CreateOrthonormalBasis(XAxis, YAxis, ZAxis);
	const float U = Location | XAxis;
	const float V = Location | YAxis;
	return FVector2D(U, V) / FVector2D::Max(Bounds, FVector2D(SMALL_NUMBER));
}

FVector2D UProceduralLibrary::ComputeTwinBounds(USplineComponent* Left, USplineComponent* Right, const FVector& Normal)
{
	return ProjectUV((Left->Bounds + Right->Bounds).BoxExtent * 2, Normal, FVector2D(1.0f));
}

FVector2D UProceduralLibrary::ComputeBounds(USplineComponent* Spline, const FVector& Normal)
{
	return ProjectUV(Spline->Bounds.BoxExtent * 2, Normal, FVector2D(1.0f));
}


float UProceduralLibrary::GetAveragePointDistance(USplineComponent* Left, USplineComponent* Right)
{
	float AverageDistance = 0.0f;

	const int32 LeftNum = Left->GetNumberOfSplineSegments();
	const int32 RightNum = Right->GetNumberOfSplineSegments();
	const int32 Points = FMath::Min(LeftNum, RightNum);
	for (int32 Point = 0; Point < Points; Point++)
	{
		// Compute position on the spline
		const FVector From = Left->GetLocationAtSplinePoint(Point, ESplineCoordinateSpace::World);
		const FVector To = Right->GetLocationAtSplinePoint(Point, ESplineCoordinateSpace::World);
		AverageDistance += (To - From).Size();
	}
	return AverageDistance / Points;
}


float UProceduralLibrary::GetMaxPointDistance(USplineComponent* Left, USplineComponent* Right)
{
	float MaxDistance = 0.0f;

	const int32 LeftNum = Left->GetNumberOfSplineSegments();
	const int32 RightNum = Right->GetNumberOfSplineSegments();
	const int32 Points = FMath::Min(LeftNum, RightNum);
	for (int32 Point = 0; Point < Points; Point++)
	{
		// Compute position on the spline
		const FVector From = Left->GetLocationAtSplinePoint(Point, ESplineCoordinateSpace::World);
		const FVector To = Right->GetLocationAtSplinePoint(Point, ESplineCoordinateSpace::World);
		MaxDistance = FMath::Max(MaxDistance, (To - From).Size());
	}
	return MaxDistance / Points;
}

float UProceduralLibrary::GetMinPointDistance(USplineComponent* Left, USplineComponent* Right)
{
	float MinDistance = 0.0f;

	const int32 LeftNum = Left->GetNumberOfSplineSegments();
	const int32 RightNum = Right->GetNumberOfSplineSegments();
	const int32 Points = FMath::Min(LeftNum, RightNum);
	for (int32 Point = 0; Point < Points; Point++)
	{
		// Compute position on the spline
		const FVector From = Left->GetLocationAtSplinePoint(Point, ESplineCoordinateSpace::World);
		const FVector To = Right->GetLocationAtSplinePoint(Point, ESplineCoordinateSpace::World);
		MinDistance = FMath::Min(MinDistance, (To - From).Size());
	}
	return MinDistance / Points;
}


FGenTriangleMesh UProceduralLibrary::CombineMesheSection(const FGenTriangleMesh& Mesh, const FGenTriangleMesh& Other)
{
	FGenTriangleMesh Output = Mesh;

	const int32 VertexNum = Mesh.Triangulation.Points.Num();
	const int32 TriangleNum = Mesh.Triangulation.Triangles.Num();
	TArray<FGenTriangle> Triangles = Other.Triangulation.Triangles;
	for (FGenTriangle& Triangle : Triangles)
	{
		Triangle.Verts[0] += VertexNum;
		Triangle.Verts[1] += VertexNum;
		Triangle.Verts[2] += VertexNum;
		Triangle.Adjs[0] += TriangleNum;
		Triangle.Adjs[1] += TriangleNum;
		Triangle.Adjs[2] += TriangleNum;
	}
	Output.Triangulation.Points.Append(Other.Triangulation.Points);
	Output.Triangulation.Triangles.Append(Triangles);
	Output.Vertices.Append(Other.Vertices);
	Output.Convex.Append(Other.Convex);
	return Output;
}

TArray<FGenTriangleMesh> UProceduralLibrary::CombineMeshes(const TArray<FGenTriangleMesh>& Meshes, const TArray<FGenTriangleMesh>& Other)
{
	TArray<FGenTriangleMesh> Output;
	Output.Append(Meshes);
	Output.Append(Other);
	return Output;
}

TArray<FGenTriangleMesh> UProceduralLibrary::MergeMaterials(const TArray<FGenTriangleMesh>& Meshes)
{
	TArray<FGenTriangleMesh> Output;
	TArray<FGenTriangleMesh> Tmp = Meshes;

	int32 Num = Tmp.Num();
	for (int32 Index = 0; Index < Num; Index++)
	{
		// Look ahead for possible merges
		bool Combined = false;
		for (int32 Other = Index + 1; Other < Num; Other++)
		{
			if (Tmp[Index].Material == Tmp[Other].Material)
			{
				Tmp[Other] = CombineMesheSection(Tmp[Index], Tmp[Other]);
				Combined = true;
				break;
			}
		}

		// Only add mesh to list if all merges have been done
		if (!Combined && IsValid(Tmp[Index].Material))
		{
			Output.Emplace(Tmp[Index]);
		}
	}

	return Output;
}

void UProceduralLibrary::PopulateInstancedMeshes(TArray<UInstancedStaticMeshComponent*> Components, const TArray<FTransform>& Transforms)
{
	FRandomStream Random;

	// Randomly assign lists of transforms
	TArray<TArray<FTransform>> Samples;
	const int32 Num = Components.Num();
	Samples.SetNum(Num);

	for (const FTransform& Transform : Transforms)
	{
		const int32 Index = Random.RandRange(0, Num - 1);
		Samples[Index].Emplace(Transform);
	}

	// Assign instances
	for (int32 Index = 0; Index < Num; Index++)
	{
		const int32 Count = Samples[Index].Num();

		UInstancedStaticMeshComponent* Component = Components[Index];
		while (Component->GetNumRenderInstances() > Count)
		{
			Component->RemoveInstance(0);
		}

		while (Component->GetNumRenderInstances() < Count)
		{
			Component->AddInstance(FTransform());
		}

		Component->BatchUpdateInstancesTransforms(0, Samples[Index], false, true, false);
	}
}

void UProceduralLibrary::PopulateInstancedMesh(UInstancedStaticMeshComponent* Component, const TArray<FTransform>& Transforms)
{
	TArray<UInstancedStaticMeshComponent*> Components;
	Components.Emplace(Component);
	PopulateInstancedMeshes(Components, Transforms);
}


void UProceduralLibrary::ApplyToMeshes(UProceduralMeshComponent* ProceduralMesh, const TArray<FGenTriangleMesh>& Meshes, bool EnableCollision)
{
	if (IsValid(ProceduralMesh))
	{
		ProceduralMesh->ClearCollisionConvexMeshes();
		ProceduralMesh->EmptyOverrideMaterials();
		ProceduralMesh->ClearAllMeshSections();

		const int32 MeshNum = Meshes.Num();
		for (int32 Index = 0; Index < MeshNum; Index++)
		{
			const FGenTriangleMesh& Mesh = Meshes[Index];

			bool HasConvex = false;
			for (const FGenConvexMesh& Convex : Mesh.Convex)
			{
				if (Convex.Points.Num() >= 4)
				{
					ProceduralMesh->AddCollisionConvexMesh(Convex.Points);
					HasConvex = true;
				}
			}

			// Convert into mesh
			TArray<int32> Faces;
			for (const FGenTriangle& Triangle : Mesh.Triangulation.Triangles)
			{
				if (Triangle.Enabled)
				{
					Faces.Append({ Triangle.Verts[0], Triangle.Verts[1], Triangle.Verts[2] });
				}
			}

			TArray<FVector> Normals;
			TArray<FVector2D> UVs;
			TArray<FColor> Colors;
			TArray<FProcMeshTangent> Tangents;
			for (const FGenTriangleVertex& Vertex : Mesh.Vertices)
			{
				Normals.Emplace(Vertex.Normal);
				UVs.Emplace(Vertex.UV);
				Colors.Emplace(Vertex.Color);
				Tangents.Emplace(FProcMeshTangent(Vertex.Tangent, false));
			}

			ProceduralMesh->CreateMeshSection(Index, Mesh.Triangulation.Points, Faces, Normals, UVs, Colors, Tangents, EnableCollision && (ProceduralMesh->bUseComplexAsSimpleCollision || HasConvex));

			ProceduralMesh->SetMaterial(Index, Mesh.Material);
		}
	}
}

int32 FProceduralSplineMeshArray::SamplePostMeshIndex(float Angle, FRandomStream& Random) const
{
	float Weight = 0.0f;

	TArray<int32> Indices;

	const int32 Num = PostMeshes.Num();
	for (int32 Index = 0; Index < Num; Index++)
	{
		const FProceduralPostMesh& PostMesh = PostMeshes[Index];
		if (PostMesh.MinAngle <= Angle && Angle <= PostMesh.MaxAngle)
		{
			Weight += PostMeshes[Index].Weight;
			Indices.Emplace(Index);
		}
	}

	float Sample = Random.RandRange(0.0f, Weight);

	const int32 SampleNum = Indices.Num();
	for (int32 Index = 0; Index < SampleNum; Index++)
	{
		const FProceduralPostMesh& PostMesh = PostMeshes[Indices[Index]];
		Sample -= PostMesh.Weight;

		if (Sample <= 0.0f)
		{
			return Indices[Index];
		}
	}
	return INDEX_NONE;
}

int32 FProceduralSplineMeshArray::SampleSplineIndex(const TArray<int32>& Indices, FRandomStream& Random) const
{
	float Weight = 0.0f;

	const int32 Num = Indices.Num();
	for (int32 Index = 0; Index < Num; Index++)
	{
		const FProceduralSplineMesh& SplineMesh = SplineMeshes[Indices[Index]];
		Weight += SplineMesh.Weight;
	}

	float Sample = Random.RandRange(0.0f, Weight);
	for (int32 Index = 0; Index < Num; Index++)
	{
		const FProceduralSplineMesh& SplineMesh = SplineMeshes[Indices[Index]];
		Sample -= SplineMesh.Weight;

		if (Sample < 0.0f)
		{
			return Index;
		}
	}
	return Num - 1;
}

int32 FProceduralSplineMeshArray::SampleSplineMesh(float Distance, bool IsFirst, FRandomStream& Random) const
{
	TArray<int32> StrongCap; // Is a cap and fits
	TArray<int32> WeakCap; // Is a cap but doesn't fit
	TArray<int32> Match; // Is a segment and fits
	TArray<int32> Tiled; // Is a segment and only a little bit too big
	TArray<int32> Outed; // Is a segment but doesn't fit

	const int32 MeshNum = SplineMeshes.Num();
	for (int32 MeshIndex = 0; MeshIndex < MeshNum; MeshIndex++)
	{
		const FProceduralSplineMesh& SplineMesh = SplineMeshes[MeshIndex];

		if ((SplineMesh.CanBeHead && IsFirst) || (SplineMesh.CanBeTail && Distance < SplineMesh.MaxLength))
		{
			if (SplineMesh.MinLength < Distance)
			{
				StrongCap.Emplace(MeshIndex);
			}
			else
			{
				WeakCap.Emplace(MeshIndex);
			}
		}
		else if (SplineMesh.MinLength < Distance && Distance < SplineMesh.MaxLength && SplineMesh.CanBeSegment)
		{
			Match.Emplace(MeshIndex);
		}
		else if (SplineMesh.MinLength < Distance && Distance * 0.75f < SplineMesh.MaxLength && SplineMesh.CanBeSegment)
		{
			Tiled.Emplace(MeshIndex);
		}
		else if(SplineMesh.CanBeSegment)
		{
			Outed.Emplace(MeshIndex);
		}
	}

	if (StrongCap.Num() > 0)
	{
		return SampleSplineIndex(StrongCap, Random);
	}
	else if (WeakCap.Num() > 0)
	{
		return SampleSplineIndex(WeakCap, Random);
	}
	else if (Match.Num() > 0)
	{
		return SampleSplineIndex(Match, Random);
	}
	else if (Tiled.Num() > 0)
	{
		return SampleSplineIndex(Tiled, Random);
	}
	return SampleSplineIndex(Outed, Random);
}

void UProceduralLibrary::ResetSplineMeshes(FProceduralMeshContainer& MeshContainer)
{
	for (USplineMeshComponent* SplineMesh : MeshContainer.SplineMeshes)
	{
		SplineMesh->DestroyComponent();
	}
	for (USplineMeshComponent* HoleMesh : MeshContainer.HoleMeshes)
	{
		HoleMesh->DestroyComponent();
	}
	for (UStaticMeshComponent* PostMesh : MeshContainer.PostMeshes)
	{
		PostMesh->DestroyComponent();
	}
}

template<typename T>
T* CreateMeshToSplineParent(USplineComponent* Spline, const FProceduralStaticMesh& StaticMesh, TArray<T*>& Container, int32& Count)
{
	T* Mesh = nullptr;
	if (Container.IsValidIndex(Count))
	{
		Mesh = Container[Count];
	}
	else
	{
		USceneComponent* Root = Spline->GetOwner()->GetRootComponent();
		Mesh = NewObject<T>(Root);
		Mesh->SetupAttachment(Root);
		Mesh->RegisterComponent();
		Mesh->SetCullDistance(Spline->CachedMaxDrawDistance);
	}
	Count += 1;

	Mesh->SetStaticMesh(StaticMesh.StaticMesh);

	Mesh->EmptyOverrideMaterials();
	const int32 MaterialNum = StaticMesh.Materials.Num();
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialNum; MaterialIndex++)
	{
		Mesh->SetMaterial(MaterialIndex, StaticMesh.Materials[MaterialIndex]);
	}
	return Mesh;
}

void UpdateSplineMesh(USplineComponent* Spline, USplineMeshComponent* Mesh, float Start, float End, float Ratio, const FVector& Offset, const FProceduralStaticMesh& StaticMesh)
{
	const FVector FinalOffset = Offset + StaticMesh.Offset;
	const float FinalStartDistance = Start - FinalOffset.X;
	const float FinalEndDistance = End + FinalOffset.X;

	if (IsValid(Mesh))
	{
		// Update SplineMesh
		Mesh->SetForwardAxis(StaticMesh.Axis, false);

		Mesh->SetStartAndEnd(
			Spline->GetLocationAtDistanceAlongSpline(FinalStartDistance, ESplineCoordinateSpace::Local),
			Spline->GetTangentAtDistanceAlongSpline(FinalStartDistance, ESplineCoordinateSpace::Local) * Ratio,
			Spline->GetLocationAtDistanceAlongSpline(FinalEndDistance, ESplineCoordinateSpace::Local),
			Spline->GetTangentAtDistanceAlongSpline(FinalEndDistance, ESplineCoordinateSpace::Local) * Ratio, false);

		Mesh->SetStartScale(FVector2D(Spline->GetScaleAtDistanceAlongSpline(FinalStartDistance)), false);
		Mesh->SetEndScale(FVector2D(Spline->GetScaleAtDistanceAlongSpline(FinalEndDistance)), false);

		Mesh->SetStartRoll(FMath::DegreesToRadians(Spline->GetRollAtDistanceAlongSpline(FinalStartDistance, ESplineCoordinateSpace::Local)), false);
		Mesh->SetEndRoll(FMath::DegreesToRadians(Spline->GetRollAtDistanceAlongSpline(FinalEndDistance, ESplineCoordinateSpace::Local)), false);

		Mesh->SetStartOffset(FVector2D(FinalOffset.Y, FinalOffset.Z), false);
		Mesh->SetEndOffset(FVector2D(FinalOffset.Y, FinalOffset.Z), false);

		Mesh->UpdateMesh();
	}
}

void UProceduralLibrary::CreateSplineMeshes(USplineComponent* Spline, const TArray<FProceduralSplineMeshArray>& MeshArrays, int32 Seed, FProceduralMeshContainer& MeshContainer)
{
	int32 SplineMeshCount = 0;
	int32 HoleMeshCount = 0;
	int32 PostMeshCount = 0;
	MeshContainer.Holes.Empty();

	if (IsValid(Spline) && MeshArrays.Num() > 0)
	{
		FRandomStream Random(Seed);

		for (const FProceduralSplineMeshArray& MeshArray : MeshArrays)
		{

			auto CreatePost = [&](float Distance, float Left, float Right) {

				const FTransform Transform = Spline->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
				const FVector Before = Spline->GetLocationAtDistanceAlongSpline(Left, ESplineCoordinateSpace::Local);
				const FVector After = Spline->GetLocationAtDistanceAlongSpline(Right, ESplineCoordinateSpace::Local);
				const float Angle = FMath::Acos((After - Transform.GetLocation()).GetSafeNormal() | (Transform.GetLocation() - Before).GetSafeNormal()) * 180.0f / PI;

				const int32 PostMeshIndex = MeshArray.SamplePostMeshIndex(Angle, Random);
				if (MeshArray.PostMeshes.IsValidIndex(PostMeshIndex))
				{

					const FProceduralPostMesh& PostMesh = MeshArray.PostMeshes[PostMeshIndex];
					UStaticMeshComponent* Mesh = CreateMeshToSplineParent<UStaticMeshComponent>(Spline, PostMesh, MeshContainer.PostMeshes, PostMeshCount);
					Mesh->SetRelativeTransform(FTransform(FQuat::Identity, MeshArray.Offset + PostMesh.Offset, FVector::OneVector) * Transform);
				}
			};


			int32 SegmentStart = 0;
			int32 SegmentEnd = 0;

			const int32 SegmentNum = Spline->GetNumberOfSplineSegments();
			while (SegmentEnd <= SegmentNum)
			{
				const bool IsLast = (SegmentEnd == SegmentNum);
				const bool IsHole = MeshArray.Holes.Contains(SegmentEnd);
				const bool IsSegmented = IsHole || IsLast;
				if (MeshArray.PointAligned || IsSegmented)
				{
					if (IsHole)
					{
						const float Start = Spline->GetDistanceAlongSplineAtSplinePoint(SegmentEnd);
						const float Stop = Spline->GetDistanceAlongSplineAtSplinePoint(SegmentEnd+1);
						MeshContainer.Holes.Emplace(Spline->GetTransformAtDistanceAlongSpline((Start + Stop) / 2, ESplineCoordinateSpace::World));

						const FProceduralStaticMesh& SplineMesh = MeshArray.Holes[SegmentEnd];
						USplineMeshComponent* Mesh = CreateMeshToSplineParent<USplineMeshComponent>(Spline, SplineMesh, MeshContainer.HoleMeshes, HoleMeshCount);
						UpdateSplineMesh(Spline, Mesh, Start, Stop, 1.0f, MeshArray.Offset, SplineMesh);
					}

					if (SegmentStart != SegmentEnd)
					{
						TArray<float> Poles;

						const float Start = Spline->GetDistanceAlongSplineAtSplinePoint(SegmentStart);
						const float Stop = Spline->GetDistanceAlongSplineAtSplinePoint(SegmentEnd);
						const float Total = Stop - Start;

						TArray<int32> Collection;

						// Sample meshes to fill spline
						float RestLength = Total;
						float TotalGap = 0.0f;
						while (RestLength > 0.0f)
						{
							const int32 MeshIndex = MeshArray.SampleSplineMesh(RestLength, TotalGap < SMALL_NUMBER, Random);

							if (MeshIndex != INDEX_NONE)
							{
								const FProceduralSplineMesh& SplineMesh = MeshArray.SplineMeshes[MeshIndex];

								TotalGap += SplineMesh.MaxLength - SplineMesh.MinLength;
								RestLength -= SplineMesh.MaxLength;

								Collection.Emplace(MeshIndex);
							}
						}

						// Make sure total gap is not 0 so we can divide
						// The result is 0 either way so it doesn't matter
						TotalGap = FMath::Max(TotalGap, SMALL_NUMBER);
						const float RestGap = -RestLength;
						float StartDistance = Start;

						// Add and stretch meshes according to their gapsize
						const int32 CollectionNum = Collection.Num();
						for (int32 Index = 0; Index < CollectionNum; Index++)
						{
							// Compute location on the spline
							const FProceduralSplineMesh& SplineMesh = MeshArray.SplineMeshes[Collection[Index]];
							const float Length = SplineMesh.MaxLength - RestGap * (SplineMesh.MaxLength - SplineMesh.MinLength) / TotalGap;
							const float EndDistance = StartDistance + Length;

							Poles.Emplace(StartDistance);

							USplineMeshComponent* Mesh = CreateMeshToSplineParent<USplineMeshComponent>(Spline, SplineMesh, MeshContainer.SplineMeshes, SplineMeshCount);
							UpdateSplineMesh(Spline, Mesh, StartDistance, EndDistance, Length / Total, MeshArray.Offset, SplineMesh);

							StartDistance = EndDistance;
						}

						// Only add last post if necessary
						if (!MeshArray.PointAligned || IsHole || (IsLast && !Spline->IsClosedLoop()))
						{
							Poles.Emplace(StartDistance);
						}


						// Create poles
						const int32 PoleNum = Poles.Num();
						if (PoleNum > 1)
						{
							const float FirstPole = Poles[0];
							float LastPole = FirstPole;

							if (MeshArray.PostDistances < SMALL_NUMBER)
							{
								for (int32 PoleIndex = 1; PoleIndex < PoleNum - 1; PoleIndex++)
								{
									const float NextPole = Poles[PoleIndex + 1];
									CreatePost(Poles[PoleIndex], LastPole, NextPole);
									LastPole = Poles[PoleIndex];
								}
							}
							else
							{
								float CurrentPole = LastPole + MeshArray.PostDistances;
								while (CurrentPole < Poles.Last())
								{
									const float NextPole = CurrentPole + MeshArray.PostDistances;
									CreatePost(CurrentPole, LastPole, NextPole);
									LastPole = CurrentPole;
									CurrentPole = NextPole;
								}
							}

							if (Spline->IsClosedLoop())
							{
								CreatePost(0.0f, LastPole, Poles[1]);
							}
							else
							{
								CreatePost(FirstPole, FirstPole, Poles[1]);
								CreatePost(Poles.Last(), LastPole, Poles.Last());
							}
						}
					}

					// Go to next segment
					SegmentStart = SegmentEnd;
					if (IsHole)
					{
						// Skip if hole
						SegmentStart += 1;
					}
				}
				SegmentEnd += 1;
			}
		}
	}

	while (MeshContainer.SplineMeshes.IsValidIndex(SplineMeshCount))
	{
		MeshContainer.SplineMeshes.Pop()->DestroyComponent();
	}

	while (MeshContainer.HoleMeshes.IsValidIndex(HoleMeshCount))
	{
		MeshContainer.HoleMeshes.Pop()->DestroyComponent();
	}

	while (MeshContainer.PostMeshes.IsValidIndex(PostMeshCount))
	{
		MeshContainer.PostMeshes.Pop()->DestroyComponent();
	}
}

