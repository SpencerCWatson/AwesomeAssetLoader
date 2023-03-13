// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "ItemLibrary.generated.h"

/** Describes the primary asset to load and the required bundles */
USTRUCT(BlueprintType)
struct FPrimaryAssetLoadRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPrimaryAssetId PrimaryAssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> PrimaryAssetBundles;
};

/** Describes each item to be tracked and have it's dependencies loaded. */
USTRUCT(BlueprintType)
struct FAwesomeAssetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UniqueId;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPrimaryAssetLoadRequest> FPrimaryAssetLoadRequests;

	// Tags that describe this asset for filtering and number values for sorting.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, float> AssetDescriptions;
};

UENUM(BlueprintType)
enum class EBufferType : uint8
{
	Page,
	Range
};

UCLASS()
class AWESOMEASSETLOADER_API UItemLibrary : public UObject
{
	GENERATED_BODY()

	UItemLibrary(){};

public:

	/** Sets up initial variables */
	void Initialize(FName NewLibraryName, const TArray<FAwesomeAssetData>& NewAssets);


private:
	
	friend class UAwesomeAssetManager;
	
	FName LibraryName;
	TArray<FAwesomeAssetData> Items;

	EBufferType BufferType;

	// This is left and right buffer amount if type is range or the amount of pages to load beside the target one.
	int32 BufferSize;

	// Size of a buffer page.
	int32 BufferPageSize;
	
	TArray<FAwesomeAssetData*> FilteredAssets;

	TArray<FAwesomeAssetData*> SortedAssets;

	/** Same items as SortedAssets just in Id form. */
	TArray<FName> SortedAssetIds;


	//~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~ For the buffer ~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~

	TArray<FName> CurrentLoadedIds;
	
	// The target item load around.
	FName BufferItemTarget;
	// The target page that we want to load. Zero based
	int32 BufferPageTarget;
};
