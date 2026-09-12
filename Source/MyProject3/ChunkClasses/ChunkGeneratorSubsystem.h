// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ChunkStructures.h"
#include "ChunkGeneratorSubsystem.generated.h"


class AChunkActor;

UCLASS()
class MYPROJECT3_API UChunkGeneratorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void SetChunkGeneratorSetup(const FChunkGeneratorSetup& InChunkSetup);
	
	UFUNCTION(BlueprintPure)
	const FChunkGeneratorSetup& GetChunkGeneratorSetup() const;
	
	UFUNCTION(BlueprintCallable, meta=(WorldContext="WorldContext"))
	void LoadChunk(UObject* WorldContext, TSubclassOf<AChunkActor> ChunkActorClass, FIntVector ChunkPos);
	
	UFUNCTION(BlueprintCallable)
	void UnloadChunk(FIntVector ChunkPos);
	
	UFUNCTION(BlueprintCallable)
	AChunkActor* GetChunkActor(FIntVector ChunkPos);
	
	UFUNCTION(BlueprintPure)
	FVector CalculateChunkRealPosition(FIntVector ChunkPos) const;
	
	UFUNCTION(BlueprintPure)
	FIntVector GetChunkGridPosition(FVector RealPos) const;
	
	UFUNCTION(BlueprintPure)
	bool IsChunkPosInBounds(FIntVector ChunkPos) const;

private:
	
	UPROPERTY()
	TMap<FIntVector, AChunkActor*> ChunkActors;
	
	FChunkGeneratorSetup ChunkGeneratorSetup;
	
};
