# Unreal_Portfolio

언리얼 프로젝트를 C++를 이용하여 다시 만들어보았습니다.

---

## 캐릭터 이동

![20240301_포트폴리오 이동](https://github.com/keesung98/Unreal_Portfolio/assets/70887592/4c97ed8a-1c73-4bd1-bea5-9cc4962690e0)

EnHancedInput을 사용하여 캐릭터의 움직임을 구현했습니다. 
이동과 점프, 달리기를 Bind시켜 작동하게 만들었습니다.

```cpp
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
```
---

## 공격과 구르기

![20240303_포트폴리오 공격모션](https://github.com/keesung98/Unreal_Portfolio/assets/70887592/e96e6378-67d4-4bb0-8e12-8478c1a15b39)
![20240305_포트폴리오 구르기](https://github.com/keesung98/Unreal_Portfolio/assets/70887592/40b2b598-38cf-4f19-8db3-09e527e6b763)

마찬가지로 EnHancedInput을 사용하여 구현했으며 구르기 시 

```cpp
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
```

DoOnce를 사용하여 한번만 작동하게 만들었으며 캐릭터가 바라보는 방향으로 애니메이션을 작동하며 이동하게 만들었습니다.

---

## 적 AI

![20240306_포트폴리오 적AI](https://github.com/keesung98/Unreal_Portfolio/assets/70887592/ae9afecf-6c74-4449-8e4b-e8e77409d703)
![20240310_포트폴리오 적AI공격](https://github.com/keesung98/Unreal_Portfolio/assets/70887592/bfa6c73d-7dcf-4cbc-b8c3-794748e9c33d)

적 AI는 비헤이비어 트리를 이용하여 구현하였습니다.
AI는 랜덤하게 받은 위치로 이동하며 근처에 플레이어가 있는지 탐색합니다. 
근처에 플레이어가 있다면 AI의 Target에 플레이어가 들어가며 플레이어에게 다가와 일정 거리가 만족하면 공격 모션을 실행하게 됩니다.

공격 거리 확인
```cpp
bool UBTDecorator_AttackInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (nullptr == ControllingPawn)
	{
		return false;
	}

	ISLBearAIInterface* AIPawn = Cast<ISLBearAIInterface>(ControllingPawn);
	if (nullptr == AIPawn)
	{
		return false;
	}

	APawn* Target = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BBKEY_TARGET));
	if (nullptr == Target)
	{
		return false;
	}

	float DistanceToTarget = ControllingPawn->GetDistanceTo(Target);
	float AttackRangeWithRadius = AIPawn->GetAIAttackRange();
	bResult = (DistanceToTarget <= AttackRangeWithRadius);
	UE_LOG(LogTemp, Log,TEXT("%f %f"), DistanceToTarget, AttackRangeWithRadius);
	return bResult;
}
```

---

## 적 AI 기능 추가

![20240312_포트폴리오 공격패턴 증가](https://github.com/keesung98/Unreal_Portfolio/assets/70887592/065592ee-203e-4e9a-b016-2b5828e366b1)
![20240312_포트폴리오 Death모션 추가](https://github.com/keesung98/Unreal_Portfolio/assets/70887592/4aecb52a-8b50-448f-9816-9712c5272d14)

더 다양한 공격모션과 죽는 모션을 추가하였습니다.
AI가 캐릭터에 다가와 공격할 시 정해진 3개의 모션 중에서 랜덤하게 선택하게 됩니다.
캐릭터에게 공격받아 Hp가 0이하로 내려갈 시 죽는 모션을 실행하며 이동과 충돌을 멈추고 5초가 지나면 사라지게 됩니다.

공격 선택
```cpp
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
```

죽음
```cpp
void ASLEnemyBearCharacter::SetDead()
{
	// ºñÇìÀÌºñ¾î Æ®¸® ¸ØÃß±â
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
```
---

## 캐릭터 죽음

![20240312_포트폴리오 캐릭터 죽음](https://github.com/keesung98/Unreal_Portfolio/assets/70887592/331b2708-fa99-4e8a-af39-3b33177ef783)

AI와 마찬가지로 캐릭터도 Hp가 0 이하로 내려갈 시 죽게 됩니다.
