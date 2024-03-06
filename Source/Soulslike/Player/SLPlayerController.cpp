// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SLPlayerController.h"

void ASLPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly GameOnlyInputMode;
	SetInputMode(GameOnlyInputMode);
}