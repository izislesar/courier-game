#include "Core/Courier404GameMode.h"
#include "Courier404.h"
#include "Player/Courier404Character.h"

ACourier404GameMode::ACourier404GameMode()
{
	DefaultPawnClass = ACourier404Character::StaticClass();
	UE_LOG(LogCourier404, Log, TEXT("Courier404GameMode created"));
}
