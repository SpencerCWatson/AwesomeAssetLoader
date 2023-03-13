// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemLibrary.h"



void UItemLibrary::Initialize(FName NewLibraryName, const TArray<FAwesomeAssetData>& NewAssets)
{
	UItemLibrary::LibraryName = NewLibraryName;
	UItemLibrary::Items = NewAssets;
}
