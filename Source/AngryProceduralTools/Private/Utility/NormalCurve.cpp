#include "Utility/NormalCurve.h"

FNormalCurve::FNormalCurve()
:	X(FVector::ZeroVector), Y(FVector::ZeroVector)
{
}

bool FNormalCurve::operator==(const FNormalCurve& Other) const
{
	return X == Other.X && Y == Other.Y;
}

FVector2D FNormalCurve::Sample(float Time) const
{
	const FVector2D A = FMath::Lerp(FVector2D(X.X, Y.X), FVector2D(X.Y, Y.Y), Time);
	const FVector2D B = FMath::Lerp(FVector2D(X.Y, Y.Y), FVector2D(X.Z, Y.Z), Time);
	return FMath::Lerp(A, B, Time);
}

FUnitCurve::FUnitCurve()
{
	CurveData.AddKey(0.00f, 0.0f);
	CurveData.AddKey(0.25f, 0.0f);
	CurveData.AddKey(0.50f, 0.0f);
	CurveData.AddKey(0.75f, 0.0f);
	CurveData.AddKey(1.00f, 0.0f);
}

bool FUnitCurve::operator==(const FUnitCurve& Other) const
{
	return CurveData == Other.CurveData;
}

float FUnitCurve::Sample(float Time) const
{
	return CurveData.Eval(Time, 0.0f);
}