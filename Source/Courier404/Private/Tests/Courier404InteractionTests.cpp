#include "Courier404InteractionTests.h"
#include "Misc/AutomationTest.h"
#include "Interaction/InteractionComponent.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FCourier404InteractionSpec, "Courier404.Interaction", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

TObjectPtr<UInteractionComponent> Interaction;
AActor* DummyInteractor;

END_DEFINE_SPEC(FCourier404InteractionSpec)

void FCourier404InteractionSpec::Define()
{
	BeforeEach([this]()
	{
		Interaction = NewObject<UInteractionComponent>();
		DummyInteractor = NewObject<AActor>();
		TestNotNull(TEXT("component created"), Interaction.Get());
	});

	AfterEach([this]()
	{
		Interaction = nullptr;
		DummyInteractor = nullptr;
	});

	It(TEXT("is interactable when enabled"), [this]()
	{
		Interaction->bInteractionEnabled = true;
		TestTrue(TEXT("can interact"), Interaction->CanInteract(DummyInteractor));
	});

	It(TEXT("rejects interaction when disabled"), [this]()
	{
		Interaction->bInteractionEnabled = false;
		TestFalse(TEXT("cannot interact when disabled"), Interaction->CanInteract(DummyInteractor));

		UMockInteractListener* Listener = NewObject<UMockInteractListener>();
		FOnInteracted::FDelegate Delegate;
		Delegate.BindUFunction(Listener, GET_FUNCTION_NAME_CHECKED(UMockInteractListener, HandleInteracted));
		Interaction->OnInteracted.Add(Delegate);

		Interaction->Interact(DummyInteractor);
		TestEqual(TEXT("no broadcast when disabled"), Listener->Count, 0);
	});

	It(TEXT("returns empty prompt when disabled"), [this]()
	{
		Interaction->PromptText = FText::FromString(TEXT("Open"));
		Interaction->bInteractionEnabled = true;
		TestTrue(TEXT("prompt present when enabled"),
			!Interaction->GetInteractionPrompt(DummyInteractor).IsEmpty());
		Interaction->bInteractionEnabled = false;
		TestTrue(TEXT("prompt empty when disabled"),
			Interaction->GetInteractionPrompt(DummyInteractor).IsEmpty());
	});

	It(TEXT("broadcasts once per successful interaction"), [this]()
	{
		Interaction->bInteractionEnabled = true;

		UMockInteractListener* Listener = NewObject<UMockInteractListener>();
		FOnInteracted::FDelegate Delegate;
		Delegate.BindUFunction(Listener, GET_FUNCTION_NAME_CHECKED(UMockInteractListener, HandleInteracted));
		Interaction->OnInteracted.Add(Delegate);

		Interaction->Interact(DummyInteractor);
		Interaction->Interact(DummyInteractor);

		TestEqual(TEXT("two interactions fired twice"), Listener->Count, 2);
	});
}

#endif
