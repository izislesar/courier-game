#include "Police/PoliceEncounter.h"
#include "Contracts/ContractDomain.h"

ECourier404PoliceOutcome FCourier404PoliceEncounter::DecideOutcome(const FCourier404EncounterInputs& Inputs)
{
	int32 Severity = 0;

	switch (Inputs.Reason)
	{
	case ECourier404StopReason::TrafficViolation:
		Severity += 0;
		break;
	case ECourier404StopReason::SuspiciousArea:
		Severity += 1;
		break;
	case ECourier404StopReason::CargoInspection:
		Severity += 1;
		break;
	}

	if (Inputs.CargoRiskLevel > 0)
	{
		++Severity;
	}
	if (Inputs.PriorOffenses >= 1)
	{
		++Severity;
	}
	if (Inputs.PriorOffenses >= 3)
	{
		++Severity; // repeat offender escalation
	}
	if (Inputs.bTriedToFlee)
	{
		++Severity; // flight escalates one step
	}

	if (Severity <= 0)
	{
		return ECourier404PoliceOutcome::Warning;
	}
	if (Inputs.CargoRiskLevel >= 3 && (Inputs.PriorOffenses >= 1 || Inputs.bTriedToFlee))
	{
		return ECourier404PoliceOutcome::Arrest;
	}
	return ECourier404PoliceOutcome::Fine;
}

FCourier404EncounterResult FCourier404PoliceEncounter::Execute(
	const FCourier404EncounterInputs& Inputs, const FAppliers& Appliers)
{
	FCourier404EncounterResult Result;
	Result.Outcome = DecideOutcome(Inputs);

	switch (Result.Outcome)
	{
	case ECourier404PoliceOutcome::Warning:
		break;

	case ECourier404PoliceOutcome::Fine:
	{
		const int32 Base = Inputs.CargoRiskLevel > 0 ? 150 * Inputs.CargoRiskLevel : 100;
		Result.FineAmount = Appliers.ChargeWallet ? Appliers.ChargeWallet(Base) : Base;
		break;
	}

	case ECourier404PoliceOutcome::Arrest:
	{
		// Confiscation-style fine: half of current balance via clamped charge of 300.
		constexpr int32 ArrestFine = 300;
		Result.FineAmount = Appliers.ChargeWallet ? Appliers.ChargeWallet(ArrestFine) : ArrestFine;

		Result.DetentionHours = 8.f;
		if (Appliers.AdvanceClock)
		{
			Appliers.AdvanceClock(Result.DetentionHours);
		}

		const bool bRiskyWork = Appliers.HasActiveRiskyContract ? Appliers.HasActiveRiskyContract() : false;
		if (bRiskyWork && Appliers.FailActiveContracts)
		{
			Appliers.FailActiveContracts(TEXT("Police.Arrest"));
			Result.bFailedActiveContract = true;
		}
		break;
	}
	}

	return Result;
}