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
				
		int32 Seed	= ChunkGeneratorSetup.bGenerateRandomSeed 
					? FMath::Rand()
					: ChunkGeneratorSetup.Seed;
		
		NoiseGenerator.SetSeed(Seed);
		NoiseGenerator.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
		NoiseGenerator.SetFrequency(0.01f);
		NoiseGenerator.SetFractalType(FastNoiseLite::FractalType_FBm);
		NoiseGenerator.SetFractalOctaves(6);
		NoiseGenerator.SetFractalLacunarity(1.94f);
		NoiseGenerator.SetFractalGain(0.46f);
		NoiseGenerator.SetFractalWeightedStrength(0.38f);
	}
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

const FChunkGeneratorSetup& UChunkGeneratorSubsystem::GetChunkGeneratorSetup() const
{
	return ChunkGeneratorSetup;
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
	return UChunkHelperFunctions::IsPosInBounds(
		ChunkPos, 
		{INT32_MIN, INT32_MIN, 0}, 
		{INT32_MAX, INT32_MAX, 1}
	);
}

EBlockType UChunkGeneratorSubsystem::GenerateBlockType(FIntVector ChunkPos, FIntVector BlockPos) const
{
	if (BlockPos.Z == 0) { return EBlockType::Bedrock; }
	
	FVector BlockLocation = UChunkHelperFunctions::GetBlockRealPos(
		ChunkGeneratorSetup.ChunkSetup, ChunkPos, BlockPos
		);
	
	BlockLocation /= 10;
	
	float Height = NoiseGenerator.GetNoise(BlockLocation.X, BlockLocation.Y);
	// (-1.0, 1.0) -> (0.0, 1.0)
	Height = (Height + 1.0f) / 2.0f;
	
	float Height_Z = (float)BlockPos.Z / ChunkGeneratorSetup.ChunkSetup.ChunkSize.Z;
	
	if (Height_Z > Height)	{ return EBlockType::Air; }
	if (Height_Z > 0.5f)	{ return EBlockType::Snow; }
	if (Height_Z > 0.25f)	{ return EBlockType::Grass; }
	return EBlockType::Stone;
}
