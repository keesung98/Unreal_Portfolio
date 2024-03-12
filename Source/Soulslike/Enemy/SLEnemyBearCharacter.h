// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/SLBearAIInterface.h"
#include "Interface/SLAnimationAttackInterface.h"
#include "SLEnemyBearCharacter.generated.h"

UCLASS()
class SOULSLIKE_API ASLEnemyBearCharacter : public ACharacter, public ISLBearAIInterface, public ISLAnimationAttackInterface
{
	GENERATED_BODY()

public:
	ASLEnemyBearCharacter();

	// AI Section
protected:
	virtual float GetAIPatrolRadius() override;
	virtual float GetAIDetectRange() override;
	virtual float GetAIAttackRange() override;
	virtual float GetAITurnSpeed() override;

	virtual void SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished) override;
	virtual void AttackByAI() override;

	FAICharacterAttackFinished OnAttackFinished;

	virtual void NotifyActionEnd();

	//Attack Section
protected:
	UPROPERTY(BlueprintReadOnly,Category = Animation)
	TObjectPtr<class UAnimMontage> ActionMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> Action1Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> Action2Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> Action3Montage;

	void ComboActionEnd(class UAnimMontage* TargetMontage, bool IsProperlyEnded);

	// Attack Hit Section
protected:
	virtual void AttackHitCheck() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// Dead Section
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Stat, Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAnimMontage> DeadMontage;

	virtual void SetDead();
	void PlayDeadAnimation();

	float DeadEventDelayTime = 5.0f;
};
