// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemLibrary.h"
#include "GameplayTagContainer.h"
#include "UObject/PrimaryAssetId.h"
#include "AwesomeAssetManager.generated.h"



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
	
private:

	// Asset manager
	UPROPERTY()
	UAssetManager* AssetManager;
	
	/** Stores the libraries of assets by name to find easier later */
	UPROPERTY()
	TMap<FName, UItemLibrary*> Libraries;

public:
	/**
	 * @brief Add a new library of assets to manage.
	 * If a library with the same name already exists it will be replaced.
	 * @param LibraryName	The name that the library should be referenced by.
	 * @param Assets		Assets to be tracked by the newly created library.
	 * @return				Was successful?
	 */
	UFUNCTION(BlueprintCallable)
	bool AddAssetLibrary(FName LibraryName, const TArray<FAwesomeAssetData>& Assets);

	/**
	 * @brief Dump an asset library by name.
	 * @param LibraryName	The library that should be dumped.
	 */
	UFUNCTION(BlueprintCallable)
	void DumpAssetLibrary(FName LibraryName) {Libraries.Remove(LibraryName);}

	/**
	 * @brief Filter library down to only the assets that match the gameplay tag container. 
	 * @param LibraryName	Name of the library to apply the filter to.
	 * @param Filter		Container of tags to filter down the full asset list by.
	 */
	UFUNCTION(BlueprintCallable)
	void FilterAssets(FName LibraryName, const FGameplayTagContainer& Filter);

	/**
	 * @brief Sort assets based on the order of the supplied gameplay tags array.
	 * Items are sorted their tags and then further sorted by the float value associated with the tag in each group.
	 * @param LibraryName			Library to apply the sort to.
	 * @param Order					Array of tags that define the sort order.
	 * @param bSortValueAscending	False to sort items by tag value in descending order within each tag group.
	 */
	UFUNCTION(BlueprintCallable)
	void SortAssets(FName LibraryName, const TArray<FGameplayTag>& Order, bool bSortValueAscending = true);

	/**
	 * @brief Set the buffer type on the named library
	 * @param LibraryName Name of the library to set the buffer type for.
	 * @param BufferType What type of buffer we want to use for loading assets.
	 */
	UFUNCTION(BlueprintCallable)
	void SetBufferType(FName LibraryName, EBufferType BufferType);

	void SetBufferTarget(FName LibraryName, FName AssetId);
	void SetBufferTarget(FName LibraryName, int32 Page);
	
private:
	/** Get the library by name */
	UItemLibrary* GetAssetLibrary(FName LibraryName);

	/** Called internally to update the buffer of the library. */
	void UpdateBuffer(UItemLibrary* Library);

	void CalculateBufferLoadIndexes(UItemLibrary* Library, int32& StartIndex, int32& EndIndex);

	
};
