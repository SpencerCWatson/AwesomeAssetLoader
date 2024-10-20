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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPrimaryAssetId PrimaryAssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> PrimaryAssetBundles;

	FORCEINLINE friend uint32 GetTypeHash(const FAssetLoadRequest& Key)
	{
		return HashCombine(GetTypeHash(Key.PrimaryAssetId), GetTypeHash(Key.PrimaryAssetBundles));
	}
};

/** Describes each item to be tracked and have its dependencies loaded. */
USTRUCT(BlueprintType)
struct FAwesomeAssetData
{
	GENERATED_BODY()

	/** Unique identifier for this asset. Can leave blank if assets are just referenced by list index. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UniqueId;

	/** List of assets to load*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<UObject>> AssetsToLoad;
	
	/** List of primary assets to load */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAssetLoadRequest> PrimaryAssetLoadRequests;

	/** Tags that describe this asset for filtering and number values for sorting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, float> AssetDescriptions;

	/** Delegate handles to call when the load status of this changes */
	FOnStatusChange OnStatusChange;

	/** Handle to keep the assets alive that this asset depends on */
	TSharedPtr<FStreamableHandle> LoadHandle;

	
};

UCLASS()
class AWESOMEASSETLOADER_API UItemLibrary : public UObject
{
	GENERATED_BODY()

public:

	/** Sets up initial variables */
	void Initialize(TArray<FAwesomeAssetData>&& NewAssets);

private:

	void Update();
	
	friend class UAwesomeAssetManager;

	/** All items belonging to this library */
	TArray<TSharedPtr<FAwesomeAssetData>> Items;

	/** Library items that are part of the last set filter */
	TArray<TSharedPtr<FAwesomeAssetData>> FilteredAssets;

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