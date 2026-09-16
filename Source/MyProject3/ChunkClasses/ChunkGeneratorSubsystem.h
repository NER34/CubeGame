// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ChunkStructures.h"
#include "MyProject3/ThirdParty/FastNoiseLite.h"
#include "ChunkGeneratorSubsystem.generated.h"


class AChunkActor;

UCLASS()
class MYPROJECT3_API UChunkGeneratorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContext"))
	void LoadChunk(UObject* WorldContext, FIntVector ChunkPos);
	
	UFUNCTION(BlueprintCallable)
	void UnloadChunk(FIntVector ChunkPos);
	
	UFUNCTION(BlueprintCallable)
	AChunkActor* GetChunkActor(FIntVector ChunkPos);
	
private:

	AChunkActor* AcquireChunkActor(const UObject* WorldContext);
	
public:

	UFUNCTION(BlueprintPure)
	const FChunkGeneratorSetup& GetChunkGeneratorSetup() const;
	
	UFUNCTION(BlueprintPure)
	FVector CalculateChunkRealPosition(FIntVector ChunkPos) const;
	
	UFUNCTION(BlueprintPure)
	FIntVector GetChunkGridPosition(FVector RealPos) const;
	
	UFUNCTION(BlueprintPure)
	bool IsChunkPosInBounds(FIntVector ChunkPos) const;
	
	EBlockType GenerateBlockType(FIntVector ChunkPos, FIntVector BlockPos) const;

private:
	
	UPROPERTY()
	TSubclassOf<AChunkActor> ChunkActorClass;
	
	UPROPERTY()
	TMap<FIntVector, AChunkActor*> ChunkActors;

	UPROPERTY()
	TArray<AChunkActor*> InactiveChunkActors;
	
	FChunkGeneratorSetup ChunkGeneratorSetup;
	
	FastNoiseLite NoiseGenerator;
	
};
