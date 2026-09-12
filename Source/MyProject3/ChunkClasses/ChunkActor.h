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
	void Initialize(const FChunkSetup&  InChunkSetup);
	virtual void Initialize_Implementation(const FChunkSetup& InChunkSetup);
	
	UFUNCTION(BlueprintPure)
	const FChunkSetup& GetChunkSetup() const;
	
	UFUNCTION(BlueprintCallable)
	void SetBlockType(EBlockType BlockType, FIntVector Pos);
	
	UFUNCTION(BlueprintPure)
	EBlockType GetBlockType(FIntVector Pos) const;
	
	UFUNCTION(BlueprintPure)
	bool IsBlockPosInChunkBounds(FIntVector BlockPos);
	
protected:
	
	void GenerateInstances();
	void GenerateChunkData();
	void UpdateInstancesVisibility(FIntVector ModifiedBlock);
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
	
	TMap<int32, FPrimitiveInstanceId> VisibleInstances;

	FChunkSetup ChunkSetup;
	
	FastNoiseLite NoiseGenerator;
	
	TArray<EBlockType> ChunkData;
	
	
	
};
