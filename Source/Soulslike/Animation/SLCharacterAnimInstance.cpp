// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/SLCharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "Character/SLPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

USLCharacterAnimInstance::USLCharacterAnimInstance()
{
	MovingThreshould = 3.0f;
	JumpingThreshould = 100.0f;
}

void USLCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Owner = Cast<ACharacter>(GetOwningActor());
	if (Owner)
	{
		Movement = Owner->GetCharacterMovement();
	}
}

void USLCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Speed & Direction
	APawn* Pawn = TryGetPawnOwner();
	if (!::IsValid(Pawn)) return;
	GroundSpeed = Pawn->GetVelocity().Size();
	Direction = CalculateDirection(Pawn->GetVelocity(), Pawn->GetActorRotation());
	Velocity = Movement->Velocity;

	// Bool
	if (Movement)
	{
		bIsIdle = GroundSpeed < MovingThreshould;
		bIsFalling = Movement->IsFalling();
		bIsJumping = bIsFalling & (Velocity.Z > JumpingThreshould);
	}
}
