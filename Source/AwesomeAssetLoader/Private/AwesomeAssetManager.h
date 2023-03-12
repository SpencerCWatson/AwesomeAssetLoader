// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AssetContainer.h"
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
	virtual void Initialize(FSubsystemCollectionBase& Collection) override {}
	/** Deconstruction */
	virtual void Deinitialize() override {};
	
private:
	
	/** Stores the libraries of assets by name to find easier later */
	UPROPERTY()
	TMap<FName, UAssetContainer*> Libraries;

public:
	/**
	 * @brief Add a new library of assets to manage
	 * @param LibraryName The name that the library should be referenced by.
	 * @param Assets Assets to be tracked by the newly created library.
	 * @return 
	 */
	UFUNCTION(BlueprintCallable)
	bool AddAssetLibrary(FName LibraryName, const TArray<FAwesomeAssetData>& Assets);

	/**
	 * @brief Dump an asset library by name.
	 * @param LibraryName The library that should be dumped.
	 */
	UFUNCTION(BlueprintCallable)
	void DumpAssetLibrary(FName LibraryName) {Libraries.Remove(LibraryName);}

	/**
	 * @brief Filter library down to only the assets that match the gameplay tag container. 
	 * @param LibraryName 
	 * @param Filter 
	 */
	UFUNCTION(BlueprintCallable)
	void FilterAssets(FName LibraryName, FGameplayTagContainer Filter);

	/** Sort assets based on the order of the supplied gameplay tags array */
	UFUNCTION(BlueprintCallable)
	void SortAssets(FName LibraryName, TArray<FGameplayTag> Order);
	
	UFUNCTION(BlueprintCallable)
	void SetBufferType(FName LibraryName, EBufferType BufferType);

	void SetBufferTarget(FName LibraryName, FName AssetId);
	
	
private:
	
	UAssetContainer* GetAssetLibrary(FName LibraryName);
	
};
