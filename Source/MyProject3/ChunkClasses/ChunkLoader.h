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
	// Sets default values for this component's properties
	UChunkLoader();

protected:

	virtual void BeginPlay() override;

public:

	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction
		) override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunks", meta=(AllowPrivateAccess="true", ClampMin="0"))
	int32 LoadRadius = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunks", meta=(AllowPrivateAccess="true", ClampMin="0"))
	int32 UnloadRadius = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunks", meta=(AllowPrivateAccess="true", ClampMin="1"))
	int32 MaxLoadsPerTick = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunks", meta=(AllowPrivateAccess="true", ClampMin="1"))
	int32 MaxCollisionLoadsPerTick = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chunks", meta=(AllowPrivateAccess="true", ClampMin="1"))
	int32 MaxUnloadsPerTick = 4;

	UPROPERTY()
	UChunkGeneratorSubsystem* ChunkGeneratorSubsystem = nullptr;

	TSet<FIntVector> LoadedChunks;

	TQueue<FIntVector> LoadQueue;
	//TQueue<FIntVector> LoadCollisionQueue;
	TQueue<FIntVector> UnloadQueue;

	FIntVector CurrentPlayerChunk = FIntVector::ZeroValue;

	void UpdatePlayerChunk(bool bForceUpdate = false);
	void RebuildChunkRequests(const FIntVector& CenterChunk);
	void ProcessChunkQueues();
	void ProcessUnloadQueue();
	void ProcessLoadQueue();
	//void ProcessLoadCollisionQueue();
	void BuildChunkSetInRadius(const FIntVector& CenterChunk, int32 Radius, TSet<FIntVector>& OutChunks) const;
	bool IsChunkPosInBounds(const FIntVector& ChunkPos, const FIntVector& CenterChunk, int32 Radius) const;
	bool CanUseChunkGenerator() const;
};
