// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SLAnimationAttackInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USLAnimationAttackInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOULSLIKE_API ISLAnimationAttackInterface
{
	GENERATED_BODY()

public:
	virtual void AttackHitCheck() = 0;
};
