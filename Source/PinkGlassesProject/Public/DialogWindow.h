// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogWindow.generated.h"

/**
 * 
 */
UCLASS()
class PINKGLASSESPROJECT_API UDialogWindow : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
		FString PhraseToDisplay;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Dialog System")
		void Show();
};
