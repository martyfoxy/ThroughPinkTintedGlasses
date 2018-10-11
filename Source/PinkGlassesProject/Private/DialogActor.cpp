// Fill out your copyright notice in the Description page of Project Settings.

#include "DialogActor.h"
#include "GameFramework/Actor.h"
#include "Components/InputComponent.h"

UDialogActor::UDialogActor()
{
	PrimaryComponentTick.bCanEverTick = true;

	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	TriggerZone->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	TriggerZone->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
}

void UDialogActor::BeginPlay()
{
	Super::BeginPlay();
	
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent)
	{
		InputComponent->BindAction("Interact", IE_Released, this, &UDialogActor::StartDialog);
	}
}

void UDialogActor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UDialogActor::StartDialog()
{
}



