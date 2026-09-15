// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkStructures.h"
#include "Engine/DeveloperSettings.h"
#include "ChunkGeneratorSettings.generated.h"

class AChunkActor;

UCLASS(Config=Game, DefaultConfig, DisplayName="Chunk Generator Settings")
class MYPROJECT3_API UChunkGeneratorSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, Config)
	FChunkGeneratorSetup ChunkGeneratorSetup;
	
	UPROPERTY(EditAnywhere, Config)
	TSubclassOf<AChunkActor> ChunkActorClass;
	
};
