// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PinkGlassesProjectCharacter.generated.h"

UCLASS(config=Game)
class APinkGlassesProjectCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APinkGlassesProjectCharacter();

	void ChangeForwardVector(FVector Direction);

	void ChangeRightVector(FVector Direction);

private:
	FVector MovementForwardVector;
	FVector MovementRightVector;

protected:
	//Движение вперед/назад
	void MoveForward(float Value);

	//Движение влево/вправо
	void MoveRight(float Value);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};

