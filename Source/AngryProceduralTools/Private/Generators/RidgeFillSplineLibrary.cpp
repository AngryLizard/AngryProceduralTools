#include "Generators/RidgeFillSplineLibrary.h"
#include "ProceduralMeshComponent/Public/ProceduralMeshComponent.h"

FRidgeSurfaceParams::FRidgeSurfaceParams()
:	UpVector(FVector::UpVector),
	FillerSplines(5),
	FillerSegments(5),
	FillerHeight(100.0f)
{
}

FRidgeMaterialParams::FRidgeMaterialParams()
:	ProjectUV(false),
	UnwrapLane(0.5f)
{
}

void Generate(
	USplineComponent* Left,
	USplineComponent* Right,
	const FTransform& Transform,
	const FRidgeSurfaceParams& Surface,
	const FRidgeMaterialParams& Material,

	const TArray<FRidgeCurvePoint> CurveSamples,
	const TArray<FRidgeSegmentPoint> SegmentSamples,

	TArray<FTriangleMesh>& Meshes)
{
	const int32 CurveNum = CurveSamples.Num();
	const int32 SegmentNum = SegmentSamples.Num();

	const float LeftLength = Left->GetSplineLength();
	const float RightLength = Right->GetSplineLength();
	const float AverageDistance = UProceduralLibrary::GetAveragePointDistance(Left, Right);
	const FVector2D Bounds = FVector2D((LeftLength + RightLength) / 2, AverageDistance);

	const FVector Project = Transform.TransformVectorNoScale(Surface.UpVector).GetSafeNormal();
	const FVector2D Boundary = UProceduralLibrary::ComputeTwinBounds(Left, Right, Project);

	// Go through each curve index
	FTriangleMesh TriangleMesh;
	for (int32 CurveIndex = 0; CurveIndex < CurveNum; CurveIndex++)
	{
		const FRidgeCurvePoint& CurveSample = CurveSamples[CurveIndex];

		// Go through each segment
		for (int32 SegmentIndex = 0; SegmentIndex < SegmentNum; SegmentIndex++)
		{
			const FRidgeSegmentPoint& SegmentSample = SegmentSamples[SegmentIndex];

			// Compute average tangent on the spline
			const FVector FromTangent = Left->GetTangentAtDistanceAlongSpline(SegmentSample.LeftDistance, ESplineCoordinateSpace::World);
			const FVector ToTangent = Right->GetTangentAtDistanceAlongSpline(SegmentSample.RightDistance, ESplineCoordinateSpace::World);
			const FVector Tangent = FMath::Lerp(FromTangent, ToTangent, CurveSample.Ratio).GetSafeNormal();

			// Compute position on the spline
			const FVector From = Left->GetLocationAtDistanceAlongSpline(SegmentSample.LeftDistance, ESplineCoordinateSpace::World);
			const FVector To = Right->GetLocationAtDistanceAlongSpline(SegmentSample.RightDistance, ESplineCoordinateSpace::World);
			const float Distance = (To - From).Size();

			// Set vertex position
			FVector VertexLocation = From;
			FVector VertexNormal = FVector::UpVector;
			if (Distance > SMALL_NUMBER)
			{
				const FVector Normal = (To - From) / Distance;
				const FVector UpVector = (Normal ^ Tangent).GetSafeNormal();
				const FVector Direction = (UpVector ^ Tangent).GetSafeNormal();

				const float Height = Surface.FillerHeight * Transform.GetScale3D().GetMax();
				VertexLocation = FMath::Lerp(From, To, CurveSample.Ratio) + CurveSample.Value * UpVector * Height;
				VertexNormal = (UpVector * Distance + Direction * CurveSample.Slope * Height).GetSafeNormal();
			}

			const FVector VertexPoint = Transform.InverseTransformPosition(VertexLocation);
			TriangleMesh.Triangulation.Points.Emplace(VertexPoint);

			FTriangleVertex Vertex;
			const FVector VertexProject = Transform.InverseTransformVectorNoScale(Project);
			if (Material.ProjectUV)
			{
				Vertex.UV = Material.Material.Transform(UProceduralLibrary::ProjectUV(VertexPoint, VertexProject, Boundary), Boundary);
			}
			else
			{
				// Use position on left spline for unwrapping (Needs to be )
				const float LeftRatio = SegmentSample.LeftDistance / LeftLength;
				const float RightRatio = SegmentSample.RightDistance / RightLength;
				const float U = FMath::Lerp(LeftRatio, RightRatio, Material.UnwrapLane);
				const float V = CurveSample.Ratio;
				Vertex.UV = Material.Material.Transform(FVector2D(U, V), Bounds);
			}

			Vertex.Normal = Transform.InverseTransformVector(VertexNormal);
			Vertex.Tangent = Tangent;
			Vertex.Color = Material.Material.VertexColor.ToFColor(false);
			TriangleMesh.Vertices.Emplace(Vertex);
		}
	}

	// Generate triangles
	for (int32 CurveIndex = 0; CurveIndex < CurveNum - 1; CurveIndex++)
	{
		for (int32 SegmentIndex = 0; SegmentIndex < SegmentNum - 1; SegmentIndex++)
		{
			const int32 A = (CurveIndex + 0) * (SegmentNum)+(SegmentIndex + 0);
			const int32 B = (CurveIndex + 0) * (SegmentNum)+(SegmentIndex + 1);
			const int32 C = (CurveIndex + 1) * (SegmentNum)+(SegmentIndex + 0);
			const int32 D = (CurveIndex + 1) * (SegmentNum)+(SegmentIndex + 1);

			const TArray<FVector>& Vertices = TriangleMesh.Triangulation.Points;
			const float AreaL = ((Vertices[C] - Vertices[A]) ^ (Vertices[C] - Vertices[B])).SizeSquared();
			const float AreaR = ((Vertices[C] - Vertices[B]) ^ (Vertices[C] - Vertices[D])).SizeSquared();
			if (AreaL + AreaR > KINDA_SMALL_NUMBER)
			{
				TriangleMesh.Triangulation.Triangles.Emplace(FTriangle(A, B, C));
				TriangleMesh.Triangulation.Triangles.Emplace(FTriangle(B, D, C));
			}
		}
	}

	TriangleMesh.Material = Material.Material.Material;
	Meshes.Emplace(TriangleMesh);
}


