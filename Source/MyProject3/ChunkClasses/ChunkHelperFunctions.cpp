// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkHelperFunctions.h"

#include "ChunkActor.h"


FVector UChunkHelperFunctions::CalculateChunkRealPosition(const FChunkSetup& ChunkSetup, FIntVector ChunkPos)
{
	return {
		ChunkSetup.BlockSize.X * ChunkSetup.ChunkSize.X * ChunkPos.X,
		ChunkSetup.BlockSize.Y * ChunkSetup.ChunkSize.Y * ChunkPos.Y,
		ChunkSetup.BlockSize.Z * ChunkSetup.ChunkSize.Z * ChunkPos.Z,
	};	
}

FIntVector UChunkHelperFunctions::GetChunkGridPosition(const FChunkSetup& ChunkSetup, FVector RealPos)
{
	return {
		FMath::FloorToInt32(RealPos.X / (ChunkSetup.ChunkSize.X * ChunkSetup.BlockSize.X)),
		FMath::FloorToInt32(RealPos.Y / (ChunkSetup.ChunkSize.Y * ChunkSetup.BlockSize.Y)),
		// TODO: I've decided that I'm going to use 2D chunk grid. I either need to remember it, or modify the whole system.
		// Maybe even switch FIntVector to FIntVector2D. Such interface change will be a mess, so I'l leave it for now
		0,
		//FMath::FloorToInt32(RealPos.Z / (ChunkSetup.ChunkSize.Z * ChunkSetup.BlockSize.Z)),
	};
}

bool UChunkHelperFunctions::IsChunkPosInBounds(FIntVector ChunkPos, FIntVector Min, FIntVector Max)
{
	return Min.X <= ChunkPos.X && ChunkPos.X <= Max.X
		&& Min.Y <= ChunkPos.Y && ChunkPos.Y <= Max.Y
		&& Min.Z <= ChunkPos.Z && ChunkPos.Z <= Max.Z;
}

int32 UChunkHelperFunctions::GetBlockIDFromPos(const FChunkSetup& ChunkSetup, FIntVector Pos)
{
	return Pos.Z 
		+ ChunkSetup.ChunkSize.Z * Pos.X 
		+ ChunkSetup.ChunkSize.Z * ChunkSetup.ChunkSize.X * Pos.Y;
}

FIntVector UChunkHelperFunctions::GetBlockPosFromID(const FChunkSetup& ChunkSetup, int32 ID)
{
	int32 Z = ID % ChunkSetup.ChunkSize.Z;
	int32 XY = ID / ChunkSetup.ChunkSize.Z;
	int32 X = XY % ChunkSetup.ChunkSize.X;
	int32 Y = XY / ChunkSetup.ChunkSize.X;
	return {X, Y, Z};
}

FVector UChunkHelperFunctions::GetBlockRealPos(const AChunkActor* ChunkActor, const FIntVector& Pos)
{
	FVector ChunkLocation = ChunkActor->GetActorLocation();
	const auto& ChunkSetup = ChunkActor->GetChunkSetup();
	return {
		Pos.X * ChunkSetup.BlockSize.X + ChunkLocation.X,
		Pos.Y * ChunkSetup.BlockSize.Y + ChunkLocation.Y,
		Pos.Z * ChunkSetup.BlockSize.Z + ChunkLocation.Z,
	};
}

FIntVector UChunkHelperFunctions::GetBlockGridPos(const AChunkActor* ChunkActor, const FVector& RealPos)
{
	const auto& ChunkSetup = ChunkActor->GetChunkSetup();
	FVector RelativeBlockLocation = RealPos - ChunkActor->GetActorLocation();
	
	FVector ChunkSize_Double = ToVector(ChunkSetup.ChunkSize);
	FVector ChunkRealSize = ChunkSize_Double * ChunkSetup.BlockSize;
	
	return ToIntVector_Floor(
		ChunkSize_Double * (RelativeBlockLocation / ChunkRealSize)
		);
}

FVector UChunkHelperFunctions::ToVector(const FIntVector& IntVec)
{
	return { (double)IntVec.X, (double)IntVec.Y, (double)IntVec.Z };
}

FIntVector UChunkHelperFunctions::ToIntVector_Floor(const FVector& Vec)
{
	return { 
		FMath::FloorToInt32(Vec.X),
		FMath::FloorToInt32(Vec.Y),
		FMath::FloorToInt32(Vec.Z)
	};
}

float UChunkHelperFunctions::EncodeBlockInstanceData(const FBlockInstanceData& BlockInstanceData)
{
	float Data;
	uint8* DataBytes = (uint8*)&Data;

	const uint8 BlockTypeByte = static_cast<uint8>(BlockInstanceData.BlockType);
	const uint8 DestructionAlphaByte = static_cast<uint8>(
		FMath::Clamp(FMath::RoundToInt(BlockInstanceData.DestructionAlpha * 255.0f), 0, 255)
	);

	DataBytes[0] = BlockTypeByte;
	DataBytes[1] = DestructionAlphaByte;
	DataBytes[2] = BlockInstanceData.bHighlighted;

	return Data;
}