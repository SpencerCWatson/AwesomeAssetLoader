// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemLibrary.h"
#include "GameplayTagContainer.h"
#include "UObject/PrimaryAssetId.h"
#include "AwesomeAssetManager.generated.h"


/**
 * ~~~~ TODOs ~~~~
 * Handle so that libraries do not persist between level changes.
 * We do not need to add a library with that big struct. Only necessary data.
 * Add profiling
 */


/**
 * 
 */
UCLASS()
class AWESOMEASSETLOADER_API UAwesomeAssetManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	/** Construction */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	/** Deconstruction */
	virtual void Deinitialize() override {};
	
	/**
	 * Add a new library of assets to manage.
	 * If a library with the same name already exists it will be replaced.
	 * @param LibraryName		The name that the library should be referenced by.
	 * @param Assets			Assets to be tracked by the newly created library.
	 * @return					Was successful.
	 */
	UFUNCTION(BlueprintCallable)
	bool AddAssetLibrary(const FName& LibraryName, TArray<FAwesomeAssetData> Assets);
	
	/**
	 * Dump an asset library by name.
	 * @param LibraryName		The library that should be dumped.
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveAssetLibrary(const FName& LibraryName) { Libraries.Remove(LibraryName); }

	/**
	 * Filter library down to only the assets that match the gameplay tag container. 
	 * @param LibraryName		Name of the library to apply the filter to.
	 * @param Filter			Container of tags to filter down the full asset list by.
	 */
	UFUNCTION(BlueprintCallable)
	void FilterAssets(const FName& LibraryName, const FGameplayTagContainer& Filter);

	/**
	 * Sort assets based on the order of the supplied gameplay tags array.
	 * Items are sorted their tags and then further sorted by the float value associated with the tag in each group.
	 * @param LibraryName			Library to apply the sort to.
	 * @param Order					Array of tags that define the sort order.
	 * @param bSortValuesDescending	False to sort items by tag value in descending order within each tag group.
	 */
	UFUNCTION(BlueprintCallable)
	void SortAssets(const FName& LibraryName, const TArray<FGameplayTag>& Order, bool bSortValuesDescending = false);

	void SetBufferTarget(UItemLibrary* Library, int32 TargetStart, int32 TargetEnd, const int32 BufferSize);
	void SetBufferTarget(const FName& LibraryName, const int32 AssetIndex, const int32 BufferSize);
	void SetBufferTarget(const FName& LibraryName, const FName& UniqueId, const int32 BufferSize);
	void SetBufferTarget(const FName& LibraryName, const int32 PageIndex, const int32 PageSize, const int32 NumBufferPages);
	
private:
	
	/** Get the library by name */
	FORCEINLINE UItemLibrary* GetLibrary(const FName& LibraryName)
	{
		UItemLibrary** LibraryPointer = Libraries.Find(LibraryName);
		return LibraryPointer ? *LibraryPointer : nullptr;
	}

	/** Called internally to update the buffer of the library. */
	void UpdateBuffer(UItemLibrary* Library);
	
	/** Stores the libraries of assets by name to find easier later */
	UPROPERTY()
	TMap<FName, UItemLibrary*> Libraries;
};