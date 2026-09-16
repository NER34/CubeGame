
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
	InstancedStaticMeshComponent->bNavigationRelevant = 0;
}

void AChunkActor::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		ChunkGeneratorSubsystem = GameInstance->GetSubsystem<UChunkGeneratorSubsystem>();
	}
}

void AChunkActor::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	
	if (!ChunkGeneratorSubsystem)
	{
		return;
	}
	
	if (!bInstancesInitialized)
	{
		bool bNeighborsInitialized = true;
		bNeighborsInitialized &= IsValid(ChunkGeneratorSubsystem->GetChunkActor(ChunkPos + FIntVector{1,0,0}));
		bNeighborsInitialized &= IsValid(ChunkGeneratorSubsystem->GetChunkActor(ChunkPos + FIntVector{-1,0,0}));
		bNeighborsInitialized &= IsValid(ChunkGeneratorSubsystem->GetChunkActor(ChunkPos + FIntVector{0,1,0}));
		bNeighborsInitialized &= IsValid(ChunkGeneratorSubsystem->GetChunkActor(ChunkPos + FIntVector{0,-1,0}));
		
		if (bNeighborsInitialized)
		{
			bInstancesInitialized = true;
			InitializeInstances();
			SetActorHiddenInGame(false);
		}
	}
}

void AChunkActor::Initialize(const FChunkSetup& InChunkSetup, const FIntVector& InChunkPos)
{
	ChunkPos = InChunkPos;
	ChunkSetup = InChunkSetup;
	
	bInstancesInitialized = false;
	InstancedStaticMeshComponent->ClearInstances();
	VisibleInstances.Reset();
	ChunkData.Reset();
	ChunkData.SetNum(ChunkSetup.ChunkSize.X * ChunkSetup.ChunkSize.Y * ChunkSetup.ChunkSize.Z);
	
	FVector ChunkLocation = UChunkHelperFunctions::CalculateChunkRealPosition(ChunkSetup, ChunkPos);
	SetActorLocation(ChunkLocation);
	SetActorTickEnabled(true);
	
	InitializeChunkData();
}

void AChunkActor::Deinitialize()
{
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
}

void AChunkActor::InitializeChunkData()
{	
	ParallelFor(ChunkSetup.ChunkSize.X * ChunkSetup.ChunkSize.Y, 
		[this](int32 ID)
		{
			int32 X = ID % ChunkSetup.ChunkSize.X;
			int32 Y = ID / ChunkSetup.ChunkSize.X;
			
			for (int32 Z = 0; Z < ChunkSetup.ChunkSize.Z; Z++)
			{
				FIntVector BlockPos = {X, Y, Z};
				int32 BlockID = GetBlockIDFromPos({X, Y, Z});
				ChunkData[BlockID] = ChunkGeneratorSubsystem->GenerateBlockType(ChunkPos, BlockPos);
			}
		}
	);
}

void AChunkActor::CreateInstance(int32 BlockID)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AChunkActor::CreateInstance"));
	FVector BlockPosition_Double = UChunkHelperFunctions::ToVector(GetBlockPosFromID(BlockID));
	FVector BlockRelativeLocation = BlockPosition_Double * ChunkSetup.BlockSize;
			
	FTransform InstanceTransform;
	InstanceTransform.SetLocation(BlockRelativeLocation);
	InstanceTransform.SetScale3D(UChunkHelperFunctions::GetBlockScale(ChunkSetup));

	FBlockInstanceData InstanceData;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AddInstanceById"));
		InstanceData.InstanceId = InstancedStaticMeshComponent->AddInstancesById(
			{InstanceTransform}, false, false
			)[0];
	}
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

