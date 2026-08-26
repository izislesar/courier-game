#include "Player/Courier404Character.h"
#include "Interaction/InteractorComponent.h"
#include "UI/Courier404PhoneComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

ACourier404Character::ACourier404Character()
{
	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 320.f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(-10.f, 0.f, 70.f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	Interactor = CreateDefaultSubobject<UInteractorComponent>(TEXT("Interactor"));

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveFinder(TEXT("/Game/Input/IA_CourierMove.IA_CourierMove"));
	static ConstructorHelpers::FObjectFinder<UInputAction> LookFinder(TEXT("/Game/Input/IA_CourierLook.IA_CourierLook"));
	static ConstructorHelpers::FObjectFinder<UInputAction> InteractFinder(TEXT("/Game/Input/IA_CourierInteract.IA_CourierInteract"));
	static ConstructorHelpers::FObjectFinder<UInputAction> PhoneToggleFinder(TEXT("/Game/Input/IA_PhoneToggle.IA_PhoneToggle"));
	static ConstructorHelpers::FObjectFinder<UInputAction> PhoneCycleFinder(TEXT("/Game/Input/IA_PhoneCycle.IA_PhoneCycle"));
	static ConstructorHelpers::FObjectFinder<UInputAction> PhoneAcceptFinder(TEXT("/Game/Input/IA_PhoneAccept.IA_PhoneAccept"));
	static ConstructorHelpers::FObjectFinder<UInputAction> PhoneDeclineFinder(TEXT("/Game/Input/IA_PhoneDecline.IA_PhoneDecline"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCFinder(TEXT("/Game/Input/IMC_CourierDefault.IMC_CourierDefault"));

	if (MoveFinder.Succeeded()) { MoveAction = MoveFinder.Object; }
	if (LookFinder.Succeeded()) { LookAction = LookFinder.Object; }
	if (InteractFinder.Succeeded()) { InteractAction = InteractFinder.Object; }
	if (PhoneToggleFinder.Succeeded()) { PhoneToggleAction = PhoneToggleFinder.Object; }
	if (PhoneCycleFinder.Succeeded()) { PhoneCycleAction = PhoneCycleFinder.Object; }
	if (PhoneAcceptFinder.Succeeded()) { PhoneAcceptAction = PhoneAcceptFinder.Object; }
	if (PhoneDeclineFinder.Succeeded()) { PhoneDeclineAction = PhoneDeclineFinder.Object; }
	if (IMCFinder.Succeeded()) { DefaultMappingContext = IMCFinder.Object; }

	Phone = CreateDefaultSubobject<UCourier404PhoneComponent>(TEXT("Phone"));


}

void ACourier404Character::BeginPlay()
{
	Super::BeginPlay();
}

void ACourier404Character::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ACourier404Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction) { EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACourier404Character::Move); }
		if (LookAction) { EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACourier404Character::Look); }
		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ACourier404Character::Interact);
		}
		if (PhoneToggleAction)
		{
			EIC->BindAction(PhoneToggleAction, ETriggerEvent::Started, this, &ACourier404Character::TogglePhone);
		}
		if (PhoneCycleAction)
		{
			EIC->BindAction(PhoneCycleAction, ETriggerEvent::Started, this, &ACourier404Character::PhoneCycle);
		}
		if (PhoneAcceptAction)
		{
			EIC->BindAction(PhoneAcceptAction, ETriggerEvent::Started, this, &ACourier404Character::PhoneAccept);
		}
		if (PhoneDeclineAction)
		{
			EIC->BindAction(PhoneDeclineAction, ETriggerEvent::Started, this, &ACourier404Character::PhoneDecline);
		}
	}
}

void ACourier404Character::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator YawOnly(0.f, Controller->GetControlRotation().Yaw, 0.f);
		AddMovementInput(YawOnly.RotateVector(FVector::ForwardVector), Axis.Y);
		AddMovementInput(YawOnly.RotateVector(FVector::RightVector), Axis.X);
	}
}

void ACourier404Character::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X * 0.6f);
	AddControllerPitchInput(Axis.Y * 0.6f);
}

void ACourier404Character::Interact()
{
	Interactor->TryInteract();
}

void ACourier404Character::TogglePhone()
{
	Phone->ToggleOpen();
}

void ACourier404Character::PhoneCycle()
{
	if (Phone->IsOpen())
	{
		Phone->GetViewModel().CycleScreen(1);
	}
}

void ACourier404Character::PhoneAccept()
{
	if (Phone->IsOpen())
	{
		Phone->GetViewModel().AcceptSelected();
	}
}

void ACourier404Character::PhoneDecline()
{
	if (Phone->IsOpen())
	{
		Phone->GetViewModel().RejectSelected();
	}
}
