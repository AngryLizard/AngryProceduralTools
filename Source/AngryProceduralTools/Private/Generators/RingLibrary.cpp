#include "Generators/RingLibrary.h"
#include "ProceduralMeshComponent/Public/ProceduralMeshComponent.h"

FRingShapeParams::FRingShapeParams()
:	Segments(32),
	Radius(1.0f),
	Girth(1.0f),
	Principal(0.0f),
	Residual(0.0f)
{
}

FRingMaterialParams::FRingMaterialParams()
	: ProjectUV(false)
{
}


void URingLibrary::GenerateRing(
	UArrowComponent* Direction,
	const FTransform& Transform,
	FRingShapeParams Shape,
	FRingMaterialParams Material,

	TArray<FTriangleMesh>& Meshes)
{
	if (IsValid(Direction))
	{
		const FTransform Local = Direction->GetComponentTransform();
		const FVector Project = -Direction->GetForwardVector();

		const float Circumference = PI * 2.0f * Shape.Radius;
		const FVector2D Bounds = FVector2D(Circumference, Shape.Girth);
		const FVector2D Boundary = FVector2D(Shape.Radius + Shape.Girth) * 2;

		// Create vertices
		FTriangleMesh TriangleMesh;
		for (int32 Index = 0; Index <= Shape.Segments; Index++)
		{
			const float Ratio = ((float)Index) / Shape.Segments;

			// Polar coordinates
			const float Angle = Ratio * PI * 2.0f;
			const FVector2D C = FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
			const FVector U = FVector(Shape.Principal * C.X * C.X + Shape.Residual * C.Y * C.Y, C.X, C.Y);

			const FVector2D Cd = FVector2D(-FMath::Sin(Angle), FMath::Cos(Angle));
			const FVector Ud = FVector(Shape.Principal * 2.0f * (C.X * Cd.X) + Shape.Residual * 2.0f * (C.Y * Cd.Y), Cd.X, Cd.Y);

			const FVector AP = U * Shape.Radius;
			const FVector BP = U * (Shape.Radius + Shape.Girth);

			TriangleMesh.Triangulation.Points.Emplace(Transform.InverseTransformVectorNoScale(Local.TransformVector(AP)));
			TriangleMesh.Triangulation.Points.Emplace(Transform.InverseTransformVectorNoScale(Local.TransformVector(BP)));

			const FVector Tangent = Ud.GetSafeNormal();
			const FVector Normal = (U ^ Tangent).GetSafeNormal();

			// Inner vertex
			FTriangleVertex AV;
			const FVector VertexProject = Transform.InverseTransformVectorNoScale(Project);
			if (Material.ProjectUV)
			{
				AV.UV = Material.Material.Transform(UProceduralLibrary::ProjectUV(AP, VertexProject, Boundary), Boundary);
			}
			else
			{
				AV.UV = Material.Material.Transform(FVector2D(Ratio, 0.0f), Bounds);
			}

			AV.Tangent = Transform.InverseTransformVectorNoScale(Local.TransformVector(Tangent));
			AV.Normal = Transform.InverseTransformVectorNoScale(Local.TransformVector(Normal));
			AV.Color = Material.Material.VertexColor.ToFColor(false);
			TriangleMesh.Vertices.Emplace(AV);

			// Outer vertex
			FTriangleVertex BV;
			if (Material.ProjectUV)
			{
				BV.UV = Material.Material.Transform(UProceduralLibrary::ProjectUV(BP, VertexProject, Boundary), Boundary);
			}
			else
			{
				BV.UV = Material.Material.Transform(FVector2D(Ratio, 1.0f), Bounds);
			}

			BV.Tangent = AV.Tangent;
			BV.Normal = AV.Normal;
			BV.Color = AV.Color;
			TriangleMesh.Vertices.Emplace(BV);
		}

		// Wrap around
		TriangleMesh.Triangulation.Points.Emplace(TriangleMesh.Triangulation.Points[0]);
		FTriangleVertex AV = TriangleMesh.Vertices[0];
		AV.UV = Material.Material.Transform(FVector2D(1.0f, 0.0f), Bounds);
		TriangleMesh.Vertices.Emplace(AV);

		TriangleMesh.Triangulation.Points.Emplace(TriangleMesh.Triangulation.Points[1]);
		FTriangleVertex BV = TriangleMesh.Vertices[1];
		BV.UV = Material.Material.Transform(FVector2D(1.0f, 1.0f), Bounds);
		TriangleMesh.Vertices.Emplace(BV);

		// Create triangles
		for (int32 Index = 0; Index < Shape.Segments; Index++)
		{
			const int32 I = 2 * (Index);
			const int32 J = 2 * (Index + 1);
			TriangleMesh.Triangulation.Triangles.Emplace(FTriangle(I, J, J + 1));
			TriangleMesh.Triangulation.Triangles.Emplace(FTriangle(I, J + 1, I + 1));
		}

		TriangleMesh.Material = Material.Material.Material;
		Meshes.Emplace(TriangleMesh);
	}
}
