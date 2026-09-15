
#include "ChunkActor.h"

#include "ChunkHelperFunctions.h"
#include "ChunkGeneratorSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"


AChunkActor::AChunkActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));

	InstancedStaticMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedStaticMeshComponent"));
	InstancedStaticMeshComponent->SetupAttachment(GetRootComponent());
	InstancedStaticMeshComponent->NumCustomDataFloats = 1;
}

void AChunkActor::Initialize(const FChunkSetup& InChunkSetup, const FIntVector& InChunkPos)
{
	ChunkPos = InChunkPos;
	ChunkSetup = InChunkSetup;

	InstancedStaticMeshComponent->ClearInstances();
	VisibleInstances.Reset();
	ChunkData.Reset();
	ChunkData.SetNum(ChunkSetup.ChunkSize.X * ChunkSetup.ChunkSize.Y * ChunkSetup.ChunkSize.Z);
	
	NoiseGenerator.SetSeed(ChunkSetup.RandomSeed);
	NoiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
	NoiseGenerator.SetFrequency(0.01f);
	NoiseGenerator.SetFractalType(FastNoiseLite::FractalType_FBm);
	NoiseGenerator.SetFractalOctaves(6);
	NoiseGenerator.SetFractalLacunarity(1.94f);
	NoiseGenerator.SetFractalGain(0.46f);
	NoiseGenerator.SetFractalWeightedStrength(0.38f);
	
	FVector ChunkLocation = UChunkHelperFunctions::CalculateChunkRealPosition(ChunkSetup, ChunkPos);
	GenerateChunkData(ChunkLocation);
	GenerateInstances();
	
	SetActorLocation(ChunkLocation);
	SetActorHiddenInGame(false);
	//SetActorEnableCollision(true);
	SetActorTickEnabled(true);
	
	OnInitialize(ChunkSetup, ChunkPos);
}

void AChunkActor::Deinitialize()
{
	SetActorHiddenInGame(true);
	//SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	
	OnDeinitialize();
}

void AChunkActor::CreateInstance(int32 BlockID)
{
	FVector BlockPosition_Double = UChunkHelperFunctions::ToVector(GetBlockPosFromID(BlockID));
	FVector BlockRelativeLocation = BlockPosition_Double * ChunkSetup.BlockSize;
			
	FTransform InstanceTransform;
	InstanceTransform.SetLocation(BlockRelativeLocation);
	InstanceTransform.SetScale3D(ChunkSetup.BlockSize / 100.f);	

	FBlockInstanceData InstanceData;
	InstanceData.InstanceId = InstancedStaticMeshComponent->AddInstanceById(InstanceTransform, false);
	InstanceData.BlockType = ChunkData[BlockID];
	
	VisibleInstances.Add(BlockID, InstanceData);
}

void AChunkActor::ActualizeInstanceData(const FBlockInstanceData& InstanceData) const
{	
	float EncodedInstanceData = UChunkHelperFunctions::EncodeBlockInstanceData(InstanceData);
	InstancedStaticMeshComponent->SetCustomDataValueById(
		InstanceData.InstanceId, 0, EncodedInstanceData
		);
}

void AChunkActor::GenerateInstances()
{
	for (int ID = 0; ID < ChunkData.Num(); ID++)
	{
		if (ChunkData[ID] != EBlockType::Air)
		{
			CreateInstance(ID);
			ActualizeInstanceData(VisibleInstances[ID]);
		}
	}
}

void AChunkActor::GenerateChunkData(const FVector& ChunkLocation)
{	
	ParallelFor(ChunkSetup.ChunkSize.X * ChunkSetup.ChunkSize.Y, [this, ChunkLocation](int32 ID)
	{
		int32 X = ID % ChunkSetup.ChunkSize.X;
		int32 Y = ID / ChunkSetup.ChunkSize.X;
		
		double XLocation = X / 100.0 * ChunkSetup.BlockSize.X + ChunkLocation.X;
		double YLocation = Y / 100.0 * ChunkSetup.BlockSize.Y + ChunkLocation.Y;
		
		float ZHeight = NoiseGenerator.GetNoise(XLocation, YLocation);
		// (-1.0, 1.0) -> (0.0, 1.0)
		ZHeight = (ZHeight + 1.0f) / 2.0f;
		// To avoid division by zero
		ZHeight += 0.01f;
		
		for (int32 Z = 0; Z < ChunkSetup.ChunkSize.Z; Z++)
		{
			float CurrentZHeight = (float)Z / ChunkSetup.ChunkSize.Z;
			ChunkData[GetBlockIDFromPos({X, Y, Z})] = GetBlockTypeByHeight(CurrentZHeight / ZHeight);
		}
	}
	);
}