// Sample curve on key points, compute slope
TArray<FRidgeCurvePoint> GetCurveSamples(
	USplineComponent* Left,
	USplineComponent* Right,
	const FRidgeSurfaceParams& Surface)
{
	TArray<FRidgeCurvePoint> Samples;

	const FRichCurve* Filler = Surface.Curve.GetRichCurveConst();
	if (Filler->Keys.Num() <= 1)
	{
		FRidgeCurvePoint Sample;
		Sample.Ratio = 0.0f;
		Sample.Value = Filler->Eval(0);
		Sample.Slope = 0.0f;
		Samples.Emplace(Sample);
	}
	else
	{
		const float Start = Filler->GetFirstKey().Time;
		const float End = Filler->GetLastKey().Time;

		// First handle for base case
		FKeyHandle Key = Filler->GetFirstKeyHandle();
		float Previous = Filler->GetKey(Key).Time;

		// Keep adding splines for each point
		Key = Filler->GetNextKey(Key);
		while (Key != FKeyHandle::Invalid())
		{
			const float Next = Filler->GetKey(Key).Time;

			if (Surface.FillerSplines > 0)
			{
				// Distribute splines linearly between points
				for (int32 Spline = 0; Spline <= Surface.FillerSplines; Spline++)
				{
					const float SplineRatio = ((float)Spline) / Surface.FillerSplines;
					const float Time = FMath::Lerp(Previous, Next, SplineRatio);

					FRidgeCurvePoint Sample;
					Sample.Ratio = (Time - Start) / (End - Start);
					Sample.Value = Filler->Eval(Time);

					// Finite difference for slope (Bias to keep sharp edges sharp)
					const float Bias = FMath::Lerp(1.0f, -1.0f, SplineRatio);
					const float R = Filler->Eval(Time + 1.e-2f + Bias * 1.e-2f);
					const float L = Filler->Eval(Time - 1.e-2f + Bias * 1.e-2f);
					Sample.Slope = (R - L) / 2.e-2f;

					Samples.Emplace(Sample);
				}
			}

			Key = Filler->GetNextKey(Key);
			Previous = Next;
		}
	}

	if (Samples.Num() == 1)
	{
		FRidgeCurvePoint Sample;
		Sample.Ratio = Samples[0].Ratio + 1.0f;
		Sample.Value = Filler->Eval(0);
		Sample.Slope = 0.0f;
		Samples.Emplace(Sample);
	}
	return Samples;
}


