// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkStructures.h"
#include "MyProject3/ThirdParty/FastNoiseLite.h"
#include "ChunkActor.generated.h"

class UChunkGeneratorSubsystem;
class UInstancedStaticMeshComponent;

UCLASS()
class MYPROJECT3_API AChunkActor : public AActor
{
	GENERATED_BODY()

public:
	
	AChunkActor();
	
	virtual void BeginPlay() override;
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	
	virtual void Initialize(const FChunkSetup& InChunkSetup, const FIntVector& InChunkPos);
	virtual void Deinitialize();
	
	UFUNCTION(BlueprintPure)
	const FChunkSetup& GetChunkSetup() const;
	
	UFUNCTION(BlueprintCallable)
	void SetBlockType(EBlockType BlockType, const FIntVector& Pos);
	
	UFUNCTION(BlueprintPure)
	EBlockType GetBlockType(const FIntVector& Pos) const;
	
	UFUNCTION(BlueprintPure)
	bool IsBlockPosInChunkBounds(const FIntVector& BlockPos) const;

	UFUNCTION(BlueprintPure)
	bool IsAirBlock(const FIntVector& BlockPos) const;

	UFUNCTION(BlueprintPure)
	bool IsBlockVisible(const FIntVector& BlockPos) const;
	
	UFUNCTION(BlueprintCallable)
	void SetBlockDestructionAlpha(const FIntVector& BlockPos, float Alpha);
	
	UFUNCTION(BlueprintCallable)
	void SetBlockHighlightFlag(const FIntVector& BlockPos, bool bHighlight);
	
	
	UFUNCTION(BlueprintPure)
	int32 GetBlockIDFromPos(const FIntVector& Pos) const;

	UFUNCTION(BlueprintPure)
	FIntVector GetBlockPosFromID(int32 ID) const;
	
	UFUNCTION(BlueprintPure)
	FVector GetBlockRealPos(const FIntVector& Pos) const;
	
	UFUNCTION(BlueprintPure)
	FIntVector GetBlockGridPos(const FVector& Pos) const;
	
	/*
	void SetISMCollisionEnabled(bool bEnable);
	*/

protected:
	
	void CreateInstance(int32 BlockID);
	void ActualizeInstanceData(const FBlockInstanceData& InstanceData) const;
	
	void InitializeInstances();
	void InitializeChunkData();
	void ActualizeModifiedInstance(const FIntVector& ModifiedBlock);
	void UpdateNeighborInstancesVisibility(FIntVector NeighborBlockPos);
	
	UFUNCTION(BlueprintPure)
	const TArray<EBlockType>& GetChunkData() const;
private:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> InstancedStaticMeshComponent;
	
	TMap<int32, FBlockInstanceData> VisibleInstances;

	FChunkSetup ChunkSetup;
	
	TArray<EBlockType> ChunkData;
	
	FIntVector ChunkPos;
	
	bool bInstancesInitialized = false;
	
	UPROPERTY()
	UChunkGeneratorSubsystem* ChunkGeneratorSubsystem = nullptr;
	
};
