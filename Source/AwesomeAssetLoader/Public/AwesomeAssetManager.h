// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemLibrary.h"
#include "GameplayTagContainer.h"
#include "UObject/PrimaryAssetId.h"
#include "AwesomeAssetManager.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FK2_OnStatusChange, bool, bShouldLoad);


/**
 * ~~~~ TODOs ~~~~
 * Should the sort order and the filter just be remembered and everything ran again after a change? Also, should support ordered list...
 * BP actually needs to be able to set the target... lol
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
	bool AddAssetLibrary(const FName& LibraryName, TSet<FAssetInitializeData> Assets);
	
	/**
	 * Dump an asset library by name.
	 * @param LibraryName		The library that should be dumped.
	 */
	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	void RemoveAssetLibrary(const FName& LibraryName) { Libraries.Remove(LibraryName); }

	/**
	 * Filter library down to only the assets that match the gameplay tag container. 
	 * @param LibraryName		Name of the library to apply the filter to.
	 * @param Filter			Container of tags to filter down the full asset list by.
	 */
	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	void FilterAssets(const FName& LibraryName, const FGameplayTagContainer& Filter);

	/**
	 * Sort assets based on the order of the supplied gameplay tags array.
	 * Items are sorted their tags and then further sorted by the float value associated with the tag in each group.
	 * @param LibraryName			Library to apply the sort to.
	 * @param Order					Array of tags that define the sort order.
	 * @param bSortValuesDescending	False to sort items by tag value in descending order within each tag group.
	 */
	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	void SortAssets(const FName& LibraryName, const TArray<FGameplayTag>& Order, bool bSortValuesDescending = false);

	void SetBufferTarget(TSharedPtr<FItemLibrary> Library, int32 TargetStart, int32 TargetEnd, const int32 BufferSize);

	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	void SetBufferTargetByIndex(const FName& LibraryName, const int32 AssetIndex, const int32 BufferSize);

	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	void SetBufferTargetByUniqueId(const FName& LibraryName, const FName& UniqueId, const int32 BufferSize);

	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	void SetBufferTargetByPage(const FName& LibraryName, const int32 PageIndex, const int32 PageSize, const int32 NumBufferPages);
	
private:
	
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