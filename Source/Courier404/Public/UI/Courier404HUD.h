#pragma once

#include "GameFramework/HUD.h"
#include "Courier404HUD.generated.h"

class UCourier404PhoneComponent;

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

private:
	UCourier404PhoneComponent* ResolvePhone() const;
	void DrawPhonePanel();
};
