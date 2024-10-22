// Fill out your copyright notice in the Description page of Project Settings.


#include "AwesomeAssetManager.h"



bool UAwesomeAssetManager::AddAssetLibrary(const FName& LibraryName, TSet<FAssetInitializeData> Assets)
{
	if (!LibraryName.IsNone() && !Assets.IsEmpty())
	{
		TSharedPtr<FItemLibrary> NewLibrary = MakeShared<FItemLibrary>();
		NewLibrary->Initialize(MoveTemp(Assets));
		Libraries.Emplace(LibraryName, NewLibrary);
		return true;
	}
	return false;
}

void UAwesomeAssetManager::FilterAssets(const FName& LibraryName, const FGameplayTagContainer& Filter)
{
	if (TSharedPtr<FItemLibrary> Library = GetLibrary(LibraryName))
	{
		Library->FilteredAssets.Empty(Library->Items.Num());
		for (const auto& Item : Library->Items)
		{
			TArray<FGameplayTag> TagsAsArray;
			Item->AssetDescriptions.GetKeys(TagsAsArray);
			const FGameplayTagContainer ItemTagContainer = FGameplayTagContainer::CreateFromArray(TagsAsArray);
			if (ItemTagContainer.HasAll(Filter))
			{
				Library->FilteredAssets.Emplace(Item);
			}
		}
	}
}

void UAwesomeAssetManager::SortAssets(const FName& LibraryName, const TArray<FGameplayTag>& Order, const bool bSortValuesDescending)
{
	TSharedPtr<FItemLibrary> Library = GetLibrary(LibraryName);
	
	if (!Library)
	{
		return;
	}
	
	// Organize and sort the asset data by each tag before combining. 
	Library->SortedAssets.Reset(Library->FilteredAssets.Num());
	TArray<TArray<TSharedPtr<FAwesomeAssetData>>> SortBuckets;
	SortBuckets.Init(TArray<TSharedPtr<FAwesomeAssetData>>(), Order.Num());
	
	for (const auto& Item : Library->Items)
	{
		for (int32 i = 0; i < Order.Num(); ++i)
		{
			// If current asset contains current tag.
			if (Item->AssetDescriptions.Contains(Order[i]))
			{
				SortBuckets[i].Emplace(Item);
				break;
			}
		}
	}
	
	// Sort by values in buckets.
	for (int32 i = 0; i < SortBuckets.Num(); ++i)
	{
		const FGameplayTag SortTag = Order[i];

		if (bSortValuesDescending)
		{
			SortBuckets[i].Sort([SortTag](const TSharedPtr<FAwesomeAssetData>& A, const TSharedPtr<FAwesomeAssetData>& B)
			{
				return A->AssetDescriptions.FindChecked(SortTag) > B->AssetDescriptions.FindChecked(SortTag);
			});
		}
		else
		{
			SortBuckets[i].Sort([SortTag](const TSharedPtr<FAwesomeAssetData>& A, const TSharedPtr<FAwesomeAssetData>& B)
			{
				return A->AssetDescriptions.FindChecked(SortTag) < B->AssetDescriptions.FindChecked(SortTag);
			});
		}
		
		Library->SortedAssets.Append(MoveTemp(SortBuckets[i]));
	}
}

void UAwesomeAssetManager::SetBufferTarget(TSharedPtr<FItemLibrary> Library, const int32 TargetStart, const int32 TargetEnd, const int32 BufferSize)
{
	if (Library)
	{
		Library->BufferSize = BufferSize;
		Library->TargetStart = TargetStart;
		Library->TargetEnd = TargetEnd;
		UpdateBuffer(Library);
	}
}

void UAwesomeAssetManager::SetBufferTargetByIndex(const FName& LibraryName, const int32 AssetIndex, const int32 BufferSize)
{
	SetBufferTarget(GetLibrary(LibraryName), AssetIndex, AssetIndex, BufferSize);
}

void UAwesomeAssetManager::SetBufferTargetByUniqueId(const FName& LibraryName, const FName& UniqueId, const int32 BufferSize)
{
	if (TSharedPtr<FItemLibrary> Library = GetLibrary(LibraryName))
	{
		for (int i = 0; i < Library->SortedAssets.Num(); ++i)
		{
			if (Library->SortedAssets[i]->UniqueId == UniqueId)
			{
				SetBufferTarget(Library, i, i, BufferSize);
			}
		}
	}
}

void UAwesomeAssetManager::SetBufferTargetByPage(const FName& LibraryName, const int32 PageIndex, const int32 PageSize, const int32 NumBufferPages)
{
	if (TSharedPtr<FItemLibrary> Library = GetLibrary(LibraryName))
	{
		const int32 StartIndex = PageIndex * PageSize;
		const int32 EndIndex = StartIndex + PageSize - 1;
		
		const int32 BufferSize = NumBufferPages * PageSize;
		SetBufferTarget(Library, StartIndex, EndIndex, BufferSize);
	}
}

void UAwesomeAssetManager::UpdateBuffer(TSharedPtr<FItemLibrary> Library)
{
	Library->Update();
}