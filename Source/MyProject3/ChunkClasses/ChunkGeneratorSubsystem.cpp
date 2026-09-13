// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkGeneratorSubsystem.h"

#include "ChunkActor.h"
#include "ChunkHelperFunctions.h"


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
		auto* ChunkActor = ChunkActors.Add(ChunkPos, static_cast<AChunkActor*>(SpawnedActor));
		ChunkActor->Initialize(ChunkGeneratorSetup.ChunkSetup, ChunkPos);
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

AChunkActor* UChunkGeneratorSubsystem::GetChunkActor(FIntVector ChunkPos)
{
	auto** ChunkActor = ChunkActors.Find(ChunkPos); 
	return ChunkActor ? *ChunkActor : nullptr;
}

FVector UChunkGeneratorSubsystem::CalculateChunkRealPosition(FIntVector ChunkPos) const
{
	return UChunkHelperFunctions::CalculateChunkRealPosition(ChunkGeneratorSetup.ChunkSetup, ChunkPos);
}

FIntVector UChunkGeneratorSubsystem::GetChunkGridPosition(FVector RealPos) const
{
	return UChunkHelperFunctions::GetChunkGridPosition(ChunkGeneratorSetup.ChunkSetup, RealPos);
}

bool UChunkGeneratorSubsystem::IsChunkPosInBounds(FIntVector ChunkPos) const
{
	return UChunkHelperFunctions::IsChunkPosInBounds(ChunkGeneratorSetup, ChunkPos);
}