void AChunkActor::ActualizeModifiedInstance(const FIntVector& ModifiedBlock)
{
	auto BlockID = GetBlockIDFromPos(ModifiedBlock);
	if (ChunkData[BlockID] == EBlockType::Air)
	{
		if (auto* InstanceData = VisibleInstances.Find(BlockID))
		{
			InstancedStaticMeshComponent->RemoveInstanceById(InstanceData->InstanceId);
			VisibleInstances.Remove(BlockID);
		}
	}
	else
	{
		if (!VisibleInstances.Contains(BlockID))
		{
			CreateInstance(BlockID);
		}
		else
		{
			VisibleInstances[BlockID].BlockType = ChunkData[BlockID];
		}
		ActualizeInstanceData(VisibleInstances[BlockID]);
	}		
}

void AChunkActor::UpdateNeighborInstancesVisibility(const FIntVector& ModifiedBlock, const FIntVector& Delta)
{
	// TODO
	/*FIntVector NeighborBlock = ModifiedBlock + Delta;
	if (IsBlockPosInChunkBounds(NeighborBlock))
	{
		ActualizeModifiedInstance(NeighborBlock);
	}
	else
	{
		if (auto* ChunkGenerator = GetGameInstance()->GetSubsystem<UChunkGeneratorSubsystem>())
			if (AChunkActor* NeighborChunk = ChunkGenerator->GetChunkActor(ChunkPos + Delta))
			{
				NeighborBlock += ChunkSetup.ChunkSize;
				NeighborBlock.X %= ChunkSetup.ChunkSize.X;
				NeighborBlock.Y %= ChunkSetup.ChunkSize.Y;
				NeighborBlock.Z %= ChunkSetup.ChunkSize.Z;
				NeighborChunk->ActualizeModifiedInstance(NeighborBlock);
			}		
	}*/
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

void AChunkActor::SetBlockType(EBlockType BlockType, const FIntVector& Pos)
{
	if (!IsBlockPosInChunkBounds(Pos))
	{
		return;
	}
	
	int32 ID = GetBlockIDFromPos(Pos);
	if (ChunkData[ID] == BlockType)
	{
		return;
	}
	ChunkData[ID] = BlockType;
	
	ActualizeModifiedInstance(Pos);
	
	//UpdateNeighborInstancesVisibility(Pos, FIntVector(1, 0, 0));
	//UpdateNeighborInstancesVisibility(Pos, FIntVector(-1, 0, 0));
	//UpdateNeighborInstancesVisibility(Pos, FIntVector(0, 1, 0));
	//UpdateNeighborInstancesVisibility(Pos, FIntVector(0, -1, 0));
	//UpdateNeighborInstancesVisibility(Pos, FIntVector(0, 0, 1));
	//UpdateNeighborInstancesVisibility(Pos, FIntVector(0, 0, -1));
	
}

EBlockType AChunkActor::GetBlockType(const FIntVector& Pos) const
{
	if (!IsBlockPosInChunkBounds(Pos))
	{
		return EBlockType::None;
	}
	
	int32 ID = GetBlockIDFromPos(Pos);
	return ChunkData[ID];
}

bool AChunkActor::IsBlockPosInChunkBounds(const FIntVector& BlockPos) const
{
	return 0 <= BlockPos.X && BlockPos.X < ChunkSetup.ChunkSize.X
		&& 0 <= BlockPos.Y && BlockPos.Y < ChunkSetup.ChunkSize.Y
		&& 0 <= BlockPos.Z && BlockPos.Z < ChunkSetup.ChunkSize.Z;
}

void AChunkActor::SetBlockDestructionAlpha(const FIntVector& BlockPos, float Alpha)
{
	if (!IsBlockPosInChunkBounds(BlockPos))
	{
		return;
	}
	
	int32 BlockID = GetBlockIDFromPos(BlockPos);
	if (auto* InstanceData = VisibleInstances.Find(BlockID))
	{
		InstanceData->DestructionAlpha = Alpha;
		ActualizeInstanceData(*InstanceData);
	}
}

void AChunkActor::SetBlockHighlightFlag(const FIntVector& BlockPos, bool bHighlight)
{
	if (!IsBlockPosInChunkBounds(BlockPos))
	{
		return;
	}
	
	int32 BlockID = GetBlockIDFromPos(BlockPos);
	if (auto* InstanceData = VisibleInstances.Find(BlockID))
	{
		InstanceData->bHighlighted = bHighlight;
		ActualizeInstanceData(*InstanceData);
	}
}

int32 AChunkActor::GetBlockIDFromPos(const FIntVector& Pos) const
{
	return UChunkHelperFunctions::GetBlockIDFromPos(ChunkSetup, Pos);
}

FIntVector AChunkActor::GetBlockPosFromID(int32 ID) const
{
	return UChunkHelperFunctions::GetBlockPosFromID(ChunkSetup, ID);
}

FVector AChunkActor::GetBlockRealPos(const FIntVector& Pos) const
{
	return UChunkHelperFunctions::GetBlockRealPos(this, Pos);
}

