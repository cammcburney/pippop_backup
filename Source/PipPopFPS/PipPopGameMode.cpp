#include "PipPopGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "PipPopFPS/FirstPersonCharacter.h"


APipPopGameMode::APipPopGameMode()
{
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/DefaultPawn"));
    if (PlayerPawnBPClass.Class != nullptr)
    {
        DefaultPawnClass = AFirstPersonCharacter::StaticClass();
    }
}

void APipPopGameMode::HostLANGame()
{
    GetWorld()->ServerTravel("Game/TestLevel?listen");
}

void APipPopGameMode::JoinLANGame()
{
    APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (PC)
    {
        PC->ClientTravel("192.168.0.39/Game/TestLevel?listen", TRAVEL_Absolute);
    }
}
