#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Interaction/Courier404Interaction.h"
#include "Interaction/InteractionComponent.h"
#include "Courier404FocusTests.generated.h"

/** Minimal view pawn providing the camera the interactor traces from. */
UCLASS()
class AMockViewPawn : public APawn
{
	GENERATED_BODY()

public:
	AMockViewPawn()
	{
		USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
		SetRootComponent(Root);
		Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("TestCamera"));
		Camera->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
		Camera->SetupAttachment(Root);
	}

	UPROPERTY()
	TObjectPtr<UCameraComponent> Camera;
};

/** Interactable type 1: collision box + UInteractionComponent. */
UCLASS()
class AMockComponentInteractable : public AActor
{
	GENERATED_BODY()

public:
	AMockComponentInteractable()
	{
		Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
		SetRootComponent(Root);

		Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
		Box->SetupAttachment(Root);
		Box->SetBoxExtent(FVector(50.f));
		Box->SetCollisionProfileName(TEXT("BlockAllDynamic"));

		Interaction = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction"));
	}

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<UBoxComponent> Box;

	UPROPERTY()
	TObjectPtr<UInteractionComponent> Interaction;
};

/** Interactable type 2: native ICourier404Interactable implementation with its own box. */
UCLASS()
class AMockNativeInteractable : public AActor, public ICourier404Interactable
{
	GENERATED_BODY()

public:
	AMockNativeInteractable()
	{
		Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
		SetRootComponent(Root);

		Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
		Box->SetupAttachment(Root);
		Box->SetBoxExtent(FVector(50.f));
		Box->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	}

	virtual bool CanInteract(AActor* Interactor) const override { return bEnabled; }
	virtual FText GetInteractionPrompt(AActor* Interactor) const override
	{
		return FText::FromString(TEXT("Native"));
	}
	virtual void Interact(AActor* Interactor) override { ++InteractCount; }

	UPROPERTY()
	TObjectPtr<USceneComponent> Root;

	UPROPERTY()
	TObjectPtr<UBoxComponent> Box;

	int32 InteractCount = 0;
	bool bEnabled = true;
};
