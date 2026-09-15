// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkGeneratorSubsystem.h"

#include "ChunkActor.h"
#include "ChunkGeneratorSettings.h"
#include "ChunkHelperFunctions.h"


void UChunkGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (auto* Settings = UChunkGeneratorSettings::StaticClass()->GetDefaultObject<UChunkGeneratorSettings>())
	{
		ChunkGeneratorSetup = Settings->ChunkGeneratorSetup;
		ChunkActorClass = Settings->ChunkActorClass;
	}
}

const FChunkGeneratorSetup& UChunkGeneratorSubsystem::GetChunkGeneratorSetup() const
{
	return ChunkGeneratorSetup;
}

void UChunkGeneratorSubsystem::LoadChunk(UObject* WorldContext, FIntVector ChunkPos)
{
	if (!ChunkActors.Contains(ChunkPos) && IsChunkPosInBounds(ChunkPos) && ChunkActorClass)
	{
		AChunkActor* ChunkActor = AcquireChunkActor(WorldContext/*, ChunkActorClass, ChunkPos*/);
		if (!ChunkActor)
		{
			return;
		}

		ChunkActors.Add(ChunkPos, ChunkActor);
		ChunkActor->Initialize(ChunkGeneratorSetup.ChunkSetup, ChunkPos);
	}
}

void UChunkGeneratorSubsystem::UnloadChunk(FIntVector ChunkPos)
{
	AChunkActor* ChunkActor;
	if (ChunkActors.RemoveAndCopyValue(ChunkPos, ChunkActor))
	{
		ChunkActor->Deinitialize();
		InactiveChunkActors.Add(ChunkActor);
	}
}

AChunkActor* UChunkGeneratorSubsystem::AcquireChunkActor(const UObject* WorldContext/*, FIntVector ChunkPos*/)
{
	if (!InactiveChunkActors.IsEmpty())
	{
		return InactiveChunkActors.Pop();
	}

	UWorld* World = WorldContext ? WorldContext->GetWorld() : GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AChunkActor* ChunkActor = World->SpawnActor<AChunkActor>(ChunkActorClass);

	return ChunkActor;
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
	return UChunkHelperFunctions::IsChunkPosInBounds(
		ChunkPos, ChunkGeneratorSetup.ChunkBoundsMin, ChunkGeneratorSetup.ChunkBoundsMax
		);
}

int32 UChunkGeneratorSubsystem::GetNumActiveChunks() const
{
	return ChunkActors.Num();
}

int32 UChunkGeneratorSubsystem::GetNumInactiveChunks() const
{
	return InactiveChunkActors.Num();
}
