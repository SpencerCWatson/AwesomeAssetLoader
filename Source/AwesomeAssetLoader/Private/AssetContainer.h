// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "AssetContainer.generated.h"

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

	// Tags that describe this asset for filtering and sorting.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGameplayTagContainer> AssetDescriptions;
};

UENUM(BlueprintType)
enum class EBufferType : uint8
{
	Page,
	Range
};

UCLASS()
class AWESOMEASSETLOADER_API UAssetContainer : public UObject
{
	GENERATED_BODY()

	UAssetContainer(){};

public:
	void Initialize(FName NewLibraryName, const TArray<FAwesomeAssetData>& NewAssets);


private:
	friend class UAwesomeAssetManager;
	
	FName LibraryName;
	TArray<FAwesomeAssetData> Assets;

	EBufferType BufferType;
	
	TArray<FAwesomeAssetData> FilteredAssets;

	TArray<FAwesomeAssetData> SortedAssets;
	
	// Map for faster finding of an asset's sorted index by name.
	TMap<FName, int32> SortedAssetsTable;
	
	
};