// Closeness defined by distance and tangent angle difference
float GetSplineCloseness(
	USplineComponent* Left,
	int32 LeftIndex,
	USplineComponent* Right,
	int32 RightIndex)
{
	const FVector LeftLocation = Left->GetLocationAtSplinePoint(LeftIndex, ESplineCoordinateSpace::World);
	const FVector LeftTangent = Left->GetTangentAtSplinePoint(LeftIndex, ESplineCoordinateSpace::World);

	const FVector RightLocation = Right->GetLocationAtSplinePoint(RightIndex, ESplineCoordinateSpace::World);
	const FVector RightTangent = Right->GetTangentAtSplinePoint(RightIndex, ESplineCoordinateSpace::World);

	const FVector Delta = RightLocation - LeftLocation;
	return FMath::Square(Delta | LeftTangent) + FMath::Square(Delta | RightTangent) + FMath::Abs(1.0f - (LeftTangent | RightTangent)) * Delta.Size();
}


// Match spline points according to closeness
TArray<FRidgeSplinePoint> GetSplineSamplesDynamic(
	USplineComponent* Left,
	USplineComponent* Right,
	const FRidgeSurfaceParams& Surface)
{
	TArray<FRidgeSplinePoint> Samples;

	const int32 LeftNum = Left->GetNumberOfSplineSegments();
	const int32 RightNum = Right->GetNumberOfSplineSegments();

	FRidgeSplinePoint Sample;
	Sample.LeftIndex = 0;
	Sample.RightIndex = 0;
	while (Sample.LeftIndex < LeftNum && Sample.RightIndex < RightNum)
	{
		Samples.Emplace(Sample);

		const FVector LeftLocation = Left->GetLocationAtSplinePoint(Sample.LeftIndex, ESplineCoordinateSpace::World);
		const FVector RightLocation = Right->GetLocationAtSplinePoint(Sample.RightIndex, ESplineCoordinateSpace::World);

		const FVector NextLeftLocation = Left->GetLocationAtSplinePoint(Sample.LeftIndex + 1, ESplineCoordinateSpace::World);
		const FVector NextRightLocation = Right->GetLocationAtSplinePoint(Sample.RightIndex + 1, ESplineCoordinateSpace::World);

		const float NextDist = GetSplineCloseness(Left, Sample.LeftIndex + 1, Right, Sample.RightIndex + 1);
		const float LeftDist = GetSplineCloseness(Left, Sample.LeftIndex + 1, Right, Sample.RightIndex);
		const float RightDist = GetSplineCloseness(Left, Sample.LeftIndex, Right, Sample.RightIndex + 1);

		if (LeftDist < NextDist)
		{
			Sample.LeftIndex++;
		}
		else if (RightDist < NextDist)
		{
			Sample.RightIndex++;
		}
		else
		{
			Sample.LeftIndex++;
			Sample.RightIndex++;
		}
	}

	Samples.Emplace(Sample);

	while (++Sample.LeftIndex <= LeftNum) Samples.Emplace(Sample);
	while (++Sample.RightIndex <= RightNum) Samples.Emplace(Sample);

	return Samples;
}


// Match spline points directly
TArray<FRidgeSplinePoint> GetSplineSamplesMatch(
	USplineComponent* Left,
	USplineComponent* Right,
	const FRidgeSurfaceParams& Surface)
{
	TArray<FRidgeSplinePoint> Samples;

	const int32 LeftNum = Left->GetNumberOfSplineSegments();
	const int32 RightNum = Right->GetNumberOfSplineSegments();

	for (int32 Index = 0; Index <= FMath::Min(LeftNum, RightNum); Index++)
	{
		FRidgeSplinePoint Sample;
		Sample.LeftIndex = Index;
		Sample.RightIndex = Index;
		Samples.Emplace(Sample);
	}

	return Samples;
}

