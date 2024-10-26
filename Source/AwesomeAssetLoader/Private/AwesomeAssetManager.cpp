// Fill out your copyright notice in the Description page of Project Settings.


#include "AwesomeAssetManager.h"
#include "Async/TaskGraphInterfaces.h"


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

void UAwesomeAssetManager::FilterAndSortAssets(FName LibraryName, const FFilterAndSortCriterion& Criterion, FSimpleDelegate OnComplete, bool bAllowAsynchronous)
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
		const int32 TaskNumber = Library->TaskCounter.fetch_add(1) + 1;
		Library->SortAndFilterTask = UE::Tasks::Launch(UE_SOURCE_LOCATION, [TaskNumber, Library, Criterion, OnComplete]()
		{
			if (Library && TaskNumber == Library->TaskCounter.load()) // Still relevant?
			{
				// Lock just to copy the current values;
				Library->Lock.Lock();
				TSet<TSharedPtr<FAwesomeAssetData>> Items = Library->Items;
				const FGameplayTagContainer MustHaveTagsCache = Library->MustHaveTagsCache;
				const FGameplayTagContainer MustNotHaveTagsCache = Library->MustNotHaveTagsCache;
				Library->Lock.Unlock();
				
				TSet<TSharedPtr<FAwesomeAssetData>> FilteredAssets;
				TArray<TSharedPtr<FAwesomeAssetData>> SortedAssets;
				FilterAndSortAssetsInternal(Items, MustHaveTagsCache, MustNotHaveTagsCache, Criterion, FilteredAssets, SortedAssets);

				// Check if result is still relevant
				if (TaskNumber == Library->TaskCounter.load())
				{
					FScopeLock Lock(&Library->Lock);
					Library->FilteredAssets = MoveTemp(FilteredAssets);
					Library->SortedAssets = MoveTemp(SortedAssets);
					
					FFunctionGraphTask::CreateAndDispatchWhenReady([Library, TaskNumber, OnComplete]()
					{
						// Only call if still relevant
						if (Library && TaskNumber == Library->TaskCounter.load())
						{
							OnComplete.ExecuteIfBound();
						}
					}, TStatId{}, nullptr, ENamedThreads::GameThread);
				}
			}
		});
	}
	else
	{
		// Synchronous
		FScopeLock Lock(&Library->Lock); // Blocking
		++Library->TaskCounter;
		FilterAndSortAssetsInternal(Library->Items, Library->MustHaveTagsCache, Library->MustNotHaveTagsCache, Criterion, Library->FilteredAssets, Library->SortedAssets );
		Library->MustHaveTagsCache = Criterion.MustHaveTags;
		Library->MustNotHaveTagsCache = Criterion.MustNotHaveTags;
		UE_LOG(FLogAwesomeAssetManager, Verbose, TEXT("Library %s has %i items after being filtered"), *Library->Name.ToString(), Library->FilteredAssets.Num())
		OnComplete.ExecuteIfBound();
	}
}

void UAwesomeAssetManager::K2_FilterAndSortAssets(FName LibraryName, const FFilterAndSortCriterion& Criterion, FOnFilteredAndSorted OnComplete, bool bAllowAsynchronous)
{
	FilterAndSortAssets(LibraryName, Criterion, FSimpleDelegate::CreateUFunction(OnComplete.GetUObject(), OnComplete.GetFunctionName()), bAllowAsynchronous);
}

bool UAwesomeAssetManager::GetSortedAssets(FName LibraryName, TArray<FName>& SortedAssets)
{
	const TSharedPtr<FItemLibrary> Library = GetLibrary(LibraryName);
	if (!Library)
	{
		UE_LOG(FLogAwesomeAssetManager, Log, TEXT("Failed to find a library of name: %s"), *LibraryName.ToString())
		return false;
	}

	verify(Library->SortAndFilterTask.Wait());
	FScopeLock Lock(&Library->Lock);
	SortedAssets.Empty(Library->SortedAssets.Num());
	for (const auto Asset : Library->SortedAssets)
	{
		if (Asset.IsValid())
		{
			SortedAssets.Emplace(Asset->UniqueId);
		}
	}
	
	return true;
}

bool UAwesomeAssetManager::SetBufferTarget(TSharedPtr<FItemLibrary> Library, const int32 TargetStart, const int32 TargetEnd, const int32 BufferSize)
{
	if (Library)
	{
		Library->BufferSize = BufferSize;
		Library->TargetStart = TargetStart;
		Library->TargetEnd = TargetEnd;
		UpdateBuffer(Library);
		return true;
	}
	
	return false;
}

