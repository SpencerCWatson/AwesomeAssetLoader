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
 * Setting buffer target is not thread safe. Block if Task running?
 * Add logging
 * Handle so that libraries do not persist between level changes.
 * Add profiling
 * Add a way to supply an ordered list and keep it ordered.
 */


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
	 * @param MustHaveTags 
	 * @param MustNotHaveTags 
	 * @param SortOrder
	 * @param OnComplete 
	 * @param bSortValuesDescending
	 * @param bAllowAsynchronous 
	 */
	void FilterAndSortAssets(FName LibraryName, const FGameplayTagContainer& MustHaveTags, const FGameplayTagContainer& MustNotHaveTags, const TArray<FGameplayTag>& SortOrder, FSimpleDelegate OnComplete, bool bSortValuesDescending = false, bool bAllowAsynchronous = false);
	DECLARE_DYNAMIC_DELEGATE(FOnFilteredAndSorted);
	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader", DisplayName=FilterAndSortAssets, meta=(AutoCreateRefTerm="MustHaveTags,MustNotHaveTags"))
	void K2_FilterAndSortAssets(FName LibraryName, const FGameplayTagContainer& MustHaveTags, const FGameplayTagContainer& MustNotHaveTags, const TArray<FGameplayTag>& SortOrder, FOnFilteredAndSorted OnComplete, bool bSortValuesDescending = false, bool bAllowAsynchronous = false);
	
	void SetBufferTarget(TSharedPtr<FItemLibrary> Library, int32 TargetStart, int32 TargetEnd, const int32 BufferSize);

	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	void SetBufferTargetByIndex(FName LibraryName, const int32 AssetIndex, const int32 BufferSize);

	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	void SetBufferTargetByUniqueId(FName LibraryName, FName UniqueId, const int32 BufferSize);

	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	void SetBufferTargetByPage(FName LibraryName, const int32 PageIndex, const int32 PageSize, const int32 NumBufferPages);
	
private:

	static void FilterAndSortAssetsInternal(TSharedPtr<FItemLibrary> Library, const FGameplayTagContainer& MustHaveTags, const FGameplayTagContainer& MustNotHaveTags, const TArray<FGameplayTag>& SortOrder, bool bSortValuesDescending = false);
	
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