void AChunkActor::InitializeInstances()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AChunkActor::GenerateInstances"));
	TArray<int32> VisibleBlockIds;
	TArray<FTransform> InstanceTransforms;

	const int32 BlockCount = ChunkData.Num();
	VisibleBlockIds.Reserve(BlockCount);
	InstanceTransforms.Reserve(BlockCount);
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("CollectVisibleInstances"));
		for (int32 ID = 0; ID < BlockCount; ++ID)
		{
			if (!IsBlockVisible(GetBlockPosFromID(ID)))
			{
				continue;
			}

			const FVector BlockPosition = UChunkHelperFunctions::ToVector(GetBlockPosFromID(ID));
			const FVector BlockRelativeLocation = BlockPosition * ChunkSetup.BlockSize;

			FTransform InstanceTransform;
			InstanceTransform.SetLocation(BlockRelativeLocation);
			InstanceTransform.SetScale3D(UChunkHelperFunctions::GetBlockScale(ChunkSetup));

			VisibleBlockIds.Add(ID);
			InstanceTransforms.Add(InstanceTransform);
		}
	}
	
	if (InstanceTransforms.IsEmpty())
	{
		return;
	}
	
	TArray<FPrimitiveInstanceId> InstanceIds;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AddInstancesById"));
		InstanceIds = InstancedStaticMeshComponent->AddInstancesById(
			MakeArrayView(InstanceTransforms),
			false, 
			false
		);
	}
	
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("InstanceDataActualization"));
		for (int32 Index = 0; Index < InstanceIds.Num(); ++Index)
		{
			const int32 BlockID = VisibleBlockIds[Index];

			FBlockInstanceData InstanceData;
			InstanceData.InstanceId = InstanceIds[Index];
			InstanceData.BlockType = ChunkData[BlockID];

			VisibleInstances.Add(BlockID, InstanceData);
			ActualizeInstanceData(InstanceData);
		}
	}
}