bool UAwesomeAssetManager::SetBufferTargetByIndex(FName LibraryName, const int32 AssetIndex, const int32 CoreExtent, const int32 BufferSize)
{
	const TSharedPtr<FItemLibrary> Library = GetLibrary(LibraryName);
	if (!Library)
	{
		UE_LOG(FLogAwesomeAssetManager, Log, TEXT("Failed to find a library of name: %s"), *LibraryName.ToString())
		return false;
	}

	const int32 StartTarget = AssetIndex - CoreExtent < 0 ? 0 : AssetIndex - CoreExtent;
	const int32 EndTarget = AssetIndex + CoreExtent >= Library->SortedAssets.Num() ? Library->SortedAssets.Num() - 1 : AssetIndex + CoreExtent;
	return SetBufferTarget(Library, StartTarget, EndTarget, BufferSize);
}

bool UAwesomeAssetManager::SetBufferTargetByUniqueId(FName LibraryName, FName UniqueId, const int32 CoreExtent, const int32 BufferSize)
{
	const TSharedPtr<FItemLibrary> Library = GetLibrary(LibraryName);
	if (!Library)
	{
		UE_LOG(FLogAwesomeAssetManager, Log, TEXT("Failed to find a library of name: %s"), *LibraryName.ToString())
		return false;
	}
	
	for (int i = 0; i < Library->SortedAssets.Num(); ++i)
	{
		if (Library->SortedAssets[i]->UniqueId == UniqueId)
		{
			const int32 StartTarget = i - CoreExtent < 0 ? 0 : i - CoreExtent;
			const int32 EndTarget = i + CoreExtent > Library->SortedAssets.Num() - 1 ? Library->SortedAssets.Num() - 1 : i + CoreExtent;
			SetBufferTarget(Library, StartTarget, EndTarget, BufferSize);
			return true;
		}
	}
	
	return false;
}

bool UAwesomeAssetManager::SetBufferTargetByPage(FName LibraryName, const int32 PageIndex, const int32 PageSize, const int32 NumBufferPages)
{
	const TSharedPtr<FItemLibrary> Library = GetLibrary(LibraryName);

	if (!Library)
	{
		UE_LOG(FLogAwesomeAssetManager, Log, TEXT("Failed to find a library of name: %s"), *LibraryName.ToString())
		return false;
	}
	
	const int32 StartIndex = PageIndex * PageSize;
	const int32 EndIndex = StartIndex + PageSize - 1;
	
	const int32 BufferSize = NumBufferPages * PageSize;
	return SetBufferTarget(Library, StartIndex, EndIndex, BufferSize);
}

void UAwesomeAssetManager::FilterAndSortAssetsInternal(const TSet<TSharedPtr<FAwesomeAssetData>>& Items, const FGameplayTagContainer& MushHaveTagsCache, const FGameplayTagContainer& MushNotHaveTagsCache, const FFilterAndSortCriterion& Criterion, TSet<TSharedPtr<FAwesomeAssetData>>& FilteredAssets, TArray<TSharedPtr<FAwesomeAssetData>>& SortedAssets)
{
	// Filter
	if (Criterion.MustHaveTags != MushHaveTagsCache || Criterion.MustNotHaveTags != MushNotHaveTagsCache)
	{
		FilteredAssets.Reserve(Items.Num());
		
		for (const auto& Item : Items)
		{
			// This might be a good place for a gameplay tag query 
			if (Item->CachedAssetDescriptionTags.HasAll(Criterion.MustHaveTags) && !Item->CachedAssetDescriptionTags.HasAny(Criterion.MustNotHaveTags))
			{
				FilteredAssets.Emplace(Item);
			}
		}
		FilteredAssets = MoveTemp(FilteredAssets);
	}
	else
	{
		UE_LOG(FLogAwesomeAssetManager, Verbose, TEXT("Filter caches match new filter request. Skipping filtering"))
	}

	// Sort
	// todo get rid of moving the data and just sort it from the beginning.
	SortedAssets.Reset(FilteredAssets.Num()); 
	TArray<TArray<TSharedPtr<FAwesomeAssetData>>> SortBuckets;
	SortBuckets.Init(TArray<TSharedPtr<FAwesomeAssetData>>(), Criterion.SortOrder.Num());
	
	for (const auto& Item : FilteredAssets)
	{
		for (int32 i = 0; i < Criterion.SortOrder.Num(); ++i)
		{
			// If current asset contains current tag.
			if (Item->AssetDescriptions.Contains(Criterion.SortOrder[i]))
			{
				SortBuckets[i].Emplace(Item);
				break;
			}
		}
	}
	
	// Sort by values in buckets.
	for (int32 i = 0; i < SortBuckets.Num(); ++i)
	{
		const FGameplayTag ThisBucketsTag = Criterion.SortOrder[i];

		if (Criterion.bSortValuesDescending)
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
		
		SortedAssets.Append(MoveTemp(SortBuckets[i]));
	}
}

void UAwesomeAssetManager::UpdateBuffer(TSharedPtr<FItemLibrary> Library)
{
	Library->Update();
}
