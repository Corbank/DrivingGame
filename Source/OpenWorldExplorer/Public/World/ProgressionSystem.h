#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ProgressionSystem.generated.h"

// Struct to represent a discovered location
USTRUCT(BlueprintType)
struct FDiscoveredLocation
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LocationName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LocationCoordinates;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasBeenVisited;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasBeenPhotographed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime DiscoveryTime;
};

// Struct to represent a vehicle unlock
USTRUCT(BlueprintType)
struct FVehicleUnlock
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VehicleName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ABaseVehicle> VehicleClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnlocked;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredExplorationPoints;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredDiscoveries;
};

// Struct to represent a customization unlock
USTRUCT(BlueprintType)
struct FCustomizationUnlock
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString UnlockName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Category; // "Vehicle" or "Character"
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ItemType; // "Paint", "Wheels", "Outfit", etc.
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ItemID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnlocked;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredExplorationPoints;
};

// Structure for achievement data
USTRUCT(BlueprintType)
struct FAchievement
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AchievementName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnlocked;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RewardPoints;
    
    // Achievement type - distance, discoveries, photos, etc.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AchievementType;
    
    // Target value to complete the achievement
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetValue;
    
    // Current progress towards completion
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentProgress;
};

/**
 * Progression system for tracking player exploration, discoveries, and unlocks
 */
UCLASS(BlueprintType)
class OPENWORLDEXPLORER_API UProgressionSystem : public UObject, public FTickableGameObject
{
    GENERATED_BODY()
    
public:
    UProgressionSystem();
    
    // FTickableGameObject interface
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override;
    virtual TStatId GetStatId() const override;
    
    // Initialize the progression system with data
    UFUNCTION(BlueprintCallable, Category = "Progression")
    void Initialize();
    
    // Save and load progression data
    UFUNCTION(BlueprintCallable, Category = "Progression")
    bool SaveProgressionData();
    
    UFUNCTION(BlueprintCallable, Category = "Progression")
    bool LoadProgressionData();
    
    // Register a discovered location
    UFUNCTION(BlueprintCallable, Category = "Progression|Exploration")
    void RegisterDiscoveredLocation(const FString& LocationName, const FVector& Coordinates);
    
    // Register a location as photographed
    UFUNCTION(BlueprintCallable, Category = "Progression|Exploration")
    void RegisterLocationPhotographed(const FString& LocationName);
    
    // Register distance traveled (any mode)
    UFUNCTION(BlueprintCallable, Category = "Progression|Exploration")
    void RegisterDistanceTraveled(float DistanceInMeters, bool bInVehicle);
    
    // Check if a vehicle is unlocked
    UFUNCTION(BlueprintCallable, Category = "Progression|Unlocks")
    bool IsVehicleUnlocked(const FString& VehicleName) const;
    
    // Get all available vehicles (unlocked or not)
    UFUNCTION(BlueprintCallable, Category = "Progression|Unlocks")
    TArray<FVehicleUnlock> GetAllVehicles() const;
    
    // Get only unlocked vehicles
    UFUNCTION(BlueprintCallable, Category = "Progression|Unlocks")
    TArray<FVehicleUnlock> GetUnlockedVehicles() const;
    
    // Check if a customization item is unlocked
    UFUNCTION(BlueprintCallable, Category = "Progression|Unlocks")
    bool IsCustomizationUnlocked(const FString& Category, const FString& ItemType, const FString& ItemID) const;
    
    // Get all customization unlocks for a category and type
    UFUNCTION(BlueprintCallable, Category = "Progression|Unlocks")
    TArray<FCustomizationUnlock> GetCustomizationUnlocks(const FString& Category, const FString& ItemType) const;
    
    // Get all discovered locations
    UFUNCTION(BlueprintCallable, Category = "Progression|Exploration")
    TArray<FDiscoveredLocation> GetDiscoveredLocations() const;
    
    // Get exploration statistics
    UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
    int32 GetTotalDiscoveries() const;
    
    UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
    float GetTotalDistanceTraveled() const;
    
    UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
    float GetTotalDistanceByVehicle() const;
    
    UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
    float GetTotalDistanceOnFoot() const;
    
    UFUNCTION(BlueprintCallable, Category = "Progression|Stats")
    int32 GetTotalPhotos() const;
    
    // Get current exploration level and points
    UFUNCTION(BlueprintCallable, Category = "Progression|Level")
    int32 GetExplorationLevel() const;
    
    UFUNCTION(BlueprintCallable, Category = "Progression|Level")
    int32 GetCurrentExplorationPoints() const;
    
    UFUNCTION(BlueprintCallable, Category = "Progression|Level")
    int32 GetPointsForNextLevel() const;
    
    // Get all achievements
    UFUNCTION(BlueprintCallable, Category = "Progression|Achievements")
    TArray<FAchievement> GetAllAchievements() const;
    
    // Get unlocked achievements
    UFUNCTION(BlueprintCallable, Category = "Progression|Achievements")
    TArray<FAchievement> GetUnlockedAchievements() const;
    
    // Update achievement progress
    UFUNCTION(BlueprintCallable, Category = "Progression|Achievements")
    void UpdateAchievementProgress(const FString& AchievementType, float Progress);
    
private:
    // All discovered locations
    UPROPERTY()
    TArray<FDiscoveredLocation> DiscoveredLocations;
    
    // All vehicle unlocks
    UPROPERTY()
    TArray<FVehicleUnlock> VehicleUnlocks;
    
    // All customization unlocks
    UPROPERTY()
    TArray<FCustomizationUnlock> CustomizationUnlocks;
    
    // All achievements
    UPROPERTY()
    TArray<FAchievement> Achievements;
    
    // Statistics
    UPROPERTY()
    float TotalDistanceTraveled;
    
    UPROPERTY()
    float DistanceTraveledByVehicle;
    
    UPROPERTY()
    float DistanceTraveledOnFoot;
    
    UPROPERTY()
    int32 TotalPhotosTaken;
    
    UPROPERTY()
    int32 ExplorationPoints;
    
    UPROPERTY()
    int32 ExplorationLevel;
    
    // Calculate exploration level based on points
    void UpdateExplorationLevel();
    
    // Check for unlocks based on current progression
    void CheckForUnlocks();
    
    // Award exploration points
    void AwardExplorationPoints(int32 Points);
    
    // Check for achievement completion
    void CheckAchievements();
    
    // Create default achievements
    void CreateDefaultAchievements();
    
    // Level thresholds - points needed for each level
    TArray<int32> LevelThresholds;
};