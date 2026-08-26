#pragma once

#include "GameFramework/Character.h"
#include "Courier404Character.generated.h"

class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class UInteractorComponent;
struct FInputActionValue;

/**
 * First-person player character. Routes input to movement and the
 * InteractorComponent; owns no domain logic.
 */
UCLASS(Config = Game)
class COURIER404_API ACourier404Character : public ACharacter
{
	GENERATED_BODY()

public:
	ACourier404Character();

	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Courier404")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Courier404")
	TObjectPtr<UInteractorComponent> Interactor;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TSoftObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> InteractAction;
};
