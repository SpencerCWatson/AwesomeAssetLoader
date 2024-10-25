// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemLibrary.h"
#include "Engine/AssetManager.h"


void FItemLibrary::Initialize(FName LibraryName, TSet<FAssetInitializeData>&& NewAssets)
{
	Name = LibraryName;
	Items.Reserve(NewAssets.Num());

	for (auto& Asset : NewAssets)
	{
		Items.Emplace(MakeShared<FAwesomeAssetData>(MoveTemp(Asset)));
	}
}

void FItemLibrary::Update()
{
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	check(AssetManager);
	
	const auto SetupOrChangeLoad = [](const TSet<TSharedPtr<FAwesomeAssetData>>& Assets, const TAsyncLoadPriority Priority)
	{
		const UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
		check(AssetManager);
		
		for (const auto& AwesomeAssetData : Assets)
		{
			// Skip this asset if it is already loading correctly or stop loading to change priority
			if (AwesomeAssetData->LoadHandle.IsValid())
			{
				// Keep handle because it is either loading with the right priority or it is loaded
				if (AwesomeAssetData->LoadHandle->HasLoadCompleted() || AwesomeAssetData->LoadHandle->GetPriority() == Priority)
				{
					continue;
				}
				AwesomeAssetData->LoadHandle->ReleaseHandle();
			}

			// On load delegate
			FStreamableDelegate OnLoad = FStreamableDelegate::CreateLambda([AwesomeAssetData]()
				{
					if (AwesomeAssetData.IsValid())
					{
						AwesomeAssetData->OnStatusChange.ExecuteIfBound(true);
					}
				});

			// Perform actual load
			AwesomeAssetData->LoadHandle = AssetManager->GetStreamableManager().RequestAsyncLoad(AwesomeAssetData->AssetsToLoad.Array(), OnLoad);
		}
	};

	// Get requested assets
	TSet<TSharedPtr<FAwesomeAssetData>> HighPriority;
	TSet<TSharedPtr<FAwesomeAssetData>> DefaultPriority;
	GetRequestedAssets(HighPriority, DefaultPriority);

	// Load or change requested assets
	SetupOrChangeLoad(HighPriority, FStreamableManager::AsyncLoadHighPriority);
	SetupOrChangeLoad(DefaultPriority, FStreamableManager::DefaultAsyncLoadPriority);

	
	TSet<TSharedPtr<FAwesomeAssetData>> NewAssetRequest;
	NewAssetRequest.Append(MoveTemp(HighPriority));
	NewAssetRequest.Append(MoveTemp(DefaultPriority));
	

	// Unload
	const TSet<TSharedPtr<FAwesomeAssetData>> ToUnload = RequestedAssets.Difference(NewAssetRequest);
	for (const auto& AssetToUnload : ToUnload)
	{
		AssetToUnload->LoadHandle.Reset();
		AssetToUnload->OnStatusChange.ExecuteIfBound(false);
	}
	
	RequestedAssets = MoveTemp(NewAssetRequest);
}

void FItemLibrary::GetRequestedAssets(TSet<TSharedPtr<FAwesomeAssetData>>& HighPriority, TSet<TSharedPtr<FAwesomeAssetData>>& DefaultPriority)
{
	// Fill high priority
	const int32 NumItemsInBufferTargetRange = TargetStart - TargetEnd; 
	HighPriority.Empty(NumItemsInBufferTargetRange);
	
	for (int i = TargetStart; i <= TargetEnd; ++i)
	{
		if (SortedAssets.IsValidIndex(i))
		{
			HighPriority.Emplace(SortedAssets[i]);
		}
	}

	// Fill default priority
	const int32 FullRangeCount = NumItemsInBufferTargetRange + BufferSize * 2;
	DefaultPriority.Empty(FullRangeCount);
	
	// Front buffer
	for (int i = TargetStart - BufferSize; i < TargetStart; ++i)
	{
		if (SortedAssets.IsValidIndex(i))
		{
			DefaultPriority.Emplace(SortedAssets[i]);
		}
	}

	// Back buffer
	for (int i = TargetEnd + 1; i <= TargetEnd + BufferSize; ++i)
	{
		if (SortedAssets.IsValidIndex(i))
		{
			DefaultPriority.Emplace(SortedAssets[i]);
		}
	}
}