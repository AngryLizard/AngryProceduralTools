#include "Textures/PaintGradient.h"

FGradientFilter::FGradientFilter()
{
}

FLinearColor FGradientFilter::HSVFilter(const FLinearColor& HSV, float Time) const
{
	FLinearColor Out;

	/*
	// Apply "vibrancy" adjustment
	const float VibranceSample = Vibrance;
	if (!FMath::IsNearlyZero(VibranceSample, (float)KINDA_SMALL_NUMBER))
	{
		const float Half = FMath::Clamp(VibranceSample, 0.0f, 1.0f) * 0.5f;
		Sat += Half * UMath::IntPow(1.0f - Sat, 5);
	}
	*/

	const float HueSample = Hue.Sample(Time) * 180.0f;
	Out.R = HSV.R + HueSample;

	const float SaturationSample = Saturation.Sample(Time);
	Out.G = HSV.G * (SaturationSample + 1.0);

	const float ValueSample = Value.Sample(Time);
	Out.B = HSV.B * (ValueSample + 1.0);

	Out.R = FMath::Fmod(Out.R, 360.0f);
	if (Out.R < 0.0f) Out.R += 360.0f;
	Out.G = FMath::Clamp(Out.G, 0.0f, 1.0f);
	Out.B = FMath::Clamp(Out.B, 0.0f, 1.0f);

	Out.A = HSV.A;
	return Out;
}

FLinearColor FGradientFilter::RGBFilter(const FLinearColor& RGB, float Time) const
{
	const float MagentaGreenSample = MagentaGreen.Sample(Time);
	const float TemperatureSample = Temperature.Sample(Time);

	FLinearColor ColorOffset;
	ColorOffset.R = -1.335087f * MagentaGreenSample + -0.907013f * TemperatureSample;
	ColorOffset.G = 0.5070821f * MagentaGreenSample + 0.1607365f * TemperatureSample;
	ColorOffset.B = -1.210441f * MagentaGreenSample + 1.0677497f * TemperatureSample;
	ColorOffset.A = 0.0f;

	const float Dot = (RGB.R * 0.21f + RGB.G * 0.72f + RGB.B * 0.07f);

	FLinearColor Out = RGB + Dot * ColorOffset;
	Out.R = FMath::Clamp(Out.R, 0.0f, 1.0f);
	Out.G = FMath::Clamp(Out.G, 0.0f, 1.0f);
	Out.B = FMath::Clamp(Out.B, 0.0f, 1.0f);
	Out.A = FMath::Clamp(Out.A, 0.0f, 1.0f);
	return Out;
}


////////////////////////////////////////////////////////////////////////////////////////////////////

FGradientSection::FGradientSection()
{
}

FGradientCache::FGradientCache()
:	SizeX(1),
	SizeY(1),
	Height(0),
	Offset(0)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////

UPaintGradient::UPaintGradient(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), CacheDirty(false)
{
}


#if WITH_EDITOR
void UPaintGradient::UpdateCurveSlot(UCurveBase* Curve, EPropertyChangeType::Type ChangeType)
{
	CacheDirty = true;
	UpdateResource();
}
#endif

void UPaintGradient::SampleGradientCurves()
{
	const int32 GradientSizeX = GetSizeX();
	const int32 GradientSizeY = GetSizeY();

	// Clear cache if empty
	const int32 SectionNum = Sections.Num();
	if (PixelCache.Num() != SectionNum)
	{
		PixelCache.Empty();
	}

	// Build cache
	for (int32 Index = 0; Index < SectionNum; Index++)
	{
		const FGradientSection& Section = Sections[Index];
		if (PixelCache.Num() <= Index)
		{
			PixelCache.Emplace(FGradientCache());
		}

		// Check whether cache is valid
		FGradientCache& Cache = PixelCache[Index];
		if (Section.Curves != Cache.Curves || GradientSizeX != Cache.SizeX || GradientSizeY != Cache.SizeY || CacheDirty)
		{

#if WITH_EDITOR
			// Remove previous delegates in case curves got removed
			for (UCurveLinearColor* Curve : Cache.Curves)
			{
				if (Curve != nullptr)
				{
					Curve->OnUpdateCurve.RemoveAll(this);
				}
			}
#endif

			Cache.SizeX = GradientSizeX;
			Cache.SizeY = GradientSizeY;
			Cache.Height = SizeY / SectionNum;
			Cache.Offset = Index * Cache.Height;
			Cache.Curves = Section.Curves;
			Cache.Pixels.SetNum(Cache.Height * GradientSizeX);

#if WITH_EDITOR
			// Make sure we update when curve changes
			for (UCurveLinearColor* Curve : Cache.Curves)
			{
				if (Curve != nullptr)
				{
					Curve->OnUpdateCurve.AddUObject(this, &UPaintGradient::UpdateCurveSlot);
				}
			}
#endif
			
			for (int32 Y = 0; Y < Cache.Height; Y++)
			{
				if (Section.Curves.Num() == 0)
				{
					for (int32 X = 0; X < GradientSizeX; X++)
					{
						Cache.Pixels[GradientSizeX * Y + X] = FLinearColor::White.LinearRGBToHSV();
					}
				}
				else
				{
					const float RatioY = ((float)Y) / Cache.Height;
					const float Ratio = (Section.Curves.Num() - 1) * RatioY;
					const int32 Curve = FMath::FloorToInt((Section.Curves.Num() - 1) * RatioY);

					const UCurveLinearColor* From = Section.Curves[Curve];
					const UCurveLinearColor* To = Section.Curves[FMath::Min(Curve + 1, Section.Curves.Num() - 1)];

					for (int32 X = 0; X < GradientSizeX; X++)
					{
						const float RatioX = ((float)X) / GradientSizeX;
						const FLinearColor FromSample = IsValid(From) ? From->GetClampedLinearColorValue(RatioX) : FLinearColor::White;
						const FLinearColor ToSample = IsValid(To) ? To->GetClampedLinearColorValue(RatioX) : FLinearColor::White;
						const float Alpha = FMath::SmoothStep(0.0f, 1.0f, Ratio - (float)Curve);
						Cache.Pixels[GradientSizeX * Y + X] = FMath::Lerp(FromSample, ToSample, Alpha).LinearRGBToHSV();
					}
				}
			}
		}
	}
	CacheDirty = false;
}

void UPaintGradient::GenerateGradient(int32 GradientSizeX, int32 GradientSizeY, TArray<FLinearColor>& Pixels)
{
	SampleGradientCurves();

	const int32 SectionNum = Sections.Num();
	for (int32 Index = 0; Index < SectionNum; Index++)
	{
		const FGradientSection& Section = Sections[Index];
		const FGradientCache& Cache = PixelCache[Index];

		for (int32 Y = 0; Y < Cache.Height; Y++)
		{
			const float TimeY = ((float)Y) / Cache.Height;

			for (int32 X = 0; X < GradientSizeX; X++)
			{
				const float TimeX = ((float)X) / GradientSizeX;

				FLinearColor HSV = Cache.Pixels[GradientSizeX * Y + X];
				HSV = Section.Horizontal.HSVFilter(HSV, TimeX);
				HSV = Section.Vertical.HSVFilter(HSV, TimeY);

				FLinearColor RGB = HSV.HSVToLinearRGB();
				RGB = Section.Horizontal.RGBFilter(RGB, TimeX);
				RGB = Section.Vertical.RGBFilter(RGB, TimeY);

				Pixels[GradientSizeX * (Cache.Offset + Y) + X] = RGB;
			}
		}
	}
}

