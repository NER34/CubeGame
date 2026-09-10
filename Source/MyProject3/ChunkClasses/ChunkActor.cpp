
#include "ChunkActor.h"

#include "Field/FieldSystemNoiseAlgo.h"


AChunkActor::AChunkActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	
}

void AChunkActor::BeginPlay()
{
	Super::BeginPlay();
}

void AChunkActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChunkActor::Initialize_Implementation(const FChunkSetup& InChunkSetup)
{
	ChunkSetup = InChunkSetup;
	ChunkData.SetNum(ChunkSetup.ChunkSize.X * ChunkSetup.ChunkSize.Y * ChunkSetup.ChunkSize.Z);
	
	NoiseGenerator.SetSeed(ChunkSetup.RandomSeed);
	NoiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	NoiseGenerator.SetFrequency(0.01f);
	NoiseGenerator.SetFractalType(FastNoiseLite::FractalType_FBm);
	NoiseGenerator.SetFractalOctaves(6);
	NoiseGenerator.SetFractalLacunarity(1.94f);
	NoiseGenerator.SetFractalGain(0.46f);
	NoiseGenerator.SetFractalWeightedStrength(0.38f);
	
	GenerateChunkData();
}

void AChunkActor::GenerateChunkData()
{	
	//ParallelFor(ChunkSetup.ChunkSize.X * ChunkSetup.ChunkSize.Y, [this](int32 ID)
	for (int32 ID = 0; ID < ChunkSetup.ChunkSize.X * ChunkSetup.ChunkSize.Y; ID++)
	{
		int32 X = ID % ChunkSetup.ChunkSize.X;
		int32 Y = ID / ChunkSetup.ChunkSize.X;
		FVector ChunkLocation = GetActorLocation();
		
		double XLocation = X * ChunkSetup.BlockSize.X + ChunkLocation.X;
		double YLocation = Y * ChunkSetup.BlockSize.Y + ChunkLocation.Y;
		
		//float ZHeight = FMath::PerlinNoise2D({XLocation, YLocation});
		float ZHeight = NoiseGenerator.GetNoise(XLocation, YLocation);
		// (-1.0, 1.0) -> (0.0, 1.0)
		ZHeight = (ZHeight + 1.0f) / 2.0f;
		
		for (int32 Z = 0; Z < ChunkSetup.ChunkSize.Z; Z++)
		{
			float CurrentZHeight = (float)Z / ChunkSetup.ChunkSize.Z;
			ChunkData[GetBlockIDFromPos({X, Y, Z})] = GetBlockTypeByHeight(CurrentZHeight / ZHeight);
		}
	}
	//);
}

EBlockType AChunkActor::GetBlockTypeByHeight(float Height)
{
	if (Height <= 0.33f) { return EBlockType::Stone; }
	if (Height <= 0.66f) { return EBlockType::Grass; }
	if (Height <= 1.0f) { return EBlockType::Snow; }
	return EBlockType::Air;
}

const TArray<EBlockType>& AChunkActor::GetChunkData() const
{
	return ChunkData;
}

const FChunkSetup& AChunkActor::GetChunkSetup() const
{
	return ChunkSetup;
}

int32 AChunkActor::GetBlockIDFromPos(FIntVector Pos) const
{
	return Pos.Z 
		+ ChunkSetup.ChunkSize.Z * Pos.X 
		+ ChunkSetup.ChunkSize.Z * ChunkSetup.ChunkSize.X * Pos.Y;
}

FIntVector AChunkActor::GetBlockPosFromID(int32 ID) const
{
	int32 Z = ID % ChunkSetup.ChunkSize.Z;
	int32 XY = ID / ChunkSetup.ChunkSize.Z;
	int32 X = XY % ChunkSetup.ChunkSize.X;
	int32 Y = XY / ChunkSetup.ChunkSize.X;
	return {X, Y, Z};
}

FVector AChunkActor::GetBlockRealPos(const FIntVector& Pos) const
{
	FVector ChunkLocation = GetActorLocation();
	return {
		Pos.X * ChunkSetup.BlockSize.X + ChunkLocation.X,
		Pos.Y * ChunkSetup.BlockSize.Y + ChunkLocation.Y,
		Pos.Z * ChunkSetup.BlockSize.Z + ChunkLocation.Z,
	};
}

