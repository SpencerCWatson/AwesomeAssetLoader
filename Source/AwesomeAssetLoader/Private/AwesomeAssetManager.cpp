// Fill out your copyright notice in the Description page of Project Settings.


#include "AwesomeAssetManager.h"

#include "Engine/AssetManager.h"

void UAwesomeAssetManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	AssetManager = UAssetManager::GetIfValid();
}

bool UAwesomeAssetManager::AddAssetLibrary(FName LibraryName, const TArray<FAwesomeAssetData>& Assets)
{
	if(LibraryName != NAME_None && !Assets.IsEmpty())
	{
		UItemLibrary* NewLibrary = NewObject<UItemLibrary>(this);
		NewLibrary->Initialize(LibraryName, Assets);
		Libraries.Emplace(LibraryName, NewLibrary);
		return true;
	}
	return false;
}

void UAwesomeAssetManager::FilterAssets(FName LibraryName, const FGameplayTagContainer& Filter)
{
	if(UItemLibrary* Library =  GetAssetLibrary(LibraryName))
	{
		Library->FilteredAssets.Empty();
		for(FAwesomeAssetData Asset : Library->Items)
		{
			TArray<FGameplayTag> TagsAsArray;
			Asset.AssetDescriptions.GetKeys(TagsAsArray);
			FGameplayTagContainer TagContainer = FGameplayTagContainer::CreateFromArray(TagsAsArray);
			if(TagContainer.HasAll(Filter))
			{
				Library->FilteredAssets.Emplace(&Asset);
			}
		}
	}
}

void UAwesomeAssetManager::SortAssets(FName LibraryName, const TArray<FGameplayTag>& Order, bool bSortValueAscending)
{
	if(UItemLibrary* Library =  GetAssetLibrary(LibraryName))
	{
		Library->SortedAssets.Empty();
		// Organize and sort the asset data by each tag before combining. TODO add size to make more efficient.
		TArray<TArray<FAwesomeAssetData*>> SortBuckets;
		
		for(int32 i = 0; i < Library->Items.Num(); ++i)
		{
			for(int32 j = 0; j < Order.Num(); ++j)
			{
				// If current asset contains current tag.
				if(Library->Items[i].AssetDescriptions.Contains(Order[j]))
				{
					SortBuckets[j].Emplace(&Library->Items[i]);
					break;
				}
			}
		}
		// Sort by value in buckets.
		for(int32 i = 0; i < SortBuckets.Num(); ++i)
		{
			TMap<float,TArray<FAwesomeAssetData*>> SortingMap;
			for(FAwesomeAssetData* AssetData : SortBuckets[i])
			{
				if(TArray<FAwesomeAssetData*>* ValueArray = SortingMap.Find(*AssetData->AssetDescriptions.Find(Order[i])))
				{
					ValueArray->Emplace(AssetData);
				}
				else
				{
					TArray<FAwesomeAssetData*> TempArray;
					TempArray.Emplace(AssetData);
					SortingMap.Emplace(*AssetData->AssetDescriptions.Find(Order[i]), TArray<FAwesomeAssetData*>{AssetData});
				}
			}

			// Get asset arrays in order of sorted values.
			TArray<float> SortKeys;
			SortingMap.GetKeys(SortKeys);
			SortKeys.Sort();
			TArray<FAwesomeAssetData*> SortedBucket;
			for(float SortKey : SortKeys)
			{
				if(TArray<FAwesomeAssetData*>* ValueArray = SortingMap.Find(SortKey))
				{
					SortedBucket.Append(*ValueArray);
				}
			}
			// Reverse the bucket if requested before appending to sorted assets.
			if(!bSortValueAscending) Algo::Reverse(SortedBucket);
			Library->SortedAssets.Append(SortedBucket);
		}

		// Copy to the Id array.
		Library->SortedAssetIds.Empty();
		for(FAwesomeAssetData* Item : Library->SortedAssets)
		{
			Library->SortedAssetIds.Emplace(Item->UniqueId);
		}
	}
}

void UAwesomeAssetManager::SetBufferType(FName LibraryName, EBufferType BufferType)
{
	if(UItemLibrary* Library =  GetAssetLibrary(LibraryName))
	{
		Library->BufferType = BufferType;
	}
}

void UAwesomeAssetManager::SetBufferTarget(FName LibraryName, FName AssetId)
{
	if(UItemLibrary* Library =  GetAssetLibrary(LibraryName))
	{
		Library->BufferItemTarget = AssetId;
		UpdateBuffer(Library);
	}
}

void UAwesomeAssetManager::SetBufferTarget(FName LibraryName, int32 Page)
{
	if(UItemLibrary* Library =  GetAssetLibrary(LibraryName))
	{
		Library->BufferPageTarget = Page;
		UpdateBuffer(Library);
	}
}

UItemLibrary* UAwesomeAssetManager::GetAssetLibrary(FName LibraryName)
{
	return *Libraries.Find(LibraryName);
}

void UAwesomeAssetManager::UpdateBuffer(UItemLibrary* Library)
{
	if(Library->BufferType == EBufferType::Page)
	{
		TArray<FName> ItemsToUnload = Library->CurrentLoadedIds;
		// Find items to be loaded. Calculate chunk jsize to load.
		
		
	}
	else if(Library->BufferType == EBufferType::Range)
	{
		
	}
}

void UAwesomeAssetManager::CalculateBufferLoadIndexes(UItemLibrary* Library, int32 StartIndex&, int32 EndIndex&)
{
	if(Library->BufferType == EBufferType::Page)
	{
		StartIndex = (Library->BufferPageTarget * Library->BufferPageSize) - (Library->BufferSize * Library->BufferPageSize);
		StartIndex = FMath::Clamp(StartIndex, 0, Library->SortedAssetIds.Max());
		EndIndex = (((Library->BufferPageTarget + 1) * Library->BufferPageSize) - 1) + (Library->BufferSize * Library->BufferPageSize);
		EndIndex = FMath::Clamp(EndIndex, 0, Library->SortedAssetIds.Max());
	}
	else if(Library->BufferType == EBufferType::Range)
	{
		int32 TargetItemIndex = Library->SortedAssetIds.Find(Library->BufferItemTarget);

		StartIndex = TargetItemIndex - Library->BufferSize;
		StartIndex = FMath::Clamp(StartIndex, 0, Library->SortedAssetIds.Max());
		EndIndex = TargetItemIndex + Library->BufferSize;
		EndIndex = FMath::Clamp(EndIndex, 0, Library->SortedAssetIds.Max());
	}
}

