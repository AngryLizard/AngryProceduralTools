#include "Generators/FillDelaunayLibrary.h"
#include "ProceduralMeshComponent/Public/ProceduralMeshComponent.h"
#include "AngryProceduralTools.h"
#include "Utility/TriangleMath.h"
#include "Math/DiscreteMath.h"

FFillSurfaceParams::FFillSurfaceParams()
:	UpVector(FVector::UpVector),
	CurveThreshold(5.0f),
	Delaunay(true),
	Thickness(0.0f),
	Extrude(FVector::ZeroVector)
{
}

FFillMaterialParams::FFillMaterialParams()
{
}

TArray<float> DivideCurve(USplineComponent* Spline, float Threshold, float Left, float Right, int32 Depth)
{
	TArray<float> Distances;

	if (Depth < 5)
	{
		const float Mid = (Left + Right) / 2;

		const FVector Start = Spline->GetLocationAtDistanceAlongSpline(Left, ESplineCoordinateSpace::World);
		const FVector Middle = Spline->GetLocationAtDistanceAlongSpline(Mid, ESplineCoordinateSpace::World);
		const FVector End = Spline->GetLocationAtDistanceAlongSpline(Right, ESplineCoordinateSpace::World);

		// Check curvature
		const FVector Hypo = (End - Start);
		const FVector Kath = (Middle - Start);
		const float Height = (Hypo ^ Kath).SizeSquared() / Hypo.SizeSquared();
		if (Height > Threshold * Threshold)
		{
			Distances.Append(DivideCurve(Spline, Threshold, Left, Mid, Depth + 1));
			Distances.Emplace(Mid);
			Distances.Append(DivideCurve(Spline, Threshold, Mid, Right, Depth + 1));
		}

	}
	return Distances;
}

bool IntersectsTriangle(const FVector& A, const FVector& B, const FVector& C, const FVector& From, const FVector& To)
{
	const FVector& Normal = (B - A) ^ (C - A);

	if (UTriangleMath::ProjectToSlate(Normal, A, B, From, To))
	{
		return true;
	}

	if (UTriangleMath::ProjectToSlate(Normal, A, C, From, To))
	{
		return true;
	}

	if (UTriangleMath::ProjectToSlate(Normal, B, C, From, To))
	{
		return true;
	}

	return false;
}

bool IsValidTriangle(const FTransform& Transform, USplineComponent* Spline, const TArray<float>& Distances, const TArray<FTriangleVertex>& Vertices, int32 Core, int32 Anchor, int32 Probe, int32 Last)
{
	const FVector CoreLocation = Spline->GetLocationAtDistanceAlongSpline(Distances[Core], ESplineCoordinateSpace::World);
	const FVector AnchorLocation = Spline->GetLocationAtDistanceAlongSpline(Distances[Anchor], ESplineCoordinateSpace::World);
	const FVector ProbeLocation = Spline->GetLocationAtDistanceAlongSpline(Distances[Probe], ESplineCoordinateSpace::World);

	// Check whether triangle goes inwards
	const FVector AnchorNormal = Transform.TransformVectorNoScale(Vertices[Anchor].Normal);
	const FVector CoreNormal = Transform.TransformVectorNoScale(Vertices[Core].Normal);
	const FVector Plane = (AnchorNormal + CoreNormal) ^ AnchorLocation - CoreLocation;
	if (((ProbeLocation - CoreLocation) | Plane) > 0.0f)
	{
		const int32 Num = Distances.Num();

		int32 Collide = Anchor;
		const FVector Location = Spline->GetLocationAtDistanceAlongSpline(Distances[Collide], ESplineCoordinateSpace::World);
		FVector Prev = Location + (Location - CoreLocation).GetClampedToMaxSize(1.0f);
		while (Collide != Last)
		{
			// Check whether any future segments intersect with this triangle
			Collide = (Collide + 1) % Num;
			const FVector Next = Spline->GetLocationAtDistanceAlongSpline(Distances[Collide], ESplineCoordinateSpace::World);

			if (Collide != Probe)
			{
				if (IntersectsTriangle(CoreLocation, AnchorLocation, ProbeLocation, Prev, Next))
				{
					return false;
				}
				Prev = Next;
			}
			else
			{
				Prev = Next + (ProbeLocation - CoreLocation).GetClampedToMaxSize(1.0f);
			}
		}
		return true;
	}
	return false;
}

