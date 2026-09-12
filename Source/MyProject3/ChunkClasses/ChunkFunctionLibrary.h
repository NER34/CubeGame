// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChunkStructures.h"
#include "ChunkFunctionLibrary.generated.h"

class AChunkActor;

UCLASS()
class MYPROJECT3_API UChunkFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintPure)
	static FVector CalculateChunkRealPosition(const FChunkSetup& ChunkSetup, FIntVector ChunkPos);
	
	UFUNCTION(BlueprintPure)
	static FIntVector GetChunkGridPosition(const FChunkSetup& ChunkSetup, FVector RealPos);
	
	UFUNCTION(BlueprintPure)
	static bool IsChunkPosInBounds(const FChunkGeneratorSetup& GeneratorSetup, FIntVector ChunkPos);
	
	UFUNCTION(BlueprintPure)
	static int32 GetBlockIDFromPos(const FChunkSetup& ChunkSetup, FIntVector Pos);

	UFUNCTION(BlueprintPure)
	static FIntVector GetBlockPosFromID(const FChunkSetup& ChunkSetup, int32 ID);
	
	UFUNCTION(BlueprintPure)
	static FVector GetBlockRealPos(const AChunkActor* ChunkActor, const FIntVector& Pos);
	
	// This one is not ready yet, I need to think how to detect the right block, no metter
	// from which side i'm pointing to it
	UFUNCTION(BlueprintPure)
	static FIntVector GetBlockGridPos(const AChunkActor* ChunkActor, const FVector& RealPos);
	
	UFUNCTION(BlueprintPure)
	static FVector ToVector(const FIntVector& IntVec);
	
	UFUNCTION(BlueprintPure)
	static FIntVector ToIntVector_Floor(const FVector& Vec);
	
};
