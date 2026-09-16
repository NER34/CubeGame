// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Components/ActorComponent.h"
#include "ChunkLoader.generated.h"

class AChunkActor;
class UChunkGeneratorSubsystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYPROJECT3_API UChunkLoader : public UActorComponent
{
	GENERATED_BODY()

public:

	UChunkLoader();

	virtual void BeginPlay() override;

	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction
		) override;
	
private:

	void UpdatePlayerChunk(bool bForceUpdate = false);
	void RebuildChunkRequests(const FIntVector& CenterChunk);
	
	void BuildChunkSetInRadius(const FIntVector& CenterChunk, int32 Radius, TSet<FIntVector>& OutChunks) const;
	bool IsChunkPosInBounds(const FIntVector& ChunkPos, const FIntVector& CenterChunk, int32 Radius) const;
	bool CanUseChunkGenerator() const;

	void ProcessChunkQueues();
	void ProcessUnloadQueue();
	void ProcessLoadQueue();

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunks", meta=(AllowPrivateAccess="true", ClampMin="0"))
	int32 LoadRadius = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunks", meta=(AllowPrivateAccess="true", ClampMin="0"))
	int32 UnloadRadius = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunks", meta=(AllowPrivateAccess="true", ClampMin="1"))
	int32 MaxLoadsPerTick = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunks", meta=(AllowPrivateAccess="true", ClampMin="1"))
	int32 MaxUnloadsPerTick = 4;

private:
	
	UPROPERTY()
	UChunkGeneratorSubsystem* ChunkGeneratorSubsystem = nullptr;

	TSet<FIntVector> LoadedChunks;

	TQueue<FIntVector> LoadQueue;
	TQueue<FIntVector> UnloadQueue;

	FIntVector CurrentPlayerChunk = FIntVector::ZeroValue;
};