void AChunkActor::ActualizeModifiedInstance(const FIntVector& ModifiedBlock)
{
	auto BlockID = GetBlockIDFromPos(ModifiedBlock);
	if (!IsBlockVisible(ModifiedBlock))
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

void AChunkActor::UpdateNeighborInstancesVisibility(FIntVector NeighborBlockPos)
{
	if (IsBlockPosInChunkBounds(NeighborBlockPos))
	{
		ActualizeModifiedInstance(NeighborBlockPos);
	}
	else
	{
		if (NeighborBlockPos.Z < 0 || NeighborBlockPos.Z >= ChunkSetup.ChunkSize.Z)
		{
			return;
		}
		
		FIntVector NeighborChunkPos = ChunkPos;
		
		if (NeighborBlockPos.X < 0)
		{
			NeighborChunkPos.X -= 1;
			NeighborBlockPos.X += ChunkSetup.ChunkSize.X;
		}
		else if (NeighborBlockPos.X >= ChunkSetup.ChunkSize.X)
		{
			NeighborChunkPos.X += 1;
			NeighborBlockPos.X -= ChunkSetup.ChunkSize.X;
		}
		
		if (NeighborBlockPos.Y < 0)
		{
			NeighborChunkPos.Y -= 1;
			NeighborBlockPos.Y += ChunkSetup.ChunkSize.Y;
		}
		else if (NeighborBlockPos.Y >= ChunkSetup.ChunkSize.Y)
		{
			NeighborChunkPos.Y += 1;
			NeighborBlockPos.Y -= ChunkSetup.ChunkSize.Y;
		}	
		
		if (auto* NeighborChunkActor = ChunkGeneratorSubsystem->GetChunkActor(NeighborChunkPos))
		{
			NeighborChunkActor->ActualizeModifiedInstance(NeighborBlockPos);
		}
	}
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
	
	UpdateNeighborInstancesVisibility(Pos + FIntVector(1, 0, 0));
	UpdateNeighborInstancesVisibility(Pos + FIntVector(-1, 0, 0));
	UpdateNeighborInstancesVisibility(Pos + FIntVector(0, 1, 0));
	UpdateNeighborInstancesVisibility(Pos + FIntVector(0, -1, 0));
	UpdateNeighborInstancesVisibility(Pos + FIntVector(0, 0, 1));
	UpdateNeighborInstancesVisibility(Pos + FIntVector(0, 0, -1));
	
}

EBlockType AChunkActor::GetBlockType(const FIntVector& Pos) const
{
	if (!IsBlockPosInChunkBounds(Pos))
	{
		return EBlockType::Air;
	}
	
	int32 ID = GetBlockIDFromPos(Pos);
	return ChunkData[ID];
}

bool AChunkActor::IsBlockPosInChunkBounds(const FIntVector& BlockPos) const
{
	return UChunkHelperFunctions::IsPosInBounds(
		BlockPos, 
		{0,0,0}, 
		ChunkSetup.ChunkSize
		);
}

bool AChunkActor::IsAirBlock(const FIntVector& BlockPos) const
{
	if (!IsBlockPosInChunkBounds(BlockPos))
	{
		if (BlockPos.Z < 0)
		{
			return false;
		}
		if (BlockPos.Z >= ChunkSetup.ChunkSize.Z)
		{
			return true;
		}
		return false;
	}

	return GetBlockType(BlockPos) == EBlockType::Air;
}

bool AChunkActor::IsBlockVisible(const FIntVector& BlockPos) const
{
	if (GetBlockType(BlockPos) == EBlockType::Air)
	{
		return false;
	}

	bool bIsVisible = false;
	
	bIsVisible |= IsAirBlock(BlockPos + FIntVector(0, 0, -1));
	bIsVisible |= IsAirBlock(BlockPos + FIntVector(0, 0, 1));
	
	// Check X+ neighbor
	FIntVector NextBlockPos = BlockPos + FIntVector(1, 0, 0);
	if (NextBlockPos.X >= ChunkSetup.ChunkSize.X)
	{
		if (auto* NextChunkActor = ChunkGeneratorSubsystem->GetChunkActor(ChunkPos + FIntVector(1, 0, 0)))
		{
			bIsVisible |= NextChunkActor->IsAirBlock({0, NextBlockPos.Y, NextBlockPos.Z});
		}
	}
	else { bIsVisible |= IsAirBlock(NextBlockPos); }
	
	// Check X- neighbor
	NextBlockPos = BlockPos + FIntVector(-1, 0, 0);
	if (NextBlockPos.X < 0)
	{
		if (auto* NextChunkActor = ChunkGeneratorSubsystem->GetChunkActor(ChunkPos + FIntVector(-1, 0, 0)))
		{
			bIsVisible |= NextChunkActor->IsAirBlock({ChunkSetup.ChunkSize.X - 1, NextBlockPos.Y, NextBlockPos.Z});
		}
	}
	else { bIsVisible |= IsAirBlock(NextBlockPos); }
	
	
	// Check Y+ neighbor
	NextBlockPos = BlockPos + FIntVector(0, 1, 0);
	if (NextBlockPos.Y >= ChunkSetup.ChunkSize.Y)
	{
		if (auto* NextChunkActor = ChunkGeneratorSubsystem->GetChunkActor(ChunkPos + FIntVector(0, 1, 0)))
		{
			bIsVisible |= NextChunkActor->IsAirBlock({NextBlockPos.X, 0, NextBlockPos.Z});
		}
	}
	else { bIsVisible |= IsAirBlock(NextBlockPos); }
	
	// Check Y- neighbor
	NextBlockPos = BlockPos + FIntVector(0, -1, 0);
	if (NextBlockPos.Y < 0)
	{
		if (auto* NextChunkActor = ChunkGeneratorSubsystem->GetChunkActor(ChunkPos + FIntVector(0, -1, 0)))
		{
			bIsVisible |= NextChunkActor->IsAirBlock({NextBlockPos.X, ChunkSetup.ChunkSize.Y - 1, NextBlockPos.Z});
		}
	}
	else { bIsVisible |= IsAirBlock(NextBlockPos); }
	
	return bIsVisible;
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
	return UChunkHelperFunctions::GetBlockRealPos(ChunkSetup, ChunkPos, Pos);
}

FIntVector AChunkActor::GetBlockGridPos(const FVector& Pos) const
{
	return UChunkHelperFunctions::GetBlockGridPos(ChunkSetup, ChunkPos, Pos);
}

/*
void AChunkActor::SetISMCollisionEnabled(bool bEnable)
{
	InstancedStaticMeshComponent->SetCollisionEnabled(
		bEnable ? ECollisionEnabled::Type::QueryAndPhysics : ECollisionEnabled::Type::NoCollision
		);
}
*/

