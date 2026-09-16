// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkHelperFunctions.h"

#include "ChunkActor.h"


FVector UChunkHelperFunctions::CalculateChunkRealPosition(const FChunkSetup& ChunkSetup, const FIntVector& ChunkPos)
{
	return {
		ChunkSetup.BlockSize.X * ChunkSetup.ChunkSize.X * ChunkPos.X,
		ChunkSetup.BlockSize.Y * ChunkSetup.ChunkSize.Y * ChunkPos.Y,
		ChunkSetup.BlockSize.Z * ChunkSetup.ChunkSize.Z * ChunkPos.Z,
	};	
}

FIntVector UChunkHelperFunctions::GetChunkGridPosition(const FChunkSetup& ChunkSetup, const FVector& RealPos)
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

FVector UChunkHelperFunctions::GetChunkRealPosition(const FChunkSetup& ChunkSetup, const FIntVector& Pos)
{
	return {
		Pos.X * ChunkSetup.BlockSize.X * ChunkSetup.ChunkSize.X,
		Pos.Y * ChunkSetup.BlockSize.Y * ChunkSetup.ChunkSize.Y,
		Pos.Z * ChunkSetup.BlockSize.Z * ChunkSetup.ChunkSize.Z,
	};
}

bool UChunkHelperFunctions::IsPosInBounds(const FIntVector& Pos, const FIntVector& Min, const FIntVector& Max)
{
	return Min.X <= Pos.X && Pos.X < Max.X
		&& Min.Y <= Pos.Y && Pos.Y < Max.Y
		&& Min.Z <= Pos.Z && Pos.Z < Max.Z;
}

int32 UChunkHelperFunctions::GetBlockIDFromPos(const FChunkSetup& ChunkSetup, const FIntVector& Pos)
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

FVector UChunkHelperFunctions::GetBlockRealPos(const FChunkSetup& ChunkSetup, const FIntVector& ChunkPos, const FIntVector& BlockPos)
{
	FVector ChunkLocation = GetChunkRealPosition(ChunkSetup, ChunkPos);
	return {
		BlockPos.X * ChunkSetup.BlockSize.X + ChunkLocation.X,
		BlockPos.Y * ChunkSetup.BlockSize.Y + ChunkLocation.Y,
		BlockPos.Z * ChunkSetup.BlockSize.Z + ChunkLocation.Z,
	};
}

FVector UChunkHelperFunctions::GetBlockScale(const FChunkSetup& ChunkSetup)
{
	return ChunkSetup.BlockSize / 100.0f;
}

FIntVector UChunkHelperFunctions::GetBlockGridPos(
	const FChunkSetup& ChunkSetup, const FIntVector& ChunkPos, const FVector& RealPos
	)
{
	FVector ChunkLocation = GetChunkRealPosition(ChunkSetup, ChunkPos);
	FVector RelativeBlockLocation = RealPos - ChunkLocation;
	
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