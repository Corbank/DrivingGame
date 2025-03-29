#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Customization/CustomizationTypes.h"
#include "CustomizationManager.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OPENWORLDEXPLORER_API UCustomizationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    UCustomizationManager();

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // The customization database asset
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
    UCustomizationDatabase* CustomizationDatabase;

    // Apply vehicle customization
    UFUNCTION(BlueprintCallable, Category = "Customization|Vehicle")
    void ApplyVehicleCustomization(class ABaseVehicle* Vehicle, const TMap<FString, int32>& SelectedParts, int32 ColorIndex);

    // Apply character customization
    UFUNCTION(BlueprintCallable, Category = "Customization|Character")
    void ApplyCharacterCustomization(class AExplorerCharacter* Character, int32 OutfitIndex, int32 HeadIndex, int32 MaterialIndex);

    // Save customization to player profile
    UFUNCTION(BlueprintCallable, Category = "Customization|Save")
    void SaveCustomizationPreferences();

    // Load customization from player profile
    UFUNCTION(BlueprintCallable, Category = "Customization|Save")
    void LoadCustomizationPreferences();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

private:
    // Player's current vehicle customization selections
    TMap<FString, TMap<FString, int32>> SavedVehicleCustomizations;

    // Player's current character customization selections
    int32 SavedOutfitIndex;
    int32 SavedHeadIndex;
    int32 SavedMaterialIndex;
};