// Match segments to spline points (with inbetweens)
TArray<FRidgeSegmentPoint> GetSegmentSamplesMatch(
	USplineComponent* Left,
	USplineComponent* Right,
	const FRidgeSurfaceParams& Surface,
	const TArray<FRidgeSplinePoint>& Points)
{
	TArray<FRidgeSegmentPoint> Samples;

	const int32 PointNum = Points.Num();
	for (int32 PointIndex = 0; PointIndex < PointNum - 1; PointIndex++)
	{
		const FRidgeSplinePoint& PrevPoint = Points[PointIndex];
		const float LeftStart = Left->GetDistanceAlongSplineAtSplinePoint(PrevPoint.LeftIndex);
		const float RightStart = Right->GetDistanceAlongSplineAtSplinePoint(PrevPoint.RightIndex);

		const FRidgeSplinePoint& NextPoint = Points[PointIndex + 1];
		const float LeftEnd = Left->GetDistanceAlongSplineAtSplinePoint(NextPoint.LeftIndex);
		const float RightEnd = Right->GetDistanceAlongSplineAtSplinePoint(NextPoint.RightIndex);

		if (Surface.FillerSegments > 0)
		{
			for (int32 Segment = 0; Segment <= Surface.FillerSegments; Segment++)
			{
				const float SegmentRatio = ((float)Segment) / Surface.FillerSegments;

				FRidgeSegmentPoint Sample;
				Sample.LeftDistance = FMath::Lerp(LeftStart, LeftEnd, SegmentRatio);
				Sample.RightDistance = FMath::Lerp(RightStart, RightEnd, SegmentRatio);
				Samples.Emplace(Sample);
			}
		}

	}

	return Samples;
}

// Spread points along spline
TArray<FRidgeSegmentPoint> GetSegmentSamplesSpread(
	USplineComponent* Left,
	USplineComponent* Right,
	const FRidgeSurfaceParams& Surface)
{
	TArray<FRidgeSegmentPoint> Samples;

	const int32 LeftNum = Left->GetNumberOfSplineSegments();
	const int32 RightNum = Right->GetNumberOfSplineSegments();

	const int32 Segments = FMath::Max(LeftNum, RightNum) * Surface.FillerSegments;
	for (int32 Segment = 0; Segment <= Segments; Segment++)
	{
		const float SegmentRatio = ((float)Segment) / Segments;

		FRidgeSegmentPoint Sample;
		Sample.LeftDistance = SegmentRatio * Left->GetSplineLength();
		Sample.RightDistance = SegmentRatio * Right->GetSplineLength();
		Samples.Emplace(Sample);
	}

	return Samples;
}



void URidgeFillSplineLibrary::GenerateRidge(
	USplineComponent* Left,
	USplineComponent* Right,
	const FTransform& Transform,
	FRidgeSurfaceParams Surface,
	FRidgeMaterialParams Material,
	ERidgeFillSplineType Type,

	TArray<FTriangleMesh>& Meshes)
{
	if (IsValid(Left) && IsValid(Right))
	{
		const TArray<FRidgeCurvePoint> CurveSamples = GetCurveSamples(Left, Right, Surface);
		if (Type == ERidgeFillSplineType::Spread)
		{
			const TArray<FRidgeSegmentPoint> SegmentSamples = GetSegmentSamplesSpread(Left, Right, Surface);
			Generate(Left, Right, Transform, Surface, Material, CurveSamples, SegmentSamples, Meshes);
		}
		else if (Type == ERidgeFillSplineType::Match)
		{
			const TArray<FRidgeSplinePoint> SplineSamples = GetSplineSamplesMatch(Left, Right, Surface);
			const TArray<FRidgeSegmentPoint> SegmentSamples = GetSegmentSamplesMatch(Left, Right, Surface, SplineSamples);
			Generate(Left, Right, Transform, Surface, Material, CurveSamples, SegmentSamples, Meshes);
		}
		else
		{
			const TArray<FRidgeSplinePoint> SplineSamples = GetSplineSamplesDynamic(Left, Right, Surface);
			const TArray<FRidgeSegmentPoint> SegmentSamples = GetSegmentSamplesMatch(Left, Right, Surface, SplineSamples);
			Generate(Left, Right, Transform, Surface, Material, CurveSamples, SegmentSamples, Meshes);
		}
	}
}

