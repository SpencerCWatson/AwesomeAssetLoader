// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AwesomeAssetManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AAMBlueprintHelpers.generated.h"


/**
 * 
 */
UCLASS()
class AWESOMEASSETLOADER_API UAAMBlueprintHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	static void BindDelegate(const FAssetInitializeData& InitializeData, const FK2_OnStatusChange& OnStatusChange);
};
