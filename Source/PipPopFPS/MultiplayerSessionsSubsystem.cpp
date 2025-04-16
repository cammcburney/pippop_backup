#include "MultiplayerSessionsSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystem.h"

void PrintString(const FString& Str)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Cyan, Str);
    }
}

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
{
    CreateServerAfterDestroy = false;
    DestroyServerName = "";
    ServerNameSearched = "";
    MySessionName = FName("TestServer"); // Default session name
}

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (OnlineSubsystem)
    {
        FString SubsystemName = OnlineSubsystem->GetSubsystemName().ToString();
        SessionInterface = OnlineSubsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete);
            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnDestroySessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnFindSessionComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete);
        }
    }
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Deinitialize"));
}

void UMultiplayerSessionsSubsystem::CreateServer(FString ServerName)
{   
    if (!ServerName.IsEmpty())
    {   
        FOnlineSessionSettings SessionSettings;

        // Set MySessionName to the server name passed in
        MySessionName = FName(*ServerName);

        FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(MySessionName);
        if (ExistingSession)
        {
            CreateServerAfterDestroy = true;
            DestroyServerName = ServerName;
            SessionInterface->DestroySession(MySessionName);
            return;
        }

        SessionSettings.bAllowJoinInProgress = true;
        SessionSettings.bIsDedicated = false;
        SessionSettings.bShouldAdvertise = true;
        SessionSettings.NumPublicConnections = 10;
        SessionSettings.bUseLobbiesIfAvailable = true;
        SessionSettings.bUsesPresence = true;
        SessionSettings.bAllowJoinViaPresence = true;

        bool IsLAN = false;
        if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
        {
            IsLAN = true;
        }
        SessionSettings.bIsLANMatch = IsLAN;

        SessionSettings.Set(FName("SERVER_NAME"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

        // Use MySessionName here
        SessionInterface->CreateSession(0, MySessionName, SessionSettings);
    }
    else
    {   
        ServerCreateDel.Broadcast(false);
        PrintString("Server name cannot be empty");
        return; 
    }
}   

void UMultiplayerSessionsSubsystem::FindServer(FString ServerName)
{
    if (!ServerName.IsEmpty())
    {
        SessionSearch = MakeShareable(new FOnlineSessionSearch());

        // Set MySessionName to the server name passed in
        MySessionName = FName(*ServerName);

        bool IsLAN = false;
        if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
        {
            IsLAN = true;
        }
        SessionSearch->bIsLanQuery = IsLAN;
        SessionSearch->MaxSearchResults = 9999;
        SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

        ServerNameSearched = ServerName;

        // Use MySessionName here
        SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
    }
    else
    {   
        ServerJoinDel.Broadcast(false);
        PrintString("Server name cannot be empty");
        return; 
    }
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool WasSuccessful)
{
    ServerCreateDel.Broadcast(WasSuccessful);

    if (WasSuccessful)
    {
        GetWorld()->ServerTravel("/Game/MENU_CharacterSelect?listen");
    }
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool WasSuccessful)
{
    if (CreateServerAfterDestroy)
    {
        CreateServerAfterDestroy = false;
        CreateServer(DestroyServerName);
    }
}

void UMultiplayerSessionsSubsystem::OnFindSessionComplete(bool WasSuccessful)
{
    if (!WasSuccessful) return;
    if (ServerNameSearched.IsEmpty()) return;

    TArray<FOnlineSessionSearchResult> Results = SessionSearch->SearchResults;
    FOnlineSessionSearchResult* CorrectResult = 0;

    if (Results.Num() > 0)
    {
        FString msg = FString::Printf(TEXT("%d Sessions Found"), Results.Num());
        PrintString(msg);

        for (FOnlineSessionSearchResult Result : Results)
        {
            if (Result.IsValid())
            {
                FString ServerName = "No-name";
                Result.Session.SessionSettings.Get(FName("SERVER_NAME"), ServerName);

                if (ServerName.Equals(ServerNameSearched))
                {
                    CorrectResult = &Result;
                    FString Msg2 = FString::Printf(TEXT("Found Server Name: %s"), *ServerNameSearched);
                    PrintString(Msg2);
                    break;
                }
            }
        }

        if (CorrectResult)
        {
            SessionInterface->JoinSession(0, MySessionName, *CorrectResult);
        }
        else
        {
            
            FString MsgServerNotFound = FString::Printf(TEXT("Couldn't Find Server Name: %s"), *ServerNameSearched);
            PrintString(MsgServerNotFound);
            ServerJoinDel.Broadcast(false);
            ServerNameSearched = "";
        }
    }
    else
    {
        PrintString("No servers found");
        ServerJoinDel.Broadcast(false);
    }
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{

    ServerJoinDel.Broadcast(EOnJoinSessionCompleteResult::Success);

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        FString MsgJoiningSession = FString::Printf(TEXT("Joining Session %s"), *SessionName.ToString());
        PrintString(MsgJoiningSession);
        FString Address = ""; 
        bool AddressStored = SessionInterface->GetResolvedConnectString(SessionName, Address);  // Use SessionName here
        if (AddressStored)
        {
            APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
            if (PlayerController)
            {
                PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
            }
        }
        else
        {
            // If the address wasn't stored, print a message
            PrintString(TEXT("Failed to resolve connection address"));
        }
    }
    else
    {
        PrintString("Failed to connect to server");
    }
}
