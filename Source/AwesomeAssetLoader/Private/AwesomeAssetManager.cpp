// Fill out your copyright notice in the Description page of Project Settings.


#include "AwesomeAssetManager.h"

DEFINE_LOG_CATEGORY(FLogAwesomeAssetManager);

bool UAwesomeAssetManager::AddAssetLibrary(FName LibraryName, TSet<FAssetInitializeData> Assets)
{
	if (!LibraryName.IsNone() && !Assets.IsEmpty())
	{
		TSharedPtr<FItemLibrary> NewLibrary = MakeShared<FItemLibrary>();
		NewLibrary->Initialize(LibraryName, MoveTemp(Assets));
		Libraries.Emplace(LibraryName, NewLibrary);
		return true;
	}

	UE_LOG(FLogAwesomeAssetManager, Warning, TEXT("Failed to add a new asset library"))
	return false;
}

void UAwesomeAssetManager::FilterAndSortAssets(
	FName LibraryName,
	const FGameplayTagContainer& MustHaveTags,
	const FGameplayTagContainer& MustNotHaveTags,
	const TArray<FGameplayTag>& SortOrder,
	FSimpleDelegate OnComplete,
	bool bSortValuesDescending,
	bool bAllowAsynchronous)
{
	TSharedPtr<FItemLibrary> Library = GetLibrary(LibraryName);
	if (!Library)
	{
		UE_LOG(FLogAwesomeAssetManager, Warning, TEXT("Failed to find asset library to sort with name: %s"), *LibraryName.ToString());
		return;
	}
	
	if (bAllowAsynchronous)
	{
		// Asynchronous
		const int32 TaskNumber = Library->TaskCounter.fetch_add(1);
		Library->SortAndFilterTask = UE::Tasks::Launch(UE_SOURCE_LOCATION, [TaskNumber, Library, MustHaveTags, MustNotHaveTags, SortOrder, bSortValuesDescending, OnComplete]()
		{
			if (Library && TaskNumber == Library->TaskCounter.load() - 1)
			{
				// todo store the values and recheck the task counter before setting.
				FScopeLock Lock(&Library->Lock);
				UAwesomeAssetManager::FilterAndSortAssetsInternal(Library, MustHaveTags, MustNotHaveTags, SortOrder, bSortValuesDescending);
				FFunctionGraphTask::CreateAndDispatchWhenReady([OnComplete]()
				{
					OnComplete.ExecuteIfBound();
				}, TStatId{}, nullptr, ENamedThreads::GameThread);
			}
		});
	}
	else
	{
		// Synchronous
		FScopeLock Lock(&Library->Lock); // Blocking
		++Library->TaskCounter;
		UAwesomeAssetManager::FilterAndSortAssetsInternal(Library, MustHaveTags, MustNotHaveTags, SortOrder, bSortValuesDescending);
		OnComplete.ExecuteIfBound();
	}
}

