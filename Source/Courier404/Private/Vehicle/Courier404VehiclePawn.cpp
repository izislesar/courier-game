#include "Vehicle/Courier404VehiclePawn.h"
#include "Vehicle/VehicleFacade.h"
#include "Courier404.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

ACourier404VehiclePawn::ACourier404VehiclePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	BodyCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BodyCollision"));
	SetRootComponent(BodyCollision);
	BodyCollision->InitBoxExtent(FVector(110.f, 45.f, 30.f));
	BodyCollision->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	Facade = CreateDefaultSubobject<UVehicleFacadeComponent>(TEXT("Facade"));

	TrunkSocket = CreateDefaultSubobject<USceneComponent>(TEXT("TrunkSocket"));
	TrunkSocket->SetupAttachment(BodyCollision);
	TrunkSocket->SetRelativeLocation(FVector(-90.f, 0.f, 20.f));

	static ConstructorHelpers::FObjectFinder<UInputAction> DriveFinder(TEXT("/Game/Input/IA_CourierDrive.IA_CourierDrive"));
	static ConstructorHelpers::FObjectFinder<UInputAction> BrakeFinder(TEXT("/Game/Input/IA_CourierBrake.IA_CourierBrake"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DriveIMCFinder(TEXT("/Game/Input/IMC_CourierDrive.IMC_CourierDrive"));
	if (DriveFinder.Succeeded())
	{
		DriveAction = DriveFinder.Object;
	}
	if (BrakeFinder.Succeeded())
	{
		BrakeAction = BrakeFinder.Object;
	}
	if (DriveIMCFinder.Succeeded())
	{
		DriveMappingContext = DriveIMCFinder.Object;
	}

	bUseControllerRotationYaw = true;
}

bool ACourier404VehiclePawn::CanInteract(AActor* Interactor) const
{
	return IsValid(Interactor) && !IsOccupied();
}

FText ACourier404VehiclePawn::GetInteractionPrompt(AActor* Interactor) const
{
	return CanInteract(Interactor) ? FText::FromString(TEXT("Enter car")) : FText::GetEmpty();
}

void ACourier404VehiclePawn::Interact(AActor* Interactor)
{
	if (APawn* Pawn = Cast<APawn>(Interactor))
	{
		TryEnterVehicle(Pawn);
	}
}

bool ACourier404VehiclePawn::TryEnterVehicle(APawn* DriverToBe)
{
	if (!Facade || !Facade->CanEnter(DriverToBe))
	{
		return false;
	}
	APlayerController* PC = Cast<APlayerController>(DriverToBe->GetController());
	if (!PC)
	{
		return false;
	}

	DriverPawn = DriverToBe;
	if (!Facade->Enter(DriverToBe))
	{
		DriverPawn = nullptr;
		return false;
	}

	Facade->SetHeadlights(true);

	// Stash the driver inside the vehicle and hand over possession.
	DriverToBe->SetActorEnableCollision(false);
	DriverToBe->AttachToComponent(BodyCollision,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	DriverToBe->SetOwner(this);
	PC->Possess(this);
	return true;
}

bool ACourier404VehiclePawn::TryExitVehicle()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	APawn* Previous = DriverPawn.Get();
	if (!PC || !Previous)
	{
		return false;
	}

	const FVector ExitLocation = GetActorLocation() + GetActorRightVector() * 150.f;
	const FRotator KeepYaw(0.f, GetActorRotation().Yaw, 0.f);

	Previous->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Previous->SetActorLocationAndRotation(ExitLocation, KeepYaw);
	Previous->SetActorEnableCollision(true);
	Facade->SetHeadlights(false);

	DriverPawn = nullptr;
	Facade->Exit(Previous);
	PC->Possess(Previous);
	return true;
}

bool ACourier404VehiclePawn::IsOccupied() const
{
	return DriverPawn.IsValid() || (Facade && IsValid(Facade->GetDriver()));
}

float ACourier404VehiclePawn::GetSpeedKmh() const
{
	return FMath::Abs(ForwardSpeed) * 0.036f;
}

void ACourier404VehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsOccupied())
	{
		ForwardSpeed = 0.f;
		return;
	}

	// Longitudinal model.
	if (bHandbrake)
	{
		const float Decel = BrakeDeceleration * DeltaSeconds;
		ForwardSpeed = FMath::Sign(ForwardSpeed) * FMath::Max(0.f, FMath::Abs(ForwardSpeed) - Decel);
	}
	else if (!FMath::IsNearlyZero(ThrottleInput))
	{
		const float TargetMax = ThrottleInput > 0.f ? MaxSpeedKmh / 3.6f : -MaxReverseSpeedKmh / 3.6f;
		const float Step = Acceleration * DeltaSeconds * ThrottleInput;
		ForwardSpeed = FMath::Clamp(ForwardSpeed + Step,
			FMath::Min(TargetMax, ForwardSpeed), FMath::Max(TargetMax, ForwardSpeed));
	}
	else
	{
		// Engine braking.
		const float Decel = 120.f * DeltaSeconds;
		ForwardSpeed = FMath::Sign(ForwardSpeed) * FMath::Max(0.f, FMath::Abs(ForwardSpeed) - Decel);
	}

	// Steering scales down with speed for readability.
	const float SpeedRatio = FMath::Clamp(FMath::Abs(ForwardSpeed) / (MaxSpeedKmh / 3.6f), 0.f, 1.f);
	const float YawDelta = SteerInput * SteerRateDegPerSec * DeltaSeconds *
		FMath::Sign(ForwardSpeed == 0.f ? 1.f : ForwardSpeed) *
		(1.f - 0.55f * SpeedRatio);
	AddActorWorldRotation(FRotator(0.f, YawDelta, 0.f));

	const FVector Move = GetActorForwardVector() * ForwardSpeed * DeltaSeconds;
	AddActorWorldOffset(Move, true); // sweep: basic collision consequence

	UpdateFuel(DeltaSeconds, FMath::Abs(ThrottleInput));
}

