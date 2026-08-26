#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Courier404InteractionTests.generated.h"

UCLASS()
class UMockInteractListener : public UObject
{
	GENERATED_BODY()

public:
	int32 Count = 0;

	UFUNCTION()
	void HandleInteracted(AActor* Interactor) { ++Count; }
};
