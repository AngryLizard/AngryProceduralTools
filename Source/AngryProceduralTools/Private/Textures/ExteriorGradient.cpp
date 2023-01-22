#include "Textures/ExteriorGradient.h"

FExteriorColumn::FExteriorColumn()
{
	Color.X = FVector(0.0f, 0.5f, 1.0f);
	Color.Y = FVector(0.5f, 0.5f, 0.5f);

	Roughness.X = FVector(0.75f, 0.75f, 0.75f);
	Roughness.Y = FVector(1.0f, 1.0f, 1.0f);

	Meglow.X = FVector(0.0f, 0.0f, 0.0f);
	Meglow.Y = FVector(0.0f, 0.0f, 0.0f);
}

FExteriorRow::FExteriorRow()
	: Comment("")
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

UExteriorGradient::UExteriorGradient(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	WidthPow2 = 8;
	HeightPow2 = 6;
	RowThickness = 2;
}

void UExteriorGradient::GenerateGradient(int32 GradientSizeX, int32 GradientSizeY, TArray<FLinearColor>& Pixels)
{
	const int32 Cols = 4;
	const int32 Split = 2;

	const int32 Height = Split * 2;
	const int32 RowNum = FMath::Min(Rows.Num(), GradientSizeY / Height);
	for (int32 RowIndex = 0; RowIndex < RowNum; RowIndex++)
	{
		const FExteriorRow& Row = Rows[RowIndex];

		const int32 Width = GradientSizeX / Cols;
		const int32 ColNum = Cols;
		for (int32 ColIndex = 0; ColIndex < ColNum; ColIndex++)
		{
			const FExteriorColumn& Column = Row.Columns[ColIndex];

			for (int32 X = 0; X < Width; X++)
			{
				const float Time = ((float)X) / Width;
				const FVector2D ColorSample = Column.Color.Sample(Time);
				const FVector2D RoughnessSample = Column.Roughness.Sample(Time);
				const FVector2D MeglowSample = Column.Meglow.Sample(Time);

				const int32 PixX = ColIndex* Width + X;
				for (int32 Y = 0; Y < Split; Y++)
				{
					const int32 PixY_L = RowIndex * Height + Y;
					const int32 PixI_L = PixY_L * GradientSizeX + PixX;
					Pixels[PixI_L].R = ColorSample.X;
					Pixels[PixI_L].G = RoughnessSample.X;
					Pixels[PixI_L].B = MeglowSample.X;

					const int32 PixY_R = PixY_L + Split;
					const int32 PixI_R = PixY_R * GradientSizeX + PixX;
					Pixels[PixI_R].R = ColorSample.Y;
					Pixels[PixI_R].G = RoughnessSample.Y;
					Pixels[PixI_R].B = MeglowSample.Y;
				}
			}
		}
	}
}