void ACourier404VehiclePawn::UpdateFuel(float DeltaSeconds, float ThrottleMagnitude)
{
	const float Burn = (0.02f + 0.25f * ThrottleMagnitude) * DeltaSeconds;
	if (Facade && Facade->ConsumeFuel(Burn) < Burn - KINDA_SMALL_NUMBER && Facade->GetFuel() <= 0.f)
	{
		ThrottleInput = 0.f; // out of fuel
	}
}

void ACourier404VehiclePawn::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	AddMappingContext();
}

void ACourier404VehiclePawn::AddMappingContext()
{
	if (const APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DriveMappingContext)
			{
				Subsystem->AddMappingContext(DriveMappingContext, 1);
			}
		}
	}
}

void ACourier404VehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (DriveAction)
		{
			EIC->BindAction(DriveAction, ETriggerEvent::Triggered, this, &ACourier404VehiclePawn::ApplyDrive);
			EIC->BindAction(DriveAction, ETriggerEvent::Completed, this, &ACourier404VehiclePawn::ApplyDrive);
		}
		if (BrakeAction)
		{
			EIC->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACourier404VehiclePawn::ApplyBrake);
			EIC->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACourier404VehiclePawn::ReleaseBrake);
		}
		if (InteractExitAction)
		{
			EIC->BindAction(InteractExitAction, ETriggerEvent::Started, this, &ACourier404VehiclePawn::RequestExit);
		}
	}
}

void ACourier404VehiclePawn::ApplyDrive(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	SteerInput = Axis.X;
	ThrottleInput = Axis.Y;
}

void ACourier404VehiclePawn::ApplyBrake()
{
	bHandbrake = true;
}

void ACourier404VehiclePawn::ReleaseBrake()
{
	bHandbrake = false;
}

void ACourier404VehiclePawn::RequestExit()
{
	TryExitVehicle();
}

void ACourier404VehiclePawn::NotifyHit(UPrimitiveComponent*, AActor*, UPrimitiveComponent*, bool, FVector, FVector, FVector, const FHitResult&)
{
	if (Facade)
	{
		Facade->NotifyImpact(FMath::Abs(ForwardSpeed), nullptr);
	}
}
