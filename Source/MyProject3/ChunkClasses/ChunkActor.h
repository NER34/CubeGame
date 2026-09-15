// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkStructures.h"
#include "MyProject3/ThirdParty/FastNoiseLite.h"
#include "ChunkActor.generated.h"

class UInstancedStaticMeshComponent;

UCLASS()
class MYPROJECT3_API AChunkActor : public AActor
{
	GENERATED_BODY()

public:
	
	AChunkActor();
	
	virtual void Initialize(const FChunkSetup& InChunkSetup, const FIntVector& InChunkPos);
	UFUNCTION(BlueprintImplementableEvent)
	void OnInitialize(const FChunkSetup& InChunkSetup, const FIntVector& InChunkPos);

	virtual void Deinitialize();
	UFUNCTION(BlueprintImplementableEvent, DisplayName="Deinitialize")
	void OnDeinitialize();
	
	UFUNCTION(BlueprintPure)
	const FChunkSetup& GetChunkSetup() const;
	
	UFUNCTION(BlueprintCallable)
	void SetBlockType(EBlockType BlockType, const FIntVector& Pos);
	
	UFUNCTION(BlueprintPure)
	EBlockType GetBlockType(const FIntVector& Pos) const;
	
	UFUNCTION(BlueprintPure)
	bool IsBlockPosInChunkBounds(const FIntVector& BlockPos) const;
	
	UFUNCTION(BlueprintCallable)
	void SetBlockDestructionAlpha(const FIntVector& BlockPos, float Alpha);
	
	UFUNCTION(BlueprintCallable)
	void SetBlockHighlightFlag(const FIntVector& BlockPos, bool bHighlight);
	
protected:
	
	void CreateInstance(int32 BlockID);
	void ActualizeInstanceData(const FBlockInstanceData& InstanceData) const;
	
	void GenerateInstances();
	void GenerateChunkData(const FVector& ChunkLocation);
	void ActualizeModifiedInstance(const FIntVector& ModifiedBlock);
	void UpdateNeighborInstancesVisibility(const FIntVector& ModifiedBlock, const FIntVector& Delta);
	EBlockType GetBlockTypeByHeight(float Height);
	
	UFUNCTION(BlueprintPure)
	const TArray<EBlockType>& GetChunkData() const;
	
	UFUNCTION(BlueprintPure)
	int32 GetBlockIDFromPos(const FIntVector& Pos) const;

	UFUNCTION(BlueprintPure)
	FIntVector GetBlockPosFromID(int32 ID) const;
	
	UFUNCTION(BlueprintPure)
	FVector GetBlockRealPos(const FIntVector& Pos) const;
	
private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> InstancedStaticMeshComponent;
	
	TMap<int32, FBlockInstanceData> VisibleInstances;

	FChunkSetup ChunkSetup;
	
	FastNoiseLite NoiseGenerator;
	
	TArray<EBlockType> ChunkData;
	
	FIntVector ChunkPos;
	
};
