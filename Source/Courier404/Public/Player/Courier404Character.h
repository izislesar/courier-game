#pragma once

#include "GameFramework/Character.h"
#include "Courier404Character.generated.h"

class UInputAction;
class UInputMappingContext;
class UCameraComponent;
class UCourier404PhoneComponent;
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

	/** Audit accessors (regression tests; input must hard-reference cooked content). */
	const TObjectPtr<UInputMappingContext>& GetDefaultMappingContext() const { return DefaultMappingContext; }
	const TObjectPtr<UInputAction>& GetMoveAction() const { return MoveAction; }
	const TObjectPtr<UInputAction>& GetLookAction() const { return LookAction; }
	const TObjectPtr<UInputAction>& GetInteractAction() const { return InteractAction; }
	const TObjectPtr<UInputAction>& GetPhoneToggleAction() const { return PhoneToggleAction; }

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	void TogglePhone();
	void PhoneCycle();
	void PhoneAccept();
	void PhoneDecline();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Courier404")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Courier404")
	TObjectPtr<UInteractorComponent> Interactor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Courier404")
	TObjectPtr<UCourier404PhoneComponent> Phone;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> PhoneToggleAction;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> PhoneCycleAction;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> PhoneAcceptAction;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> PhoneDeclineAction;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Courier404|Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, Config, Category = "Courier404|Camera")
	float MouseSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, Config, Category = "Courier404|Camera")
	float FOV = 90.0f;
};
