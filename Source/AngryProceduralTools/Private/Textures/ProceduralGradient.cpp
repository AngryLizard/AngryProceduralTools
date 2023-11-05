#include "Textures/ProceduralGradient.h"
#include "TextureResource.h"
#include "EngineUtils.h"
#include "DeviceProfiles/DeviceProfile.h"

FProceduralGradientResource::FProceduralGradientResource(UProceduralGradient* InOwner)
{
	Owner = InOwner;
	bSRGB = InOwner->SRGB;
}

FProceduralGradientResource::~FProceduralGradientResource()
{
}

uint32 FProceduralGradientResource::GetSizeX() const
{
	if(Owner) return Owner->GetSizeX();
	return 256;
}

uint32 FProceduralGradientResource::GetSizeY() const
{
	if (Owner) return Owner->GetSizeY();
	return 256;
}

void FProceduralGradientResource::InitRHI(FRHICommandListBase& RHICmdList)
{
	FTextureResource::InitRHI(RHICmdList);

	const uint32 Width = GetSizeX();
	const uint32 Height = GetSizeY();

	// Create the RHI texture.
	const ETextureCreateFlags TexCreateFlags = TexCreate_SRGB | TexCreate_OfflineProcessed | TexCreate_Streamable;

	const EPixelFormat EffectiveFormat = Owner->GetPixelFormat();

	const FSamplerStateInitializerRHI SamplerStateInitializer(ESamplerFilter::SF_Bilinear, AM_Mirror, AM_Mirror);
	SamplerStateRHI = RHICreateSamplerState(SamplerStateInitializer);

	// Set the greyscale format flag appropriately.
	bGreyScaleFormat = (EffectiveFormat == PF_G8) || (EffectiveFormat == PF_BC4);
	FRHITextureCreateDesc Desc = FRHITextureCreateDesc::Create2D(TEXT("ProceduralGradientResource"), Width, Height, EffectiveFormat);
	Desc.SetFlags(TexCreateFlags);
	Desc.SetNumMips(1);
	Desc.SetNumSamples(1);
	TextureRHI = RHICreateTexture(Desc);
	TextureRHI->SetName(Owner->GetFName());
	RHIBindDebugLabelName(TextureRHI, *Owner->GetName());
	RHIUpdateTextureReference(Owner->TextureReference.TextureReferenceRHI, TextureRHI);

	uint32 DestPitch;
	void* Data = RHILockTexture2D(TextureRHI, 0, RLM_WriteOnly, DestPitch, false);

	FMemory::Memcpy(Data, Owner->DataCache.GetData(), Owner->DataCache.Num() * sizeof(uint32));

	RHIUnlockTexture2D(TextureRHI, 0, false);
}

void FProceduralGradientResource::ReleaseRHI()
{
	FTextureResource::ReleaseRHI();
}

/** Initializes the texture array resource if needed. */
void FProceduralGradientResource::UpdateResource()
{
	if (IsInitialized())
	{
		ReleaseResource();
	}

	InitResource();
}


////////////////////////////////////////////////////////////////////////////////////////////////////

UProceduralGradient::UProceduralGradient(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	WidthPow2 = 8;
	HeightPow2 = 8;

#if WITH_EDITORONLY_DATA
	SRGB = true;
	MipGenSettings = TMGS_NoMipmaps;
#endif
}

FTextureResource* UProceduralGradient::CreateResource()
{
	return new FProceduralGradientResource(this);
}

ETextureClass UProceduralGradient::GetTextureClass() const
{
	return ETextureClass::Other2DNoSource;
}

void UProceduralGradient::GenerateTexture()
{
	const int32 GradientSizeX = GetSizeX();
	const int32 GradientSizeY = GetSizeY();
	const int32 Num = GradientSizeX * GradientSizeY;

	TArray<FLinearColor> Pixels;
	Pixels.SetNumZeroed(Num);

	GenerateGradient(GradientSizeX, GradientSizeY, Pixels);

	DataCache.SetNumUninitialized(Num);
	for (int32 Index = 0; Index < Num; Index++)
	{
		Pixels[Index].A = 1.0f;
		DataCache[Index] = Pixels[Index].ToFColor(SRGB).ToPackedARGB();
	}
}


FColor GetIterationColor(uint32 Index)
{
	return FColor(
		(30 * Index) & 0xFF,
		(4 * Index) & 0xFF,
		(0xFF - (30 * Index)) & 0xFF,
		0xFF);
}

