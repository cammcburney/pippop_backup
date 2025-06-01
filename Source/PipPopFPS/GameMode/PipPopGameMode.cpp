#include "PipPopGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h" 
#include "../PlayerCharacter/Controller/FPSPlayerController.h"
#include "../PlayerCharacter/State/FPSPlayerState.h"
#include "GameState/PipPopGameState.h"
#include "../PlayerCharacter/Classes/FirstPersonCharacter.h"

APipPopGameMode::APipPopGameMode()
{
    DefaultPawnClass = AFirstPersonCharacter::StaticClass();
    PlayerControllerClass = AFPSPlayerController::StaticClass();
}

void APipPopGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (NewPlayer) ConnectedPlayers.Add(NewPlayer);
}

void APipPopGameMode::Logout(AController* Exiting)
{   
    if (Exiting) ConnectedPlayers.Remove(Cast<AFPSPlayerController>(Exiting));
    Super::Logout(Exiting);
}

void APipPopGameMode::SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC)
{
    if (NewPC) ConnectedPlayers.Add(NewPC);
}

void APipPopGameMode::SpawnPlayer_Implementation(AFPSPlayerController* PC, FName NameTag)
{
    AFPSPlayerController* FPC = Cast<AFPSPlayerController>(PC);
    if (APawn* ValidPawn = FPC->GetPawn()) ValidPawn->Destroy();
    UClass* PlayerClass = FPC->PlayerClass;
    if (PlayerClass)
    {
        FTransform PlayerStart = FindRandomPlayerStart(NameTag);
        APawn* PlayerPawn = GetWorld()->SpawnActor<APawn>(
            PlayerClass, 
            PlayerStart.GetLocation(), 
            PlayerStart.GetRotation().Rotator()
        );
        if (PlayerPawn) FPC->Possess(PlayerPawn);
    }
}

FTransform APipPopGameMode::FindRandomPlayerStart(FName Tag)
{
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
    for (AActor* Player:PlayerStarts) if (!Player->ActorHasTag(Tag)) PlayerStarts.Remove(Player);
    if (PlayerStarts.Num() > 0) 
    {
        return PlayerStarts[FMath::RandRange(0, PlayerStarts.Num() - 1)]->GetActorTransform();
    }
    return FTransform::Identity;
}

void APipPopGameMode::EndGame()
{
    TArray<AActor*> Players;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFPSPlayerController::StaticClass(), Players);
    for (AActor* Player:Players)
    {
        if (AFPSPlayerController* PC = Cast<AFPSPlayerController>(Player))
        {
            PC->UnPossess();
            // PC->PlayerCharacter = SpectatorPawn;
            AFPSPlayerState* PS = PC->GetPlayerState<AFPSPlayerState>();
            int32 WinnerScore = PS->PlayerScore;
            FString PlayerName = PC->GetName();
            GetWorld()->GetTimerManager().SetTimer(MapVoteHandle, this, &APipPopGameMode::LoadMap, MapSelectTimerLength * 60, false);
        }
    }
}

void APipPopGameMode::LoadMap()
{
    MapVoteHandle.Invalidate();
    APipPopGameState* GS = GetWorld()->GetGameState<APipPopGameState>();
    uint8 NextMap = GS->SelectNextMap();
    GetWorld()->ServerTravel(MapList[NextMap]);
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
