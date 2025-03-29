#include "Customization/CustomizationManager.h"
#include "Vehicles/BaseVehicle.h"
#include "Characters/ExplorerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"

UCustomizationManager::UCustomizationManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    SavedOutfitIndex = 0;
    SavedHeadIndex = 0;
    SavedMaterialIndex = 0;
}

void UCustomizationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Load player's saved customization preferences
    LoadCustomizationPreferences();
}

void UCustomizationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCustomizationManager::ApplyVehicleCustomization(ABaseVehicle* Vehicle, const TMap<FString, int32>& SelectedParts, int32 ColorIndex)
{
    if (!Vehicle || !CustomizationDatabase)
        return;

    // Apply color customization
    if (CustomizationDatabase->AvailableVehicleColors.IsValidIndex(ColorIndex))
    {
        FLinearColor SelectedColor = CustomizationDatabase->AvailableVehicleColors[ColorIndex];
        Vehicle->SetVehicleColor(SelectedColor);
    }

    // Apply parts customization
    for (const auto& PartSelection : SelectedParts)
    {
        const FString& PartCategory = PartSelection.Key;
        int32 PartIndex = PartSelection.Value;

        if (CustomizationDatabase->VehiclePartsOptions.Contains(PartCategory))
        {
            const TArray<FVehiclePartOption>& PartOptions = CustomizationDatabase->VehiclePartsOptions[PartCategory];
            
            if (PartOptions.IsValidIndex(PartIndex) && PartOptions[PartIndex].PartMesh)
            {
                // Create a new part component
                UStaticMeshComponent* PartComponent = NewObject<UStaticMeshComponent>(Vehicle);
                PartComponent->SetStaticMesh(PartOptions[PartIndex].PartMesh);
                
                // Attach it to the vehicle at the appropriate socket (socket names would match part categories)
                Vehicle->AddVehicleAccessory(PartComponent, FName(*PartCategory));
            }
        }
    }

    // Save the customization
    FString VehicleType = Vehicle->GetClass()->GetName();
    SavedVehicleCustomizations.Add(VehicleType, SelectedParts);
    
    SaveCustomizationPreferences();
}

void UCustomizationManager::ApplyCharacterCustomization(AExplorerCharacter* Character, int32 OutfitIndex, int32 HeadIndex, int32 MaterialIndex)
{
    if (!Character || !CustomizationDatabase)
        return;

    // Apply outfit customization
    if (CustomizationDatabase->CharacterOutfits.IsValidIndex(OutfitIndex))
    {
        const FCharacterOutfitOption& OutfitOption = CustomizationDatabase->CharacterOutfits[OutfitIndex];
        
        if (OutfitOption.OutfitMesh)
        {
            // Apply the material if valid
            if (OutfitOption.AvailableMaterials.IsValidIndex(MaterialIndex))
            {
                Character->SetCharacterOutfit(OutfitOption.AvailableMaterials[MaterialIndex]);
            }
        }
    }

    // Apply head customization
    if (CustomizationDatabase->CharacterHeadOptions.IsValidIndex(HeadIndex))
    {
        USkeletalMesh* HeadMesh = CustomizationDatabase->CharacterHeadOptions[HeadIndex];
        USkeletalMesh* BodyMesh = nullptr;
        
        if (CustomizationDatabase->CharacterOutfits.IsValidIndex(OutfitIndex))
        {
            BodyMesh = CustomizationDatabase->CharacterOutfits[OutfitIndex].OutfitMesh;
        }
        
        Character->SetCharacterAppearance(HeadMesh, BodyMesh);
    }

    // Save the player's selections
    SavedOutfitIndex = OutfitIndex;
    SavedHeadIndex = HeadIndex;
    SavedMaterialIndex = MaterialIndex;
    
    SaveCustomizationPreferences();
}

void UCustomizationManager::SaveCustomizationPreferences()
{
    // This would be implemented to save to a UE SaveGame object
    // For a complete implementation, we'd create a custom USaveGame subclass
    // to store all these preferences
    
    // Example pseudocode:
    // UCustomizationSaveGame* SaveGameInstance = Cast<UCustomizationSaveGame>(UGameplayStatics::CreateSaveGameObject(UCustomizationSaveGame::StaticClass()));
    // SaveGameInstance->SavedVehicleCustomizations = SavedVehicleCustomizations;
    // SaveGameInstance->SavedOutfitIndex = SavedOutfitIndex;
    // SaveGameInstance->SavedHeadIndex = SavedHeadIndex;
    // SaveGameInstance->SavedMaterialIndex = SavedMaterialIndex;
    // UGameplayStatics::SaveGameToSlot(SaveGameInstance, "CustomizationSave", 0);
}

void UCustomizationManager::LoadCustomizationPreferences()
{
    // This would be implemented to load from a UE SaveGame object
    
    // Example pseudocode:
    // if (UGameplayStatics::DoesSaveGameExist("CustomizationSave", 0))
    // {
    //     UCustomizationSaveGame* SaveGameInstance = Cast<UCustomizationSaveGame>(UGameplayStatics::LoadGameFromSlot("CustomizationSave", 0));
    //     if (SaveGameInstance)
    //     {
    //         SavedVehicleCustomizations = SaveGameInstance->SavedVehicleCustomizations;
    //         SavedOutfitIndex = SaveGameInstance->SavedOutfitIndex;
    //         SavedHeadIndex = SaveGameInstance->SavedHeadIndex;
    //         SavedMaterialIndex = SaveGameInstance->SavedMaterialIndex;
    //     }
    // }
}