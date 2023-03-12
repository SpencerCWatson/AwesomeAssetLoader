// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetContainer.h"



void UAssetContainer::Initialize(FName NewLibraryName, const TArray<FAwesomeAssetData>& NewAssets)
{
	UAssetContainer::LibraryName = 	NewLibraryName;
	UAssetContainer::Assets = NewAssets;
}
