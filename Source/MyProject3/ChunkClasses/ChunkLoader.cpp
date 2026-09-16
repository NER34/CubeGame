// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkLoader.h"

#include "ChunkActor.h"
#include "ChunkGeneratorSubsystem.h"
#include "ChunkHelperFunctions.h"


UChunkLoader::UChunkLoader()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UChunkLoader::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		ChunkGeneratorSubsystem = GameInstance->GetSubsystem<UChunkGeneratorSubsystem>();
	}

	UnloadRadius = FMath::Max(UnloadRadius, LoadRadius);
	UpdatePlayerChunk(true);
}


// Called every frame
void UChunkLoader::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdatePlayerChunk();
	ProcessChunkQueues();
}

void UChunkLoader::UpdatePlayerChunk(bool bForceUpdate)
{
	if (!CanUseChunkGenerator())
	{
		return;
	}

	const FIntVector NewPlayerChunk = ChunkGeneratorSubsystem->GetChunkGridPosition(GetOwner()->GetActorLocation());
	if (bForceUpdate || NewPlayerChunk != CurrentPlayerChunk)
	{
		CurrentPlayerChunk = NewPlayerChunk;
		RebuildChunkRequests(CurrentPlayerChunk);
	}
}

void UChunkLoader::RebuildChunkRequests(const FIntVector& CenterChunk)
{
	LoadQueue.Empty();
	UnloadQueue.Empty();
	//LoadCollisionQueue.Empty();

	TSet<FIntVector> WantedLoadChunks;
	TSet<FIntVector> AllowedLoadedChunks;

	BuildChunkSetInRadius(CenterChunk, LoadRadius, WantedLoadChunks);
	BuildChunkSetInRadius(CenterChunk, UnloadRadius, AllowedLoadedChunks);

	TArray<FIntVector> ChunksToLoad;
	for (const FIntVector& ChunkPos : WantedLoadChunks)
	{
		if (!LoadedChunks.Contains(ChunkPos))
		{
			ChunksToLoad.Add(ChunkPos);
		}
	}

	ChunksToLoad.Sort([CenterChunk](const FIntVector& Left, const FIntVector& Right)
	{
		return FMath::Abs(Left.X - CenterChunk.X) + FMath::Abs(Left.Y - CenterChunk.Y) + FMath::Abs(Left.Z - CenterChunk.Z)
			< FMath::Abs(Right.X - CenterChunk.X) + FMath::Abs(Right.Y - CenterChunk.Y) + FMath::Abs(Right.Z - CenterChunk.Z);
	});

	for (const FIntVector& ChunkPos : ChunksToLoad)
	{
		LoadQueue.Enqueue(ChunkPos);
		//LoadCollisionQueue.Enqueue(ChunkPos);
	}

	for (const FIntVector& ChunkPos : LoadedChunks)
	{
		if (!AllowedLoadedChunks.Contains(ChunkPos))
		{
			UnloadQueue.Enqueue(ChunkPos);
		}
	}
}

void UChunkLoader::ProcessChunkQueues()
{
	if (!CanUseChunkGenerator())
	{
		return;
	}

	ProcessUnloadQueue();
	ProcessLoadQueue();
	//ProcessLoadCollisionQueue();
}

void UChunkLoader::ProcessUnloadQueue()
{
	for (int32 UnloadsThisTick = 0; UnloadsThisTick < MaxUnloadsPerTick && !UnloadQueue.IsEmpty(); ++UnloadsThisTick)
	{
		FIntVector ChunkPos;
		UnloadQueue.Dequeue(ChunkPos);

		if (!LoadedChunks.Contains(ChunkPos) || IsChunkPosInBounds(ChunkPos, CurrentPlayerChunk, UnloadRadius))
		{
			continue;
		}

		ChunkGeneratorSubsystem->UnloadChunk(ChunkPos);
		LoadedChunks.Remove(ChunkPos);
	}
}

void UChunkLoader::ProcessLoadQueue()
{
	for (int32 LoadsThisTick = 0; LoadsThisTick < MaxLoadsPerTick && !LoadQueue.IsEmpty(); ++LoadsThisTick)
	{
		FIntVector ChunkPos;
		LoadQueue.Dequeue(ChunkPos);

		if (LoadedChunks.Contains(ChunkPos) || !IsChunkPosInBounds(ChunkPos, CurrentPlayerChunk, LoadRadius))
		{
			continue;
		}

		if (!ChunkGeneratorSubsystem->IsChunkPosInBounds(ChunkPos))
		{
			continue;
		}

		ChunkGeneratorSubsystem->LoadChunk(this, ChunkPos);
		if (ChunkGeneratorSubsystem->GetChunkActor(ChunkPos))
		{
			LoadedChunks.Add(ChunkPos);
		}
	}
}

/*
void UChunkLoader::ProcessLoadCollisionQueue()
{
	for (int32 LoadsThisTick = 0; LoadsThisTick < MaxCollisionLoadsPerTick && !LoadQueue.IsEmpty(); ++LoadsThisTick)
	{
		FIntVector ChunkPos;
		LoadCollisionQueue.Dequeue(ChunkPos);

		if (!LoadedChunks.Contains(ChunkPos) || !IsChunkPosInBounds(ChunkPos, CurrentPlayerChunk, LoadRadius))
		{
			continue;
		}

		if (!ChunkGeneratorSubsystem->IsChunkPosInBounds(ChunkPos))
		{
			continue;
		}

		auto* ChunkActor = ChunkGeneratorSubsystem->GetChunkActor(ChunkPos);
		ChunkActor->SetISMCollisionEnabled(true);
	}
}
*/

void UChunkLoader::BuildChunkSetInRadius(const FIntVector& CenterChunk, int32 Radius, TSet<FIntVector>& OutChunks) const
{
	OutChunks.Reset();
	// TODO: I've decided that I'm going to use 2D chunk grid. I either need to remember it, or modify the whole system.
	// Maybe even switch FIntVector to FIntVector2D. Such interface change will be a mess, so I'l leave it for now
	for (int32 X = -Radius; X <= Radius; ++X)
	{
		for (int32 Y = -Radius; Y <= Radius; ++Y)
		{
			const FIntVector ChunkPos(CenterChunk.X + X, CenterChunk.Y + Y, CenterChunk.Z);
			if (ChunkGeneratorSubsystem->IsChunkPosInBounds(ChunkPos))
			{
				OutChunks.Add(ChunkPos);
			}
		}
	}
}

bool UChunkLoader::IsChunkPosInBounds(const FIntVector& ChunkPos, const FIntVector& CenterChunk, int32 Radius) const
{
	return FMath::Abs(ChunkPos.X - CenterChunk.X) <= Radius
		&& FMath::Abs(ChunkPos.Y - CenterChunk.Y) <= Radius
		&& FMath::Abs(ChunkPos.Z - CenterChunk.Z) <= Radius;
}

bool UChunkLoader::CanUseChunkGenerator() const
{
	return ChunkGeneratorSubsystem && GetOwner();
}

