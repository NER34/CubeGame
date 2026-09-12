// Fill out your copyright notice in the Description page of Project Settings.


#include "ChunkFunctionLibrary.h"

#include "ChunkActor.h"


FVector UChunkFunctionLibrary::CalculateChunkRealPosition(const FChunkSetup& ChunkSetup, FIntVector ChunkPos)
{
	return {
		ChunkSetup.BlockSize.X * ChunkSetup.ChunkSize.X * ChunkPos.X,
		ChunkSetup.BlockSize.Y * ChunkSetup.ChunkSize.Y * ChunkPos.Y,
		ChunkSetup.BlockSize.Z * ChunkSetup.ChunkSize.Z * ChunkPos.Z,
	};	
}

FIntVector UChunkFunctionLibrary::GetChunkGridPosition(const FChunkSetup& ChunkSetup, FVector RealPos)
{
	return {
		static_cast<int32>(RealPos.X / (ChunkSetup.ChunkSize.X * ChunkSetup.BlockSize.X)),
		static_cast<int32>(RealPos.Y / (ChunkSetup.ChunkSize.Y * ChunkSetup.BlockSize.Y)),
		static_cast<int32>(RealPos.Z / (ChunkSetup.ChunkSize.Z * ChunkSetup.BlockSize.Z)),
	};
}

bool UChunkFunctionLibrary::IsChunkPosInBounds(const FChunkGeneratorSetup& GeneratorSetup, FIntVector ChunkPos)
{
	const auto& Min = GeneratorSetup.ChunkBoundsMin;
	const auto& Max = GeneratorSetup.ChunkBoundsMax;
	return Min.X <= ChunkPos.X && ChunkPos.X <= Max.X
		&& Min.Y <= ChunkPos.Y && ChunkPos.Y <= Max.Y
		&& Min.Z <= ChunkPos.Z && ChunkPos.Z <= Max.Z;
}

int32 UChunkFunctionLibrary::GetBlockIDFromPos(const FChunkSetup& ChunkSetup, FIntVector Pos)
{
	return Pos.Z 
		+ ChunkSetup.ChunkSize.Z * Pos.X 
		+ ChunkSetup.ChunkSize.Z * ChunkSetup.ChunkSize.X * Pos.Y;
}

FIntVector UChunkFunctionLibrary::GetBlockPosFromID(const FChunkSetup& ChunkSetup, int32 ID)
{
	int32 Z = ID % ChunkSetup.ChunkSize.Z;
	int32 XY = ID / ChunkSetup.ChunkSize.Z;
	int32 X = XY % ChunkSetup.ChunkSize.X;
	int32 Y = XY / ChunkSetup.ChunkSize.X;
	return {X, Y, Z};
}

FVector UChunkFunctionLibrary::GetBlockRealPos(const AChunkActor* ChunkActor, const FIntVector& Pos)
{
	FVector ChunkLocation = ChunkActor->GetActorLocation();
	const auto& ChunkSetup = ChunkActor->GetChunkSetup();
	return {
		Pos.X * ChunkSetup.BlockSize.X + ChunkLocation.X,
		Pos.Y * ChunkSetup.BlockSize.Y + ChunkLocation.Y,
		Pos.Z * ChunkSetup.BlockSize.Z + ChunkLocation.Z,
	};
}

FIntVector UChunkFunctionLibrary::GetBlockGridPos(const AChunkActor* ChunkActor, const FVector& RealPos)
{
	const auto& ChunkSetup = ChunkActor->GetChunkSetup();
	FVector RelativeBlockLocation = RealPos - ChunkActor->GetActorLocation();
	
	FVector ChunkSize_Double = ToVector(ChunkSetup.ChunkSize);
	FVector ChunkRealSize = ChunkSize_Double * ChunkSetup.BlockSize;
	
	return ToIntVector_Floor(
		ChunkSize_Double * (RelativeBlockLocation / ChunkRealSize)
		);
}

FVector UChunkFunctionLibrary::ToVector(const FIntVector& IntVec)
{
	return { (double)IntVec.X, (double)IntVec.Y, (double)IntVec.Z };
}

FIntVector UChunkFunctionLibrary::ToIntVector_Floor(const FVector& Vec)
{
	return { 
		FMath::FloorToInt32(Vec.X),
		FMath::FloorToInt32(Vec.Y),
		FMath::FloorToInt32(Vec.Z)
	};
}
