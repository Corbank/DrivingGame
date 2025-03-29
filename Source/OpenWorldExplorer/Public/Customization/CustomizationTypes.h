#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CustomizationTypes.generated.h"

// Struct for vehicle part customization options
USTRUCT(BlueprintType)
struct FVehiclePartOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FString PartName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    UStaticMesh* PartMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TArray<UMaterialInstance*> AvailableMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    int32 Price;
};

// Struct for character customization options
USTRUCT(BlueprintType)
struct FCharacterOutfitOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FString OutfitName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    USkeletalMesh* OutfitMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TArray<UMaterialInstance*> AvailableMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    int32 Price;
};

// Main customization data asset class
UCLASS(BlueprintType)
class OPENWORLDEXPLORER_API UCustomizationDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    // Vehicle customization options
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicles")
    TMap<FString, TArray<FVehiclePartOption>> VehiclePartsOptions;

    // Character customization options
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characters")
    TArray<FCharacterOutfitOption> CharacterOutfits;

    // Vehicle paint options
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicles")
    TArray<FLinearColor> AvailableVehicleColors;

    // Character appearance options
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Characters")
    TArray<USkeletalMesh*> CharacterHeadOptions;
};