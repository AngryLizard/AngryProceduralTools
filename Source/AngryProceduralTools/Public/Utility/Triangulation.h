// The Gateway of Angry: Planes of Existence. By Salireths.

#pragma once

#include "CoreMinimal.h"
#include "Triangulation.generated.h"

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FTriangleEdge
{
	GENERATED_USTRUCT_BODY()
public:
	FTriangleEdge();
	FTriangleEdge(int32 T, int32 E);
	FTriangleEdge(const FTriangleEdge& Other);

	int32 T;
	int32 E;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FConvex
{
	GENERATED_USTRUCT_BODY()
public:
	FConvex();
	FConvex(int32 A, int32 B);
	
	TArray<int32> Vertices;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FTriangle
{
	GENERATED_USTRUCT_BODY()
public:
	FTriangle();
	FTriangle(int32 A, int32 B, int32 C);
	FTriangle(const FTriangle& Other);

	void ClearAdjs();
	bool HasVertex(int32 Vertex) const;
	void ReplaceAdj(int32 From, int32 To);
	int32 OppositeOf(const FTriangle& Other) const;
	bool IsConnected(const FTriangle& Other) const;

	int32 Verts[3];
	int32 Adjs[3];
	bool Enabled;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FTriangulation
{
	GENERATED_USTRUCT_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "Procedural Mesh")
		TArray<FTriangle> Triangles;

	void Reparent(const TArray<int32>& TriangleIndices);
	void ReparentAll();
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FTriangulation3D : public FTriangulation
{
	GENERATED_USTRUCT_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "Procedural Mesh")
		TArray<FVector> Points;


	void DrawTriangles(UWorld* World, const FTransform& Transform);
	void Circumcenter(int32 Index, FVector& Center, float& Radius) const;
	bool FixTriangles(int32 MaxIterations);
};


USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FTriangulation2D : public FTriangulation
{
	GENERATED_USTRUCT_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "Procedural Mesh")
		TArray<FVector2D> Points;

	void Circumcenter(int32 Index, FVector2D& Center, float& Radius) const;
	bool FixTriangles(int32 MaxIterations);

	FVector ComputeArea(const FTriangle& Triangle) const;
	FVector InsideCheck(const FTriangle& Triangle, const FVector2D& Point) const;
	int32 FindTriangle(const FVector2D& Point) const;

	int32 CutEdge(int32 TriangleIndex, int32 NeighbourIndex, int32 Edge, int32 PointIndex);
	void SetBorders(const FVector2D& Min, const FVector2D& Max);
	void AddPoints(const FVector2D& Point);

	void QHull(TArray<FVector2D> Cloud, int32 Iterations);
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FTriangleVertex
{
	GENERATED_USTRUCT_BODY()
		FTriangleVertex();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector Tangent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FVector2D UV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FColor Color;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FConvexMesh
{
	GENERATED_USTRUCT_BODY()
		FConvexMesh();
	FConvexMesh(const TArray<FVector>& Points);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		TArray<FVector> Points;
};

USTRUCT(BlueprintType)
struct ANGRYPROCEDURALTOOLS_API FTriangleMesh
{
	GENERATED_USTRUCT_BODY()
		FTriangleMesh();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		FTriangulation3D Triangulation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		TArray<FTriangleVertex> Vertices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		TArray<FConvexMesh> Convex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
		UMaterialInterface* Material;
};

