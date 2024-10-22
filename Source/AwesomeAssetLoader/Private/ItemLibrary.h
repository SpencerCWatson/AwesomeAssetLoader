// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "ItemLibrary.generated.h"

DECLARE_DELEGATE_OneParam(FOnStatusChange, const bool /*ShouldLoad*/);

/** Describes the primary asset to load and the required bundles */
USTRUCT(BlueprintType)
struct FAssetLoadRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Primary Asset Data")
	FPrimaryAssetId PrimaryAssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Primary Asset Data")
	TArray<FName> PrimaryAssetBundles;

	FORCEINLINE friend uint32 GetTypeHash(const FAssetLoadRequest& Key)
	{
		return HashCombine(GetTypeHash(Key.PrimaryAssetId), GetTypeHash(Key.PrimaryAssetBundles));
	}
};

USTRUCT(Blueprintable)
struct FAssetInitializeData
{
	GENERATED_BODY()

	/** Unique identifier for this asset. Can leave blank if assets are just referenced by list index. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Init Data")
	FName UniqueId;

	/** List of assets to load*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Init Data")
	TSet<FSoftObjectPath> SoftObjectPaths;

	/** Tags that describe this asset for filtering and number values for sorting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Init Data")
	TMap<FGameplayTag, float> AssetDescriptions;

	/** Delegate handles to call when the load status of this changes */
	FOnStatusChange OnStatusChange;

	FORCEINLINE friend uint32 GetTypeHash(const FAssetInitializeData& Key)
	{
		uint32 Hash = 0;
		Hash = HashCombineFast(GetTypeHash(Key.UniqueId), GetTypeHash(static_cast<uint32>(Key.SoftObjectPaths.Num())));
		for (const auto& Path : Key.SoftObjectPaths)
		{
			Hash = HashCombine(Hash, GetTypeHash(Path));
		}
		for (const auto& AssetDescription : Key.AssetDescriptions)
		{
			Hash = HashCombine(Hash, GetTypeHash(AssetDescription));
		}
		return HashCombine(Hash, GetTypeHash(Key.OnStatusChange.GetHandle()));
	}
};

/** Describes each item to be tracked and have its dependencies loaded. */
struct FAwesomeAssetData
{
	FAwesomeAssetData() = delete;
	FAwesomeAssetData(const FAwesomeAssetData&) = delete;
	
	explicit FAwesomeAssetData(FAssetInitializeData&& InitData)
	{
		UniqueId = InitData.UniqueId;
		AssetsToLoad = MoveTemp(InitData.SoftObjectPaths);
		AssetDescriptions = MoveTemp(InitData.AssetDescriptions);
		OnStatusChange = MoveTemp(InitData.OnStatusChange);
	}

	/** Unique identifier for this asset. Can leave blank if assets are just referenced by list index. */
	FName UniqueId;

	/** List of assets to load*/
	TSet<FSoftObjectPath> AssetsToLoad;

	/** Tags that describe this asset for filtering and number values for sorting. */
	TMap<FGameplayTag, float> AssetDescriptions;

	/** Delegate handles to call when the load status of this changes */
	FOnStatusChange OnStatusChange;

	/** Handle to keep the assets alive that this asset depends on */
	TSharedPtr<FStreamableHandle> LoadHandle;
};


class FItemLibrary : public TSharedFromThis<FItemLibrary>
{
public:

	/** Sets up initial variables */
	void Initialize(TSet<FAssetInitializeData>&& NewAssets);

private:

	void Update();
	
	friend class UAwesomeAssetManager;

	/** All items belonging to this library */
	TSet<TSharedPtr<FAwesomeAssetData>> Items;

	/** Library items that are part of the last set filter */
	TSet<TSharedPtr<FAwesomeAssetData>> FilteredAssets;

	/** The sorted filtered items */
	TArray<TSharedPtr<FAwesomeAssetData>> SortedAssets;

	
	//~~~~ For the buffer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	
	TSet<TSharedPtr<FAwesomeAssetData>> RequestedAssets;

	/** Number of assets above and bellow the target range to load. this is a default priority load */
	int32 BufferSize = 0;

	/* The target range to load around. This is a high priority load */
	int32 TargetStart = 0;
	int32 TargetEnd = 0;

	/** Returns all the assets that should be loaded */
	void GetRequestedAssets(TSet<TSharedPtr<FAwesomeAssetData>>& HighPriority, TSet<TSharedPtr<FAwesomeAssetData>>& DefaultPriority);
};