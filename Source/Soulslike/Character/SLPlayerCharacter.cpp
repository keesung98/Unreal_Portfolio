// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SLPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputMappingContext.h"
#include "GameFramework/Actor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimMontage.h"
#include "Animation/SLCharacterAnimInstance.h"
#include "SLComboActionData.h"

// Sets default values
ASLPlayerCharacter::ASLPlayerCharacter()
{
	// Pawn
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	// Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Mesh
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -100.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));

	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->SetRelativeLocation(FVector(0.0f, 100.0f, 50.0f));
	FollowCamera->bUsePawnControlRotation = false;

	// Mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Bladed.SK_CharM_Bladed'"));
	if (CharacterMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshRef.Object);
	}

	//Animation
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceClassRef(TEXT("/Game/Soulslike/Animation/ABP_SLCharacter.ABP_SLCharacter_C"));
	if (AnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(AnimInstanceClassRef.Class);
	}

	// Input
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> InputMappingContextRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Soulslike/Input/IMC_Default.IMC_Default'"));
	if (nullptr != InputMappingContextRef.Object)
	{
		DefaultMappingContext = InputMappingContextRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Soulslike/Input/Actions/IA_Move.IA_Move'"));
	if (nullptr != InputActionMoveRef.Object)
	{
		MoveAction = InputActionMoveRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionJumpRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Soulslike/Input/Actions/IA_Jump.IA_Jump'"));
	if (nullptr != InputActionJumpRef.Object)
	{
		JumpAction = InputActionJumpRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionSprintRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Soulslike/Input/Actions/IA_Sprint.IA_Sprint'"));
	if (nullptr != InputActionSprintRef.Object)
	{
		SprintAction = InputActionSprintRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionLookRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Soulslike/Input/Actions/IA_Look.IA_Look'"));
	if (nullptr != InputActionLookRef.Object)
	{
		LookAction = InputActionLookRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Soulslike/Input/Actions/IA_Attack.IA_Attack'"));
	if (nullptr != InputActionAttackRef.Object)
	{
		AttackAction = InputActionAttackRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> InputActionRollRef(TEXT("/Script/EnhancedInput.InputAction'/Game/Soulslike/Input/Actions/IA_Roll.IA_Roll'"));
	if (nullptr != InputActionRollRef.Object)
	{
		RollAction = InputActionRollRef.Object;
	}
}

void ASLPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = CastChecked<APlayerController>(GetController());
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
		//Subsystem->RemoveMappingContext(DefaultMappingContext);
	}
}

void ASLPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ASLPlayerCharacter::Sprint);
	EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASLPlayerCharacter::EndSprint);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASLPlayerCharacter::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASLPlayerCharacter::Look);
	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASLPlayerCharacter::Attack);
	EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &ASLPlayerCharacter::Roll);
}

void ASLPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void ASLPlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ASLPlayerCharacter::Sprint()
{
	if (!CheckIsJumping())
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
}

void ASLPlayerCharacter::EndSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}

void ASLPlayerCharacter::Attack()
{
	if (!CheckIsJumping())
	{
		ProcessComboCommand();
	}
}

bool ASLPlayerCharacter::CheckIsJumping()
{
	if (GetCharacterMovement()->IsFalling())return true;
	else return false;
}

void ASLPlayerCharacter::Roll()
{
	if (!CheckIsJumping())
	{
		if (GetCharacterMovement()->Velocity.Size() > 1.0f && DoOnce)
		{
			DoOnce = false;
			FVector MovementInputVector = GetLastMovementInputVector();
			FRotator RollingRotation = MovementInputVector.Rotation();
			SetActorRotation(RollingRotation);

			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{

				// Montage End Event Delegate
				FOnMontageEnded MontageEndedDelegate;
				MontageEndedDelegate.BindUObject(this, &ASLPlayerCharacter::OnMontageEnded);

				// Montage Play
				AnimInstance->Montage_Play(RollMontage);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate);
			}
		}
	}
}

void ASLPlayerCharacter::OnMontageEnded(class UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	DoOnce = true;
}

void ASLPlayerCharacter::ProcessComboCommand()
{
	if (CurrentCombo == 0)
	{
		ComboActionBegin();
		return;
	}

	if (!ComboTimerHandle.IsValid())
	{
		HasNextComboCommand = false;
	}
	else
	{
		HasNextComboCommand = true;
	}
}

void ASLPlayerCharacter::ComboActionBegin()
{
	// Combo Status
	CurrentCombo = 1;

	// Movement Setting
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	// Animation Setting
	const float AttackSpeedRate = 1.0f;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(ComboActionMontage, AttackSpeedRate);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ASLPlayerCharacter::ComboActionEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ComboActionMontage);

	ComboTimerHandle.Invalidate();
	SetComboCheckTimer();
}

void ASLPlayerCharacter::ComboActionEnd(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	ensure(CurrentCombo != 0);
	CurrentCombo = 0;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void ASLPlayerCharacter::SetComboCheckTimer()
{
	int32 ComboIndex = CurrentCombo - 1;
	ensure(ComboActionData->EffectiveFrameCount.IsValidIndex(ComboIndex));

	const float AttackSpeedRate = 1.0f;
	float ComboEffectiveTime = (ComboActionData->EffectiveFrameCount[ComboIndex] / ComboActionData->FrameRate) / AttackSpeedRate;
	if (ComboEffectiveTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(ComboTimerHandle, this, &ASLPlayerCharacter::ComboCheck, ComboEffectiveTime, false);
	}
}

void ASLPlayerCharacter::ComboCheck()
{
	ComboTimerHandle.Invalidate();
	if (HasNextComboCommand)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		CurrentCombo = FMath::Clamp(CurrentCombo + 1, 1, ComboActionData->MaxComboCount);
		FName NextSection = *FString::Printf(TEXT("%s%d"), *ComboActionData->MontageSectionNamePrefix, CurrentCombo);
		AnimInstance->Montage_JumpToSection(NextSection, ComboActionMontage);
		SetComboCheckTimer();
		HasNextComboCommand = false;
	}
}
