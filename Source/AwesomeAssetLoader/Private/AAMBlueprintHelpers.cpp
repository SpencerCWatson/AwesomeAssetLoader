// Fill out your copyright notice in the Description page of Project Settings.


#include "AAMBlueprintHelpers.h"

void UAAMBlueprintHelpers::BindDelegate(FAssetInitializeData& InitializeData, FK2_OnStatusChange OnStatusChange)
{
	InitializeData.OnStatusChange.BindUFunction(OnStatusChange.GetUObject(), OnStatusChange.GetFunctionName());
}