void UAwesomeAssetManager::K2_FilterAndSortAssets(FName LibraryName, const FGameplayTagContainer& MustHaveTags,
	const FGameplayTagContainer& MustNotHaveTags, const TArray<FGameplayTag>& SortOrder,
	FOnFilteredAndSorted OnComplete, bool bSortValuesDescending, bool bAllowAsynchronous)
{
	FilterAndSortAssets(LibraryName, MustHaveTags, MustNotHaveTags, SortOrder, FSimpleDelegate::CreateUFunction(OnComplete.GetUObject(), OnComplete.GetFunctionName()), bSortValuesDescending,bAllowAsynchronous);
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

void UAwesomeAssetManager::SetBufferTargetByIndex(FName LibraryName, const int32 AssetIndex, const int32 BufferSize)
{
	SetBufferTarget(GetLibrary(LibraryName), AssetIndex, AssetIndex, BufferSize);
}

void UAwesomeAssetManager::SetBufferTargetByUniqueId(FName LibraryName, FName UniqueId, const int32 BufferSize)
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

void UAwesomeAssetManager::SetBufferTargetByPage(FName LibraryName, const int32 PageIndex, const int32 PageSize, const int32 NumBufferPages)
{
	if (TSharedPtr<FItemLibrary> Library = GetLibrary(LibraryName))
	{
		const int32 StartIndex = PageIndex * PageSize;
		const int32 EndIndex = StartIndex + PageSize - 1;
		
		const int32 BufferSize = NumBufferPages * PageSize;
		SetBufferTarget(Library, StartIndex, EndIndex, BufferSize);
	}
}

void UAwesomeAssetManager::FilterAndSortAssetsInternal(TSharedPtr<FItemLibrary> Library,
	const FGameplayTagContainer& MustHaveTags, const FGameplayTagContainer& MustNotHaveTags,
	const TArray<FGameplayTag>& SortOrder, bool bSortValuesDescending)
{
	FScopeLock Lock(&Library->Lock);
	// Filter
	if (MustHaveTags != Library->MushHaveTagsCache || MustNotHaveTags != Library->MushNotHaveTagsCache)
	{
		TSet<TSharedPtr<FAwesomeAssetData>> FilteredAssets;
		FilteredAssets.Reserve(Library->Items.Num());
		
		Library->MushHaveTagsCache = MustHaveTags;
		Library->MushNotHaveTagsCache = MustNotHaveTags;
		for (const auto& Item : Library->Items)
		{
			// This might be a good place for a gameplay tag query 
			if (Item->CachedAssetDescriptionTags.HasAll(MustHaveTags) && !Item->CachedAssetDescriptionTags.HasAny(MustNotHaveTags))
			{
				FilteredAssets.Emplace(Item);
			}
		}
		Library->FilteredAssets = MoveTemp(FilteredAssets);
		UE_LOG(FLogAwesomeAssetManager, Verbose, TEXT("Library %s has %i items after being filtered"), *Library->Name.ToString(), Library->FilteredAssets.Num())
	}

	// Sort
	// todo get rid of moving the data and just sort it from the beginning.
	Library->SortedAssets.Reset(Library->FilteredAssets.Num()); 
	TArray<TArray<TSharedPtr<FAwesomeAssetData>>> SortBuckets;
	SortBuckets.Init(TArray<TSharedPtr<FAwesomeAssetData>>(), SortOrder.Num());
	
	for (const auto& Item : Library->FilteredAssets)
	{
		for (int32 i = 0; i < SortOrder.Num(); ++i)
		{
			// If current asset contains current tag.
			if (Item->AssetDescriptions.Contains(SortOrder[i]))
			{
				SortBuckets[i].Emplace(Item);
				break;
			}
		}
	}
	
	// Sort by values in buckets.
	for (int32 i = 0; i < SortBuckets.Num(); ++i)
	{
		const FGameplayTag ThisBucketsTag = SortOrder[i];

		if (bSortValuesDescending)
		{
			SortBuckets[i].Sort([ThisBucketsTag](const TSharedPtr<FAwesomeAssetData>& A, const TSharedPtr<FAwesomeAssetData>& B)
			{
				return A->AssetDescriptions.FindChecked(ThisBucketsTag) > B->AssetDescriptions.FindChecked(ThisBucketsTag);
			});
		}
		else
		{
			SortBuckets[i].Sort([ThisBucketsTag](const TSharedPtr<FAwesomeAssetData>& A, const TSharedPtr<FAwesomeAssetData>& B)
			{
				return A->AssetDescriptions.FindChecked(ThisBucketsTag) < B->AssetDescriptions.FindChecked(ThisBucketsTag);
			});
		}
		
		Library->SortedAssets.Append(MoveTemp(SortBuckets[i]));
	}
}

void UAwesomeAssetManager::UpdateBuffer(TSharedPtr<FItemLibrary> Library)
{
	Library->Update();
}