bool BuildConvex(const FTransform& Transform, USplineComponent* Spline, const TArray<float>& Distances, const TArray<FTriangleVertex>& Vertices, int32 From, int32 To, FTriangulation3D& Triangulation, TArray<FConvex>& Convexes)
{
	const int32 Num = Distances.Num();
	int32 Prev = INDEX_NONE;

	const int32 Core = From;
	int32 Anchor = (Core + 1) % Num;
	FConvex Convex(Core, Anchor);
	while (Anchor != To)
	{
		// Probe for valid triangle to build
		int32 Probe = Anchor;
		for (;;)
		{
			Probe = (Probe + 1) % Num;
			if (IsValidTriangle(Transform, Spline, Distances, Vertices, Core, Anchor, Probe, To))
			{
				break;
			}

			// No convex polygon possible?
			if (Probe == To)
			{
				return false;
			}
		}

		const int32 Old = Triangulation.Triangles.Num();

		// Build concave part we potentially skipped over
		if (!BuildConvex(Transform, Spline, Distances, Vertices, Anchor, Probe, Triangulation, Convexes))
		{
			return false;
		}

		// Create triangle
		FTriangle triangle(Anchor, Core, Probe);
		const int32 Index = Triangulation.Triangles.Num();

		// Update last triangle in the linkage
		if (Triangulation.Triangles.IsValidIndex(Prev))
		{
			Triangulation.Triangles[Prev].Adjs[0] = Index;
			triangle.Adjs[2] = Prev;
		}

		// Update last added triangle
		if (Index > 0 && Old != Index)
		{
			Triangulation.Triangles.Last().Adjs[0] = Index;
			triangle.Adjs[1] = Index - 1;
		}

		// Add triangle
		Prev = Triangulation.Triangles.Emplace(triangle);

		// Anchor for next probe
		Anchor = Probe;
		Convex.Vertices.Emplace(Probe);
	}

	if (Convex.Vertices.Num() > 2)
	{
		Convexes.Emplace(Convex);
	}

	return true;
}