uint32 Mandelbrot(const FVector2D& c, uint32 MaxIterations, float R)
{

	FVector2D Z = FVector2D::ZeroVector;
	for (uint32 Index = 0; Index < MaxIterations; Index++)
	{
		Z = FVector2D(Z.X * Z.X - Z.Y * Z.Y, 2 * Z.X * Z.Y) + c;

		if (Z.Size() > R) return Index;
	}
	return MaxIterations;
}

void UProceduralGradient::GenerateGradient(int32 GradientSizeX, int32 GradientSizeY, TArray<FLinearColor>& Pixels)
{
	const float MinX = -2.0f;
	const float MaxX = 1.0f;
	const float MinY = -1.5f;
	const float MaxY = 1.5f;
	const uint32 MaxIterations = 64;
	for (int32 X = 0; X < GradientSizeX; X++)
	{
		const float ValX = MinX + (MaxX - MinX) * X / GradientSizeX;
		for (int32 Y = 0; Y < GradientSizeY; Y++)
		{
			const float ValY = MinY + (MaxY - MinY) * Y / GradientSizeY;

			const uint32 Index = Mandelbrot(FVector2D(ValX, ValY), MaxIterations, 2.0f);
			Pixels[GradientSizeY * X + Y] = ((Index == MaxIterations) ? FLinearColor::Black : FLinearColor(GetIterationColor(Index)));
		}
	}
}

void UProceduralGradient::UpdateResource()
{
	GenerateTexture();

	if (PlatformData)
	{
		PlatformData->Mips.Empty();
	}
#if WITH_EDITOR

	const uint32 GradientSizeX = GetSizeX();
	const uint32 GradientSizeY = GetSizeY();
	Source.Init(GradientSizeX, GradientSizeY, 1, 1, ETextureSourceFormat::TSF_BGRA8, (const uint8*)DataCache.GetData());

	// Re-cache platform data if the source has changed.
	CookedPlatformData.Empty();
#endif // #if WITH_EDITOR

	Super::UpdateResource();
}

uint32 UProceduralGradient::CalcTextureMemorySizeEnum(ETextureMipCount Enum) const
{

	return GetSizeX() * GetSizeY() * sizeof(uint32);
}


void UProceduralGradient::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	FStripDataFlags StripFlags(Ar);
	bool bCooked = Ar.IsCooking();
	Ar << bCooked;

	if (bCooked || Ar.IsCooking())
	{
		//SerializeCookedPlatformData(Ar);
	}

#if WITH_EDITOR
	if (Ar.IsLoading() && !Ar.IsTransacting() && !bCooked)
	{
		//BeginCachePlatformData();
	}
#endif
}

void UProceduralGradient::PostLoad()
{
#if WITH_EDITOR
	//FinishCachePlatformData();

#endif // #if WITH_EDITOR
	Super::PostLoad();
};

void UProceduralGradient::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
#if WITH_EDITOR
	int32 GradientSizeX = GetSizeX();
	int32 GradientSizeY = GetSizeY();
#else
	int32 GradientSizeX = 0;
	int32 GradientSizeY = 0;
#endif
	const FString Dimensions = FString::Printf(TEXT("%dx%d"), GradientSizeX, GradientSizeY);
	OutTags.Add(FAssetRegistryTag("Dimensions", Dimensions, FAssetRegistryTag::TT_Dimensional));
	OutTags.Add(FAssetRegistryTag("Format", GPixelFormats[GetPixelFormat()].Name, FAssetRegistryTag::TT_Alphabetical));

	Super::GetAssetRegistryTags(OutTags);
}

FString UProceduralGradient::GetDesc()
{
	return FString::Printf(TEXT("Array: %dx%dy [%s]"),
		GetSizeX(),
		GetSizeY(),
		GPixelFormats[GetPixelFormat()].Name
	);
}

void UProceduralGradient::GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize)
{
	Super::GetResourceSizeEx(CumulativeResourceSize);
	CumulativeResourceSize.AddUnknownMemoryBytes(CalcTextureMemorySizeEnum(TMC_ResidentMips));
}


#if WITH_EDITOR

ANGRYPROCEDURALTOOLS_API void UProceduralGradient::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName PropertyName = PropertyChangedEvent.GetPropertyName();

	if (PowerOfTwoMode == ETexturePowerOfTwoSetting::None && (!Source.IsPowerOfTwo()))
	{
		// Force NPT textures to have no mip maps.
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UProceduralGradient, MipGenSettings))
		{
			UE_LOG(LogTexture, Warning, TEXT("Cannot use mip maps for non-power of two textures."));
		}

		MipGenSettings = TMGS_NoMipmaps;
		NeverStream = true;
	}

	UpdateResource();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif