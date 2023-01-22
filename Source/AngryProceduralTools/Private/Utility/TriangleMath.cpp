#include "Utility/TriangleMath.h"

float UTriangleMath::ProjectToBox(const FVector2D& Vector)
{
	const float XX = Vector.X * Vector.X;
	const float YY = Vector.Y * Vector.Y;
	if (XX < SMALL_NUMBER) return FMath::Sqrt(YY);
	if (YY < SMALL_NUMBER) return FMath::Sqrt(XX);

	return FMath::Min(
		FMath::Sqrt((XX / YY) + 1.0f),
		FMath::Sqrt((YY / XX) + 1.0f));
}


bool UTriangleMath::ProjectToSlate(const FVector& Normal, const FVector& A, const FVector& B, const FVector& S, const FVector& T)
{
	const FVector AB = B - A;
	const FVector& Plane = (AB ^ Normal).GetSafeNormal();

	const float ASProject = (A - S) | Plane;
	const float TSProject = (T - S) | Plane;
	if (FMath::Square(TSProject) > SMALL_NUMBER)
	{
		const float Ratio = ASProject / TSProject;
		if (0.0f <= Ratio && Ratio <= 1.0f)
		{
			const float Distance = AB.Size();
			const FVector Direction = AB / Distance;
			const FVector Point = S + (T - S) * Ratio;

			const float Project = (Point - A) | Direction;
			return 0.0f <= Project && Project <= Distance;
		}
	}
	return false;
}

bool UTriangleMath::ComputeCircumcenter(const FVector& A, const FVector& B, const FVector& C, FVector& Out)
{
	const float aa = (B - C).SizeSquared();
	const float bb = (C - A).SizeSquared();
	const float cc = (A - B).SizeSquared();

	const float wa = (aa * (bb + cc - aa));
	const float wb = (bb * (cc + aa - bb));
	const float wc = (cc * (aa + bb - cc));
	const float w = wa + wb + wc;

	if (FMath::Abs(w) < SMALL_NUMBER)
	{
		Out = (A + B + C) / 3;
		return false;
	}

	Out = (wa * A + wb * B + wc * C) / w;
	return true;
}

bool UTriangleMath::ComputeCircumcenter2D(const FVector2D& A, const FVector2D& B, const FVector2D& C, FVector2D& Out)
{
	const float aa = (B - C).SizeSquared();
	const float bb = (C - A).SizeSquared();
	const float cc = (A - B).SizeSquared();

	const float wa = (aa * (bb + cc - aa));
	const float wb = (bb * (cc + aa - bb));
	const float wc = (cc * (aa + bb - cc));
	const float w = wa + wb + wc;

	if (FMath::Abs(w) < SMALL_NUMBER)
	{
		Out = (A + B + C) / 3;
		return false;
	}

	Out = (wa * A + wb * B + wc * C) / w;
	return true;
}