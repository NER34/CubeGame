// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ChunkStructures.generated.h"

UENUM(BlueprintType)
enum class EBlockType : uint8
{
	Air = 0,
	Stone,
	Grass,
	Snow
};

USTRUCT(BlueprintType)
struct FChunkSetup
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BlockSize = {50.0, 50.0, 50.0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector ChunkSize = {8, 8, 8};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RandomSeed = 0;
	
};

USTRUCT(BlueprintType)
struct FChunkGeneratorSetup
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FChunkSetup ChunkSetup;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector ChunkBoundsMin = {INT32_MIN, INT32_MIN, 0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntVector ChunkBoundsMax = {INT32_MAX, INT32_MAX, 0};
	
};