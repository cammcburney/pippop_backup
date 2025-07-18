// Fill out your copyright notice in the Description page of Project Settings.

#include "UIAmmo.h"
#include "Kismet/KismetStringLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "Engine/Font.h"
#include "UObject/ConstructorHelpers.h"

UUIAmmo::UUIAmmo(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PC = nullptr;
    PlayerPawn = nullptr;
}

void UUIAmmo::NativeConstruct()
{
    Super::NativeConstruct();
    PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        PlayerPawn = Cast<AFirstPersonCharacter>(PC->GetPawn());
    }
    
    InitializeAmmoDisplay();
    
}

void UUIAmmo::InitializeAmmoDisplay()
{
    AmmoDisplay->ClearChildren();
    UObject* FontObj = StaticLoadObject(UFont::StaticClass(), NULL, TEXT("/Game/Player/UI/Starborn_Font.Starborn_Font"));
    UFont* FontType = Cast<UFont>(FontObj);
    FSlateFontInfo FontInfo;
    FontInfo.FontObject = FontType;
    
    for (int32 i = 0; i < 9; i++)
    {
        UTextBlock* NumBlock = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
        FontInfo.Size = 24;
        NumBlock->SetFont(FontInfo);
        UHorizontalBoxSlot* NumSlot = AmmoDisplay->AddChildToHorizontalBox(NumBlock);
        NumSlot->SetPadding(FMargin(10.0f, 10.0f, 10.0f, 10.0f));
        NumSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
        NumSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
    }
    TextBlocks = AmmoDisplay->GetAllChildren();
    UpdateAmmoDisplay();
}

void UUIAmmo::UpdateAmmoDisplay()
{   
    APlayerController* PcC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    AFirstPersonCharacter* PP = Cast<AFirstPersonCharacter>(PcC->GetPawn());
    
    if (PP)
    {
        if (!PP->UpdateGunDelegate.IsBound())
        {
            PP->UpdateGunDelegate.AddDynamic(this, &UUIAmmo::UpdateGun);
        }
        
        GunData = PP->GetGunData();
        ABaseGun* PlayerGun = PP->GetGun();
        
        if (PlayerGun && CurrentBoundGun != PlayerGun)
        {
            if (CurrentBoundGun)
            {
                CurrentBoundGun->UpdateAmmoDelegate.RemoveDynamic(this, &UUIAmmo::UpdateAmmoDisplay);
            }
            PlayerGun->UpdateAmmoDelegate.AddDynamic(this, &UUIAmmo::UpdateAmmoDisplay);
            CurrentBoundGun = PlayerGun;
        }
        
        if (GunData.Num() > 0) 
        {
            CharArray.Empty();
            FString AmmoDisplayValues = FString::Printf(TEXT("%d/%d"), GunData[0], GunData[1]);
            for (int32 i = 0; i < AmmoDisplayValues.Len(); i++) 
            {
                CharArray.Add(FString::Chr(AmmoDisplayValues[i]));
            }
            
            for (int32 i = 0; i < TextBlocks.Num(); i++)
            {
                if (UTextBlock* TextBlock = Cast<UTextBlock>(TextBlocks[i]))
                {
                    if (i < CharArray.Num())
                    {
                        TextBlock->SetText(FText::FromString(CharArray[i]));
                    }
                    else
                    {
                        TextBlock->SetText(FText::GetEmpty());
                    }
                }
            }
        }
    }
}

void UUIAmmo::UpdateGun()
{
    APlayerController* PcC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    AFirstPersonCharacter* PP = Cast<AFirstPersonCharacter>(PcC->GetPawn());
    if (PP)
    {
        if (CurrentBoundGun)
        {
            CurrentBoundGun->UpdateAmmoDelegate.RemoveDynamic(this, &UUIAmmo::UpdateAmmoDisplay);
        }
        
        ABaseGun* PlayerGun = PP->GetGun();
        if (PlayerGun)
        {
            PlayerGun->UpdateAmmoDelegate.AddDynamic(this, &UUIAmmo::UpdateAmmoDisplay);
            CurrentBoundGun = PlayerGun;
        }
        
        UpdateAmmoDisplay();
    }
}

void UUIAmmo::NativeDestruct()
{
    if (CurrentBoundGun)
    {
        CurrentBoundGun->UpdateAmmoDelegate.RemoveDynamic(this, &UUIAmmo::UpdateAmmoDisplay);
        CurrentBoundGun = nullptr;
    }
    
    APlayerController* PcC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PcC)
    {
        AFirstPersonCharacter* PP = Cast<AFirstPersonCharacter>(PcC->GetPawn());
        if (PP)
        {
            PP->UpdateGunDelegate.RemoveDynamic(this, &UUIAmmo::UpdateGun);
        }
    }
    
    Super::NativeDestruct();
}