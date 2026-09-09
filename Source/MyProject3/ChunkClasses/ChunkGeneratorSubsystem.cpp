// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkGeneratorSubsystem.h"

#include "ChunkActor.h"


void UChunkGeneratorSubsystem::GenerateHeightMap(int32 NewSeed)
{
	
}

void UChunkGeneratorSubsystem::SetChunkGeneratorSetup(const FChunkGeneratorSetup& InChunkSetup)
{
	ChunkGeneratorSetup = InChunkSetup;
}

const FChunkGeneratorSetup& UChunkGeneratorSubsystem::GetChunkGeneratorSetup() const
{
	return ChunkGeneratorSetup;
}

void UChunkGeneratorSubsystem::LoadChunk(UObject* WorldContext, TSubclassOf<AChunkActor> ChunkActorClass, FIntVector ChunkPos)
{
	if (!ChunkActors.Contains(ChunkPos) && IsChunkPosInBounds(ChunkPos))
	{
		FVector ChunkRealPos = CalculateChunkRealPosition(ChunkPos);
		AActor* SpawnedActor = GetWorld()->SpawnActor(ChunkActorClass, &ChunkRealPos);
		check(SpawnedActor != nullptr);
		ChunkActors.Add(ChunkPos, static_cast<AChunkActor*>(SpawnedActor));
	}
}

void UChunkGeneratorSubsystem::UnloadChunk(FIntVector ChunkPos)
{
	AChunkActor* ChunkActor;
	if (ChunkActors.RemoveAndCopyValue(ChunkPos, ChunkActor))
	{
		ChunkActor->Destroy();
	}
}

FVector UChunkGeneratorSubsystem::CalculateChunkRealPosition(FIntVector ChunkPos)
{
	const auto& ChunkSetup = ChunkGeneratorSetup.ChunkSetup;
	return {
		ChunkSetup.BlockSize.X * ChunkSetup.ChunkSize.X * ChunkPos.X,
		ChunkSetup.BlockSize.Y * ChunkSetup.ChunkSize.Y * ChunkPos.Y,
		ChunkSetup.BlockSize.Z * ChunkSetup.ChunkSize.Z * ChunkPos.Z,
	};
}

FIntVector UChunkGeneratorSubsystem::GetChunkGridPosition(FVector RealPos)
{
	const auto& ChunkSetup = ChunkGeneratorSetup.ChunkSetup;
	return {
		static_cast<int32>(RealPos.X / (ChunkSetup.ChunkSize.X * ChunkSetup.BlockSize.X)),
		static_cast<int32>(RealPos.Y / (ChunkSetup.ChunkSize.Y * ChunkSetup.BlockSize.Y)),
		static_cast<int32>(RealPos.Z / (ChunkSetup.ChunkSize.Z * ChunkSetup.BlockSize.Z)),
	};
}

bool UChunkGeneratorSubsystem::IsChunkPosInBounds(FIntVector ChunkPos) const
{
	const auto& Min = ChunkGeneratorSetup.ChunkBoundsMin;
	const auto& Max = ChunkGeneratorSetup.ChunkBoundsMax;
	return Min.X <= ChunkPos.X && ChunkPos.X <= Max.X
		&& Min.Y <= ChunkPos.Y && ChunkPos.Y <= Max.Y
		&& Min.Z <= ChunkPos.Z && ChunkPos.Z <= Max.Z;
}
