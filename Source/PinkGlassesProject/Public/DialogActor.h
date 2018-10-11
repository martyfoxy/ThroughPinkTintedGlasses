// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Components/BoxComponent.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogActor.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PINKGLASSESPROJECT_API UDialogActor : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDialogActor();

protected:
	virtual void BeginPlay() override;
	
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
		UBoxComponent* TriggerZone;

private:
	UInputComponent* InputComponent;

	void StartDialog();
};
