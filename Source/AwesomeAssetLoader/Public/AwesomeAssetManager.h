// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemLibrary.h"
#include "GameplayTagContainer.h"
#include "UObject/PrimaryAssetId.h"
#include "AwesomeAssetManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(FLogAwesomeAssetManager, Log, All);
DECLARE_DYNAMIC_DELEGATE_OneParam(FK2_OnStatusChange, bool, bShouldLoad);


/**
 * ~~~~ TODOs ~~~~
 * Handle so that libraries do not persist between level changes.
 * Add profiling
 * Add a way to supply an ordered list and keep it ordered.
 * Should asset data take in an arbitrary set of pointers to give back when asked for the sorted items? if this more useful than the unique Ids?
 */

USTRUCT(Blueprintable, BlueprintType)
struct FFilterAndSortCriterion
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Criteria)
	FGameplayTagContainer MustHaveTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Criteria)
	FGameplayTagContainer MustNotHaveTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Criteria)
	TArray<FGameplayTag> SortOrder;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Criteria)
	bool bSortValuesDescending = false;
};

/**
 * 
 */
UCLASS()
class AWESOMEASSETLOADER_API UAwesomeAssetManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	/**
	 * Add a new library of assets to manage.
	 * If a library with the same name already exists it will be replaced.
	 * @param LibraryName		The name that the library should be referenced by.
	 * @param Assets			Assets to be tracked by the newly created library.
	 * @return					Was successful.
	 */
	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	bool AddAssetLibrary(FName LibraryName, TSet<FAssetInitializeData> Assets);
	
	/**
	 * Dump an asset library by name.
	 * @param LibraryName		The library that should be dumped.
	 */
	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	void RemoveAssetLibrary(FName LibraryName) { Libraries.Remove(LibraryName); }

	/**
	 * 
	 * @param LibraryName
	 * @param Criterion 
	 * @param OnComplete 
	 * @param bAllowAsynchronous 
	 */
	void FilterAndSortAssets(FName LibraryName, const FFilterAndSortCriterion& Criterion , FSimpleDelegate OnComplete, bool bAllowAsynchronous = false);
	DECLARE_DYNAMIC_DELEGATE(FOnFilteredAndSorted);
	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader", DisplayName=FilterAndSortAssets, meta=(AutoCreateRefTerm="MustHaveTags,MustNotHaveTags"))
	void K2_FilterAndSortAssets(FName LibraryName, const FFilterAndSortCriterion& Criterion, FOnFilteredAndSorted OnComplete, bool bAllowAsynchronous = false);

	/**
	 * Return the filtered and sorted assets by their unique Id
	 * @return Success
	 */
	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	bool GetSortedAssets(FName LibraryName, TArray<FName>& SortedAssets);

	bool SetBufferTarget(TSharedPtr<FItemLibrary> Library, int32 TargetStart, int32 TargetEnd, const int32 BufferSize);

	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	bool SetBufferTargetByIndex(FName LibraryName, const int32 AssetIndex, int32 CoreExtent, const int32 BufferSize);

	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	bool SetBufferTargetByUniqueId(FName LibraryName, FName UniqueId, int32 CoreExtent, const int32 BufferSize);

	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	bool SetBufferTargetByPage(FName LibraryName, const int32 PageIndex, const int32 PageSize, const int32 NumBufferPages);
	
private:

	static void FilterAndSortAssetsInternal(const TSet<TSharedPtr<FAwesomeAssetData>>& Items, const ::FGameplayTagContainer& MushHaveTagsCache, const ::
	                                        FGameplayTagContainer& MushNotHaveTagsCache, const FFilterAndSortCriterion& Criterion, TSet<TSharedPtr<FAwesomeAssetData>>& FilteredAssets, TArray<TSharedPtr<FAwesomeAssetData>>& SortedAssets);
	
	/** Get the library by name */
	FORCEINLINE TSharedPtr<FItemLibrary> GetLibrary(const FName& LibraryName)
	{
		TSharedPtr<FItemLibrary>* LibraryPointer = Libraries.Find(LibraryName);
		return LibraryPointer ? *LibraryPointer : nullptr;
	}

	/** Called internally to update the buffer of the library. */
	void UpdateBuffer(TSharedPtr<FItemLibrary> Library);
	
	/** Stores the libraries of assets by name to find easier later */
	TMap<FName, TSharedPtr<FItemLibrary>> Libraries;
};