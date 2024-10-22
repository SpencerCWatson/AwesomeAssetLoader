// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AwesomeAssetManager.h"
#include "ItemLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AAMBlueprintHelpers.generated.h"


/**
 * 
 */
UCLASS()
class AWESOMEASSETLOADER_API UAAMBlueprintHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category="AwesomeAssetLoader")
	static void BindDelegate(UPARAM(ref) FAssetInitializeData& InitializeData, FK2_OnStatusChange OnStatusChange);
};