void UFillDelaunayLibrary::GenerateFill(
	USplineComponent* Spline,
	const FTransform& Transform,
	FFillSurfaceParams Surface,
	FFillMaterialParams Material,

	TArray<FTriangleMesh>& Meshes)
{
	if (IsValid(Spline) && Spline->IsClosedLoop())
	{
		TArray<float> Distances;

		// Go through all segments
		const int32 Segments = Spline->GetNumberOfSplineSegments();
		for (int32 Segment = 0; Segment < Segments; Segment++)
		{
			const float Start = Spline->GetDistanceAlongSplineAtSplinePoint(Segment);
			const float End = Spline->GetDistanceAlongSplineAtSplinePoint(Segment + 1);

			Distances.Emplace(Start);
			Distances.Append(DivideCurve(Spline, Surface.CurveThreshold, Start, End, 0));
		}

		const FVector Project = Transform.TransformVectorNoScale(Surface.UpVector).GetSafeNormal();
		const FVector2D Boundary = UProceduralLibrary::ComputeBounds(Spline, Project);

		// Create vertices
		FTriangleMesh TriangleMesh;

		const int32 Num = Distances.Num();
		for (int32 Index = 0; Index < Num; Index++)
		{
			const float Distance = Distances[Index];

			const FVector VertexLocation = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
			const FVector VertexPoint = Transform.InverseTransformPosition(VertexLocation);
			TriangleMesh.Triangulation.Points.Emplace(VertexPoint);

			FTriangleVertex Vertex;
			const FVector Tangent = Spline->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
			Vertex.Tangent = Transform.InverseTransformVector(Tangent);

			/*
			const FVector Before = ProceduralSpline->GetTangentAtDistanceAlongSpline(Distances[(Num + Index - 1) % Num], ESplineCoordinateSpace::World);
			const FVector After = ProceduralSpline->GetTangentAtDistanceAlongSpline(Distances[(Num + Index + 1) % Num], ESplineCoordinateSpace::World);
			const FVector Normal = ((After - Location) ^ (Location - Before)).GetSafeNormal();
			Normals.Emplace(Transform.InverseTransformVector(Normal * FMath::Sign(Normal | UpVector)));
			*/
			const FVector VertexProject = Transform.InverseTransformVectorNoScale(Project);

			Vertex.Normal = VertexProject;
			Vertex.Color = Material.Top.VertexColor.ToFColor(false);
			Vertex.UV = Material.Top.Transform(UProceduralLibrary::ProjectUV(VertexPoint, VertexProject, Boundary), Boundary);

			TriangleMesh.Vertices.Emplace(Vertex);
			/*
			if (DrawDebug)
			{
				const FVector LocalNormal = TriangleMesh.Vertices.Last().Normal;
				DrawDebugDirectionalArrow(GetWorld(), Location, Location + Transform.TransformVector(LocalNormal) * 50.0f, 5.0f, FColor::Red, true);
			}
			*/
		}

		// Triangulate space
		TArray<FConvex> Convexes;
		if (!BuildConvex(Transform, Spline, Distances, TriangleMesh.Vertices, 0, Num - 1, TriangleMesh.Triangulation, Convexes))
		{
			UE_LOG(AngryProceduralTools, Error, TEXT("Failed building convex mesh."));
		}


		// Delaunay triangulation to improve the surface
		if (Surface.Delaunay)
		{
			TriangleMesh.Triangulation.FixTriangles(-1);
		}
		/*
		else if (DrawDebug)
		{
			TriangleMesh.Triangulation.DrawTriangles(GetWorld(), Transform);
		}
		*/

		TriangleMesh.Material = Material.Top.Material;
		Meshes.Emplace(TriangleMesh);

		// Create mesh
		if (Surface.Thickness > SMALL_NUMBER || Surface.Extrude.SizeSquared() > SMALL_NUMBER)
		{
			const float TotalDistance = Spline->GetSplineLength();

			// Mirror mesh
			FTriangleMesh MirrorMesh = TriangleMesh;
			for (int32 Index = 0; Index < Num; Index++)
			{
				MirrorMesh.Vertices[Index].Color = Material.Bottom.VertexColor.ToFColor(false);
				MirrorMesh.Vertices[Index].Normal = -MirrorMesh.Vertices[Index].Normal;
				MirrorMesh.Triangulation.Points[Index] += (MirrorMesh.Vertices[Index].Normal * Surface.Thickness - Surface.Extrude);

				// Remap UVs
				const FVector2D UV = Material.Top.Inverse(MirrorMesh.Vertices[Index].UV, Boundary);
				MirrorMesh.Vertices[Index].UV = Material.Bottom.Transform(UV, Boundary);
			}

			for (FTriangle& Triangle : MirrorMesh.Triangulation.Triangles)
			{
				// std::swap(Triangle.Verts[0], Triangle.Verts[1]);
				const int32 C0 = Triangle.Verts[0];
				Triangle.Verts[0] = Triangle.Verts[1];
				Triangle.Verts[1] = C0;

				// std::swap(Triangle.Adjs[0], Triangle.Adjs[1]);
				const int32 C1 = Triangle.Adjs[0];
				Triangle.Adjs[0] = Triangle.Adjs[1];
				Triangle.Adjs[1] = C1;
			}
			MirrorMesh.Material = Material.Bottom.Material;
			Meshes.Emplace(MirrorMesh);

			// Border mesh
			FTriangleMesh BorderMesh;
			for (int32 Index = 0; Index < Num; Index++)
			{

				const int32 A = (Index + 0) % Num;
				const int32 B = (Index + 1) % Num;

				FTriangleVertex AV = TriangleMesh.Vertices[A];
				FVector AP = TriangleMesh.Triangulation.Points[A];

				FTriangleVertex BV = TriangleMesh.Vertices[B];
				FVector BP = TriangleMesh.Triangulation.Points[B];

				FTriangleVertex CV = MirrorMesh.Vertices[A];
				FVector CP = MirrorMesh.Triangulation.Points[A];

				FTriangleVertex DV = MirrorMesh.Vertices[B];
				FVector DP = MirrorMesh.Triangulation.Points[B];

				// Compute normals from rim direction and tangent
				AV.Normal = (TriangleMesh.Vertices[A].Tangent ^ (AP - CP)).GetSafeNormal();
				CV.Normal = (TriangleMesh.Vertices[A].Tangent ^ (AP - CP)).GetSafeNormal();

				BV.Normal = (MirrorMesh.Vertices[B].Tangent ^ (BP - DP)).GetSafeNormal();
				DV.Normal = (MirrorMesh.Vertices[B].Tangent ^ (BP - DP)).GetSafeNormal();

				// Set UVs (Make sure UVs wrap around)
				const float AU = (Distances[A] / TotalDistance);
				const FVector2D ABounds = FVector2D(TotalDistance, (AP - CP).Size());
				AV.UV = Material.Rim.Transform(FVector2D(AU, 0.0f), ABounds);
				CV.UV = Material.Rim.Transform(FVector2D(AU, 1.0f), ABounds);

				AV.Color = Material.Rim.VertexColor.ToFColor(false);
				CV.Color = Material.Rim.VertexColor.ToFColor(false);

				const float BU = ((A == Num - 1) ? 1.0f : (Distances[B] / TotalDistance));
				const FVector2D BBounds = FVector2D(TotalDistance, (BP - DP).Size());
				BV.UV = Material.Rim.Transform(FVector2D(BU, 0.0f), BBounds);
				DV.UV = Material.Rim.Transform(FVector2D(BU, 1.0f), BBounds);

				BV.Color = Material.Rim.VertexColor.ToFColor(false);
				DV.Color = Material.Rim.VertexColor.ToFColor(false);

				// Add to mesh
				const int32 AI = BorderMesh.Triangulation.Points.Emplace(AP);
				BorderMesh.Vertices.Emplace(AV);
				const int32 BI = BorderMesh.Triangulation.Points.Emplace(BP);
				BorderMesh.Vertices.Emplace(BV);
				const int32 CI = BorderMesh.Triangulation.Points.Emplace(CP);
				BorderMesh.Vertices.Emplace(CV);
				const int32 DI = BorderMesh.Triangulation.Points.Emplace(DP);
				BorderMesh.Vertices.Emplace(DV);

				BorderMesh.Triangulation.Triangles.Emplace(FTriangle(AI, BI, CI));
				BorderMesh.Triangulation.Triangles.Emplace(FTriangle(BI, DI, CI));
			}

			// Build collision
			for (const FConvex& Convex : Convexes)
			{
				TArray<FVector> CollisionPoints;
				for (const int32 Vertex : Convex.Vertices)
				{
					CollisionPoints.Emplace(MirrorMesh.Triangulation.Points[Vertex]);
					CollisionPoints.Emplace(BorderMesh.Triangulation.Points[Vertex]);
				}
				if (CollisionPoints.Num() >= 6)
				{
					BorderMesh.Convex.Emplace(CollisionPoints);
				}
			}

			BorderMesh.Material = Material.Rim.Material;
			Meshes.Emplace(BorderMesh);
		}
	}
}
