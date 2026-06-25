// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BasePlayerCharacter.generated.h"

UCLASS()
class DONTDIE_API ABasePlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABasePlayerCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Mapping")
	class UInputMappingContext* ImcDefault;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Default")
	class UInputAction* IaMove;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Default")
	class UInputAction* IaLook;

protected:
	virtual void BeginPlay() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY()
	class APlayerController* PlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* CameraComp;

	void OnInputMovement(const struct FInputActionValue& Value);
	void OnInputLook(const struct FInputActionValue& Value);
};
