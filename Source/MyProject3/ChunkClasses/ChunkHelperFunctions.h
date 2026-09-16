// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChunkStructures.h"
#include "ChunkHelperFunctions.generated.h"

class AChunkActor;

UCLASS()
class MYPROJECT3_API UChunkHelperFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintPure)
	static FVector CalculateChunkRealPosition(const FChunkSetup& ChunkSetup, const FIntVector& ChunkPos);
	
	UFUNCTION(BlueprintPure)
	static FIntVector GetChunkGridPosition(const FChunkSetup& ChunkSetup, const FVector& RealPos);
	
	UFUNCTION(BlueprintPure)
	static FVector GetChunkRealPosition(const FChunkSetup& ChunkSetup, const FIntVector& Pos);
	
	UFUNCTION(BlueprintPure)
	static bool IsPosInBounds(const FIntVector& Pos, const FIntVector& Min, const FIntVector& Max);
	
	UFUNCTION(BlueprintPure)
	static int32 GetBlockIDFromPos(const FChunkSetup& ChunkSetup, const FIntVector& Pos);

	UFUNCTION(BlueprintPure)
	static FIntVector GetBlockPosFromID(const FChunkSetup& ChunkSetup, int32 ID);
	
	UFUNCTION(BlueprintPure)
	static FVector GetBlockRealPos(const FChunkSetup& ChunkSetup, const FIntVector& ChunkPos, const FIntVector& BlockPos);
	
	UFUNCTION(BlueprintPure)
	static FVector GetBlockScale(const FChunkSetup& ChunkSetup);
	
	// This one is not ready yet, I need to think how to detect the right block, no metter
	// from which side i'm pointing to it
	UFUNCTION(BlueprintPure)
	static FIntVector GetBlockGridPos(const FChunkSetup& ChunkSetup, const FIntVector& ChunkPos, const FVector& RealPos);
	
	UFUNCTION(BlueprintPure)
	static FVector ToVector(const FIntVector& IntVec);
	
	UFUNCTION(BlueprintPure)
	static FIntVector ToIntVector_Floor(const FVector& Vec);
	
	UFUNCTION(BlueprintPure)
	static float EncodeBlockInstanceData(const FBlockInstanceData& BlockInstanceData);
	
};
