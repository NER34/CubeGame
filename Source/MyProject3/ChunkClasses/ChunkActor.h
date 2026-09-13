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
	
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintNativeEvent)
	void Initialize(const FChunkSetup& InChunkSetup, FIntVector InChunkPos);
	virtual void Initialize_Implementation(const FChunkSetup& InChunkSetup, FIntVector InChunkPos);
	
	UFUNCTION(BlueprintPure)
	const FChunkSetup& GetChunkSetup() const;
	
	UFUNCTION(BlueprintCallable)
	void SetBlockType(EBlockType BlockType, FIntVector Pos);
	
	UFUNCTION(BlueprintPure)
	EBlockType GetBlockType(FIntVector Pos) const;
	
	UFUNCTION(BlueprintPure)
	bool IsBlockPosInChunkBounds(FIntVector BlockPos) const;
	
	UFUNCTION(BlueprintCallable)
	void SetBlockDestructionAlpha(FIntVector BlockPos, float Alpha);
	
	UFUNCTION(BlueprintCallable)
	void SetBlockHighlightFlag(FIntVector BlockPos, bool bHighlight);
	
protected:
	
	void CreateInstance(int32 BlockID);
	
	void GenerateInstances();
	void GenerateChunkData();
	void UpdateInstancesVisibility(FIntVector ModifiedBlock);
	void UpdateNeighborInstancesVisibility(FIntVector ModifiedBlock, FIntVector Delta);
	EBlockType GetBlockTypeByHeight(float Height);
	
	UFUNCTION(BlueprintPure)
	const TArray<EBlockType>& GetChunkData() const;
	
	UFUNCTION(BlueprintPure)
	int32 GetBlockIDFromPos(FIntVector Pos) const;

	UFUNCTION(BlueprintPure)
	FIntVector GetBlockPosFromID(int32 ID) const;
	
	UFUNCTION(BlueprintPure)
	FVector GetBlockRealPos(const FIntVector& Pos) const;
	
	virtual void BeginPlay() override;
	
private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> InstancedStaticMeshComponent;
	
	TMap<int32, FBlockInstanceData> VisibleInstances;

	FChunkSetup ChunkSetup;
	
	FastNoiseLite NoiseGenerator;
	
	TArray<EBlockType> ChunkData;
	
	FIntVector ChunkPos;
	
};
