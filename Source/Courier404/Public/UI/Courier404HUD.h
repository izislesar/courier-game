#pragma once

#include "GameFramework/HUD.h"
#include "Courier404HUD.generated.h"

class UCourier404PhoneComponent;
class UAudioComponent;

/**
 * Minimal slice HUD: draws the phone panel when open. Reads view model state;
 * never mutates domain truth directly.
 */
UCLASS()
class COURIER404_API ACourier404HUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	void SetInteractionPrompt(AActor* FocusedActor, FText Prompt);
	void BindToInteraction();

private:
	UCourier404PhoneComponent* ResolvePhone() const;
	void DrawPhonePanel();
	void DrawInteractionPrompt();

	FText CurrentPrompt;
	float PromptAlpha = 0.f;
	bool bBoundToInteraction = false;
};
