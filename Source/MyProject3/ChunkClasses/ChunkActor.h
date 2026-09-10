// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkStructures.h"
#include "MyProject3/ThirdParty/FastNoiseLite.h"
#include "ChunkActor.generated.h"

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

protected:
	
	void GenerateChunkData();
	EBlockType GetBlockTypeByHeight(float Height);
	
	UFUNCTION(BlueprintPure)
	const TArray<EBlockType>& GetChunkData() const;
	
	UFUNCTION(BlueprintPure)
	const FChunkSetup& GetChunkSetup() const;
	
	UFUNCTION(BlueprintPure)
	int32 GetBlockIDFromPos(FIntVector Pos) const;

	UFUNCTION(BlueprintPure)
	FIntVector GetBlockPosFromID(int32 ID) const;
	
	UFUNCTION(BlueprintPure)
	FVector GetBlockRealPos(const FIntVector& Pos) const;
	
	virtual void BeginPlay() override;
	
private:

	FChunkSetup ChunkSetup;
	
	FastNoiseLite NoiseGenerator;
	
	TArray<EBlockType> ChunkData;
	
};
