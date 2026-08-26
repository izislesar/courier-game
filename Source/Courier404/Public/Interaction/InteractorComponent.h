#pragma once

#include "Components/ActorComponent.h"
#include "Interaction/Courier404Interaction.h"
#include "InteractorComponent.generated.h"

class UInteractionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFocusChanged, AActor*, FocusedActor, FText, Prompt);

/**
 * Player-side interaction query. Traces from the owning pawn's view point,
 * tracks the focused interactable and routes Interact requests.
 */
UCLASS(ClassGroup = (Courier404), meta = (BlueprintSpawnableComponent))
class COURIER404_API UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractorComponent();

	/** Maximum interaction reach from the camera. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Courier404|Interaction")
	float InteractionDistance = 250.f;

	UPROPERTY(BlueprintAssignable, Category = "Courier404|Interaction")
	FOnFocusChanged OnFocusChanged;

	/** Trace from view point and update focus state. Safe to call every tick or on demand. */
	void UpdateFocus();

	/** Interact with the currently focused interactable. Returns true when an interaction fired. */
	bool TryInteract();

	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void SetFocusedActor(AActor* NewFocus);

	UPROPERTY()
	TWeakObjectPtr<AActor> FocusedActor;
};
