
#include "ChunkActor.h"

#include "ChunkFunctionLibrary.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Field/FieldSystemNoiseAlgo.h"


AChunkActor::AChunkActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));

	InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedStaticMeshComponent"));
	InstancedStaticMeshComponent->SetupAttachment(GetRootComponent());
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
	
	GenerateInstances();
}

void AChunkActor::GenerateInstances()
{
	for (int ID = 0; ID < ChunkData.Num(); ID++)
	{
		if (ChunkData[ID] != EBlockType::Air)
		{
			FVector BlockPosition_Double = UChunkFunctionLibrary::ToVector(GetBlockPosFromID(ID));
			FVector BlockRelativeLocation = BlockPosition_Double * ChunkSetup.BlockSize;
			
			FTransform InstanceTransform;
			InstanceTransform.SetLocation(BlockRelativeLocation);
			InstanceTransform.SetScale3D(ChunkSetup.BlockSize / 100.f);	
			
			auto PrimitiveInstanceId = InstancedStaticMeshComponent->AddInstanceById(InstanceTransform, false);
			VisibleInstances.Add(ID, PrimitiveInstanceId);
		}
	}
}

void AChunkActor::GenerateChunkData()
{	
	ParallelFor(ChunkSetup.ChunkSize.X * ChunkSetup.ChunkSize.Y, [this](int32 ID)
	//for (int32 ID = 0; ID < ChunkSetup.ChunkSize.X * ChunkSetup.ChunkSize.Y; ID++)
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
	);
}

void AChunkActor::UpdateInstancesVisibility(FIntVector ModifiedBlock)
{
	//
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

void AChunkActor::SetBlockType(EBlockType BlockType, FIntVector Pos)
{
	int32 ID = GetBlockIDFromPos(Pos);
	if (ChunkData[ID] == BlockType)
	{
		return;
	}
	ChunkData[ID] = BlockType;
	
	bool bShouldUpdateInstances = BlockType == EBlockType::Air || ChunkData[ID] == EBlockType::Air;
	
	UpdateInstancesVisibility(Pos);
	
	UpdateInstancesVisibility(Pos + FIntVector(1, 0, 0));
	UpdateInstancesVisibility(Pos + FIntVector(-1, 0, 0));
	UpdateInstancesVisibility(Pos + FIntVector(0, 1, 0));
	UpdateInstancesVisibility(Pos + FIntVector(0, -1, 0));
	UpdateInstancesVisibility(Pos + FIntVector(0, 0, 1));
	UpdateInstancesVisibility(Pos + FIntVector(0, 0, -1));
	
}

EBlockType AChunkActor::GetBlockType(FIntVector Pos) const
{
	int32 ID = GetBlockIDFromPos(Pos);
	return ChunkData[ID];
}

bool AChunkActor::IsBlockPosInChunkBounds(FIntVector BlockPos)
{
	return 0 <= BlockPos.X && BlockPos.X < ChunkSetup.ChunkSize.X
		&& 0 <= BlockPos.Y && BlockPos.Y < ChunkSetup.ChunkSize.Y
		&& 0 <= BlockPos.Z && BlockPos.Z < ChunkSetup.ChunkSize.Z;
}

int32 AChunkActor::GetBlockIDFromPos(FIntVector Pos) const
{
	return UChunkFunctionLibrary::GetBlockIDFromPos(ChunkSetup, Pos);
}

FIntVector AChunkActor::GetBlockPosFromID(int32 ID) const
{
	return UChunkFunctionLibrary::GetBlockPosFromID(ChunkSetup, ID);
}

FVector AChunkActor::GetBlockRealPos(const FIntVector& Pos) const
{
	return UChunkFunctionLibrary::GetBlockRealPos(this, Pos);
}

