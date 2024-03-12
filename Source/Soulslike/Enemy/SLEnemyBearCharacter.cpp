#include "Enemy/SLEnemyBearCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/Bear/SLBearAIController.h"
#include "Engine/DamageEvents.h"
#include "Physics/SLCollision.h"
#include "AI/Bear/SLBearAIController.h"

// Sets default values
ASLEnemyBearCharacter::ASLEnemyBearCharacter()
{
	//Controller
	AIControllerClass = ASLBearAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Capsule
	GetCapsuleComponent()->InitCapsuleSize(90.f, 80.0f);
	GetCapsuleComponent()->SetCollisionProfileName(CPROFILE_ABCAPSULE);

	// Movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Mesh
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -90.0f), FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));

	// Mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/InfinityBladeAdversaries/Enemy/Enemy_Bear/Enemy_Bear.Enemy_Bear'"));
	if (CharacterMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(CharacterMeshRef.Object);
	}

	//Animation
	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimInstanceClassRef(TEXT("/Game/Soulslike/Enemy/Bear/ABP_EnemyBear.ABP_EnemyBear_C"));
	if (AnimInstanceClassRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(AnimInstanceClassRef.Class);
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> Action1MontageRef(TEXT("/Script/Engine.AnimMontage'/Game/Soulslike/Enemy/Bear/AM_Attack1.AM_Attack1'"));
	if (Action1MontageRef.Object)
	{
		Action1Montage = Action1MontageRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> Action2MontageRef(TEXT("/Script/Engine.AnimMontage'/Game/Soulslike/Enemy/Bear/AM_Attack2.AM_Attack2'"));
	if (Action2MontageRef.Object)
	{
		Action2Montage = Action2MontageRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> Action3MontageRef(TEXT("/Script/Engine.AnimMontage'/Game/Soulslike/Enemy/Bear/AM_Attack3.AM_Attack3'"));
	if (Action3MontageRef.Object)
	{
		Action3Montage = Action3MontageRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DeadMontageRef(TEXT("/Script/Engine.AnimMontage'/Game/Soulslike/Enemy/Bear/AM_Dead.AM_Dead'"));
	if (DeadMontageRef.Object)
	{
		DeadMontage = DeadMontageRef.Object;
	}
}

float ASLEnemyBearCharacter::GetAIPatrolRadius()
{
	return 800.0f;
}

float ASLEnemyBearCharacter::GetAIDetectRange()
{
	return 400.0f;
}

float ASLEnemyBearCharacter::GetAIAttackRange()
{
	return 200.0f;
}

float ASLEnemyBearCharacter::GetAITurnSpeed()
{
	return 2.0f;
}

void ASLEnemyBearCharacter::SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAttackFinished = InOnAttackFinished;
}

void ASLEnemyBearCharacter::AttackByAI()
{
	UE_LOG(LogTemp, Log, TEXT("Attack Start"));
	// Movement Setting
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	//Random Attack Setting
	int AttackNum = FMath::RandRange(1, 3);
	switch (AttackNum)
	{
	case 1:
		ActionMontage = Action1Montage;
		break;
	case 2:
		ActionMontage = Action2Montage;
		break; 
	case 3:
		ActionMontage = Action3Montage;
		break;
	}

	// Animation Setting
	const float AttackSpeedRate = 1.0f;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(ActionMontage, AttackSpeedRate);

	//Set MontageEnd Delegate
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ASLEnemyBearCharacter::ComboActionEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ActionMontage);
	UE_LOG(LogTemp, Log, TEXT("Attack Complete"));
}

void ASLEnemyBearCharacter::NotifyActionEnd()
{
	OnAttackFinished.ExecuteIfBound();
}

void ASLEnemyBearCharacter::ComboActionEnd(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	UE_LOG(LogTemp, Log, TEXT("Attack End"));
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

	NotifyActionEnd();
}

void ASLEnemyBearCharacter::AttackHitCheck()
{
	FHitResult OutHitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

	const float AttackRange = 150.0f;
	const float AttackRadius = 70.0f;
	const float AttackDamage = 30.0f;
	const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector End = Start + GetActorForwardVector() * AttackRange;

	bool HitDetected = GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, CCHANNEL_ABACTION, FCollisionShape::MakeSphere(AttackRadius), Params);
	if (HitDetected)
	{
		FDamageEvent DamageEvent;
		OutHitResult.GetActor()->TakeDamage(AttackDamage, DamageEvent, GetController(), this);
	}

#if ENABLE_DRAW_DEBUG

	FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;
	FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;

	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 5.0f);

#endif
}

float ASLEnemyBearCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	SetDead();

	return DamageAmount;
}


void ASLEnemyBearCharacter::SetDead()
{
	// 비헤이비어 트리 멈추기
	ASLBearAIController* TreeController = Cast<ASLBearAIController>(GetController());
	if (TreeController)
	{
		TreeController->StopAI();
	}

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	PlayDeadAnimation();
	SetActorEnableCollision(false);

	FTimerHandle DeadTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DeadTimerHandle, FTimerDelegate::CreateLambda(
		[&]()
		{
			Destroy();
		}
	), DeadEventDelayTime, false);
}

void ASLEnemyBearCharacter::PlayDeadAnimation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->StopAllMontages(0.0f);
	AnimInstance->Montage_Play(DeadMontage, 1.0f);
}
