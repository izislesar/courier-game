#include "UI/Courier404HUD.h"
#include "UI/Courier404PhoneComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

UCourier404PhoneComponent* ACourier404HUD::ResolvePhone() const
{
	const APawn* Pawn = PlayerOwner ? PlayerOwner->GetPawn() : nullptr;
	return Pawn ? Pawn->FindComponentByClass<UCourier404PhoneComponent>() : nullptr;
}

void ACourier404HUD::DrawHUD()
{
	Super::DrawHUD();

	if (const UCourier404PhoneComponent* Phone = ResolvePhone())
	{
		if (Phone->IsOpen())
		{
			DrawPhonePanel();
		}
	}
}

void ACourier404HUD::DrawPhonePanel()
{
	if (!Canvas)
	{
		return;
	}

	const UCourier404PhoneComponent* Phone = ResolvePhone();
	if (!Phone)
	{
		return;
	}
	const FCourier404PhoneViewModel& VM = Phone->GetViewModel();

	const float PanelW = 420.f;
	const float PanelH = 320.f;
	const float OriginX = Canvas->SizeX - PanelW - 24.f;
	const float OriginY = 24.f;

	const FLinearColor Back(0.05f, 0.05f, 0.06f, 0.85f);
	FCanvasTileItem Tile(FVector2D(OriginX, OriginY), FVector2D(PanelW, PanelH), Back);
	Tile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Tile);

	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	float Y = OriginY + 12.f;

	const auto Line = [&](const FString& Text, const FLinearColor& Color = FLinearColor::White)
	{
		FCanvasTextItem TextItem(FVector2D(OriginX + 14.f, Y), FText::FromString(Text), Font, Color);
		TextItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(TextItem);
		Y += 20.f;
	};

	switch (VM.GetScreen())
	{
	case FCourier404PhoneViewModel::EScreen::Jobs:
	{
		Line(TEXT("JOBS  [E] accept  [R] decline"));
		if (VM.GetOffers().IsEmpty())
		{
			Line(TEXT("No offers right now."));
		}
		for (int32 i = 0; i < VM.GetOffers().Num(); ++i)
		{
			const FCourier404PhoneViewModel::FOffer& O = VM.GetOffers()[i];
			const bool bSel = i == VM.GetSelectedIndex();
			Line(FString::Printf(TEXT("%s%s  %d cr%s"),
				bSel ? TEXT("> ") : TEXT("  "),
				*O.ContractId.ToString(), O.Reward,
				O.Category == EContractCategory::Anonymous ? TEXT("  [ANON]") : TEXT("")),
				bSel ? FLinearColor::Yellow : FLinearColor::White);
		}
		break;
	}
	case FCourier404PhoneViewModel::EScreen::ActiveJob:
		Line(TEXT("ACTIVE JOB"));
		Line(VM.GetObjectiveLine());
		break;
	case FCourier404PhoneViewModel::EScreen::Messages:
		Line(TEXT("MESSAGES"));
		for (const FString& M : VM.GetMessages())
		{
			Line(M);
		}
		break;
	}
}
