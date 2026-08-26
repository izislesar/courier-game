#include "Contracts/ContractDomain.h"
#include "Contracts/Courier404Contracts.h"

#if WITH_DEV_AUTOMATION_TESTS

static FContractDefinition MakeNormalDefinition()
{
	FContractDefinition Def;
	Def.ContractId = TEXT("Job.NormalA");
	Def.Category = EContractCategory::Normal;
	Def.PickupPointId = TEXT("Point.Store");
	Def.DropoffPointId = TEXT("Point.Apartments");
	Def.Reward = 120;
	Def.TimeLimitSeconds = 600.f;
	Def.CargoId = TEXT("Cargo.Parcel");
	return Def;
}

static FCourier404ContractService MakeService()
{
	FCourier404ContractService Service;
	TArray<FString> Errors;
	verify(Service.RegisterDefinition(MakeNormalDefinition(), Errors));
	return Service;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404ContractAcceptPickupDeliver,
	"Courier404.Contracts.AcceptPickupDeliverPaysOnce",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404ContractAcceptPickupDeliver::RunTest(const FString& Parameters)
{
	FCourier404ContractService Service = MakeService();

	int32 PayoutSum = 0;
	Service.OnContractCompleted.AddLambda([&PayoutSum](const FContractRuntimeState&, int32 Payout)
	{
		PayoutSum += Payout;
	});

	const FName Instance = Service.Accept(TEXT("Job.NormalA"), 100.f);
	TestFalse(TEXT("instance id assigned"), Instance.IsNone());
	TestTrue(TEXT("status accepted"), Service.FindInstance(Instance)->Status == EContractStatus::Accepted);

	// Delivery before pickup must be rejected.
	int32 Payout = -1;
	TestFalse(TEXT("early delivery rejected"), Service.TryDeliver(Instance, TEXT("Point.Apartments"), 110.f, Payout));
	TestEqual(TEXT("no payout on rejection"), Payout, 0);

	TestTrue(TEXT("pickup marked"), Service.MarkPickup(Instance, 150.f));

	// Wrong drop-off must be rejected.
	FName WrongDrop;
	TestFalse(TEXT("wrong dropoff rejected"), Service.TryDeliver(Instance, TEXT("Point.SomewhereElse"), 200.f, Payout));

	// Correct drop completes once.
	TestTrue(TEXT("delivery accepted"), Service.TryDeliver(Instance, TEXT("Point.Apartments"), 200.f, Payout));
	TestEqual(TEXT("payout equals reward"), Payout, 120);
	TestEqual(TEXT("completed event fired exactly once"), PayoutSum, 120);

	// Duplicate delivery must be rejected without extra payout.
	int32 SecondPayout = 0;
	TestFalse(TEXT("duplicate delivery rejected"), Service.TryDeliver(Instance, TEXT("Point.Apartments"), 210.f, SecondPayout));
	TestEqual(TEXT("payout still once"), PayoutSum, 120);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404ContractFailState,
	"Courier404.Contracts.FailStateIsTerminal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404ContractFailState::RunTest(const FString& Parameters)
{
	FCourier404ContractService Service = MakeService();

	const FName Instance = Service.Accept(TEXT("Job.NormalA"), 10.f);

	TestTrue(TEXT("fail succeeds"), Service.Fail(Instance, TEXT("Test.Abandoned"), 20.f));
	TestTrue(TEXT("status failed"), Service.FindInstance(Instance)->Status == EContractStatus::Failed);

	// Terminal: further transitions are refused.
	TestFalse(TEXT("pickup after fail rejected"), Service.MarkPickup(Instance, 25.f));
	int32 Payout = 0;
	TestFalse(TEXT("delivery after fail rejected"), Service.TryDeliver(Instance, TEXT("Point.Apartments"), 30.f, Payout));
	TestFalse(TEXT("double fail rejected"), Service.Fail(Instance, TEXT("Test.Again"), 40.f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404ContractTimeLimit,
	"Courier404.Contracts.TimeLimitExpires",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404ContractTimeLimit::RunTest(const FString& Parameters)
{
	FCourier404ContractService Service = MakeService();

	FName Expired = NAME_None;
	Service.OnContractFailed.AddLambda([&Expired](const FContractRuntimeState& State)
	{
		Expired = State.InstanceId;
	});

	const FName Instance = Service.Accept(TEXT("Job.NormalA"), 0.f); // limit 600

	TestFalse(TEXT("not expired before limit"), Service.IsExpired(Instance, 599.f));
	TestTrue(TEXT("expired past limit"), Service.IsExpired(Instance, 601.f));
	TestTrue(TEXT("expire transitions to failed"), Service.ExpireIfPastLimit(Instance, 601.f));
	TestEqual(TEXT("fail event emitted for expiry"), Expired, Instance);

	TestFalse(TEXT("pickup after expiry rejected"), Service.MarkPickup(Instance, 610.f));
	int32 Payout = 0;
	TestFalse(TEXT("delivery after expiry rejected"), Service.TryDeliver(Instance, TEXT("Point.Apartments"), 620.f, Payout));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCourier404ContractValidationAndData,
	"Courier404.Contracts.ValidationAndDataDrivenRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCourier404ContractValidationAndData::RunTest(const FString& Parameters)
{
	FCourier404ContractService Service;

	// Invalid definitions are rejected at registration.
	FContractDefinition Invalid = MakeNormalDefinition();
	Invalid.ContractId = NAME_None;
	TArray<FString> Errors;
	TestFalse(TEXT("empty id rejected"), Service.RegisterDefinition(Invalid, Errors));
	TestTrue(TEXT("errors reported"), Errors.Num() > 0);

	Invalid = MakeNormalDefinition();
	Invalid.Reward = -5;
	Errors.Reset();
	TestFalse(TEXT("negative reward rejected"), Service.RegisterDefinition(Invalid, Errors));

	// Duplicate ids rejected.
	TArray<FString> OkErrors;
	TestTrue(TEXT("first registration ok"), Service.RegisterDefinition(MakeNormalDefinition(), OkErrors));
	TestFalse(TEXT("duplicate registration rejected"), Service.RegisterDefinition(MakeNormalDefinition(), OkErrors));

	// Anonymous contract with extensible rule survives into runtime.
	FContractDefinition Anon = MakeNormalDefinition();
	Anon.ContractId = TEXT("Job.AnonX");
	Anon.Category = EContractCategory::Anonymous;
	Anon.Reward = 900;
	Anon.RiskLevel = 3;
	Anon.Rules = { TEXT("Rule.Fragile") };
	Errors.Reset();
	TestTrue(TEXT("anonymous registration ok"), Service.RegisterDefinition(Anon, Errors));

	const FName Instance = Service.Accept(TEXT("Job.AnonX"), 500.f);
	TestFalse(TEXT("anonymous instance created"), Instance.IsNone());

	const FContractDefinition* Def = Service.FindDefinition(TEXT("Job.AnonX"));
	TestTrue(TEXT("category preserved"), Def && Def->Category == EContractCategory::Anonymous);
	TestTrue(TEXT("rule preserved"),
		Def && Def->Rules.Contains(FName(TEXT("Rule.Fragile"))));
	TestEqual(TEXT("risk preserved"), Def ? Def->RiskLevel : -1, 3);

	// Unknown contract cannot be accepted.
	TestTrue(TEXT("unknown accept returns none"), Service.Accept(TEXT("Job.Missing"), 1.f).IsNone());

	return true;
}

#endif
