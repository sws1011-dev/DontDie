// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "player/BasePlayerCharacter.h"
#include "CampPlayerCharacter.generated.h"

UCLASS()
class DONTDIE_API ACampPlayerCharacter : public ABasePlayerCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACampPlayerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Build")
	class UCampBuildComponent* CampBuildComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Mapping")
	class UInputMappingContext* ImcBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Default")
	class UInputAction* IaToggleBuildMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaConfirmBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaCancelBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaRotateBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaChangeBuild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Build")
	class UInputAction* IaUpgradeBuild;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void ToggleBuildMode();
	void SetBuildMode(bool bEnabled);
	void ConfirmBuild();
	void CancelBuild();
	void RotateBuild(const struct FInputActionValue& Value);
	void ChangeBuild(const struct FInputActionValue& Value);
	void UpgradeBuild(const struct FInputActionValue& Value);
	void AddBuildMappingContext();
	void RemoveBuildMappingContext();
};
