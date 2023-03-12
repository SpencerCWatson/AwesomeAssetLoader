// Fill out your copyright notice in the Description page of Project Settings.


#include "AwesomeAssetManager.h"

bool UAwesomeAssetManager::AddAssetLibrary(FName LibraryName, const TArray<FAwesomeAssetData>& Assets)
{
	if(LibraryName != NAME_None && !Assets.IsEmpty())
	{
		UAssetContainer* NewLibrary = NewObject<UAssetContainer>(this);
		NewLibrary->Initialize(LibraryName, Assets);
		Libraries.Emplace(LibraryName, NewLibrary);
		return true;
	}
	return false;
}

void UAwesomeAssetManager::FilterAssets(FName LibraryName, FGameplayTagContainer Filter)
{
}

void UAwesomeAssetManager::SortAssets(FName LibraryName, TArray<FGameplayTag> Order)
{
}

void UAwesomeAssetManager::SetBufferType(FName LibraryName, EBufferType BufferType)
{
}

void UAwesomeAssetManager::SetBufferTarget(FName LibraryName, FName AssetId)
{
}

UAssetContainer* UAwesomeAssetManager::GetAssetLibrary(FName LibraryName)
{
	return *Libraries.Find(LibraryName);
}

