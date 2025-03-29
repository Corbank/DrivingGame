#include "World/ProgressionSystem.h"
#include "Vehicles/BaseVehicle.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

// SaveGame class for progression data
UCLASS()
class UProgressionSaveGame : public USaveGame
{
    GENERATED_BODY()
    
public:
    UPROPERTY()
    TArray<FDiscoveredLocation> DiscoveredLocations;
    
    UPROPERTY()
    TArray<FVehicleUnlock> VehicleUnlocks;
    
    UPROPERTY()
    TArray<FCustomizationUnlock> CustomizationUnlocks;
    
    UPROPERTY()
    TArray<FAchievement> Achievements;
    
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
};

UProgressionSystem::UProgressionSystem()
{
    TotalDistanceTraveled = 0.0f;
    DistanceTraveledByVehicle = 0.0f;
    DistanceTraveledOnFoot = 0.0f;
    TotalPhotosTaken = 0;
    ExplorationPoints = 0;
    ExplorationLevel = 1;
    
    // Define level thresholds
    LevelThresholds = { 0, 1000, 2500, 5000, 10000, 15000, 25000, 40000, 60000, 100000 };
}

void UProgressionSystem::Tick(float DeltaTime)
{
    // Nothing to tick for now
}

bool UProgressionSystem::IsTickable() const
{
    return !IsTemplate() && !IsPendingKill();
}

TStatId UProgressionSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UProgressionSystem, STATGROUP_Tickables);
}

void UProgressionSystem::Initialize()
{
    // Try to load saved progress data
    if (!LoadProgressionData())
    {
        // If no save data, set up default values
        CreateDefaultAchievements();
        SetupDefaultVehicles();
        SetupDefaultCustomizations();
        SaveProgressionData();
    }
}

bool UProgressionSystem::SaveProgressionData()
{
    UProgressionSaveGame* SaveGameInstance = Cast<UProgressionSaveGame>(UGameplayStatics::CreateSaveGameObject(UProgressionSaveGame::StaticClass()));
    if (SaveGameInstance)
    {
        // Copy current data to save game
        SaveGameInstance->DiscoveredLocations = DiscoveredLocations;
        SaveGameInstance->VehicleUnlocks = VehicleUnlocks;
        SaveGameInstance->CustomizationUnlocks = CustomizationUnlocks;
        SaveGameInstance->Achievements = Achievements;
        SaveGameInstance->TotalDistanceTraveled = TotalDistanceTraveled;
        SaveGameInstance->DistanceTraveledByVehicle = DistanceTraveledByVehicle;
        SaveGameInstance->DistanceTraveledOnFoot = DistanceTraveledOnFoot;
        SaveGameInstance->TotalPhotosTaken = TotalPhotosTaken;
        SaveGameInstance->ExplorationPoints = ExplorationPoints;
        SaveGameInstance->ExplorationLevel = ExplorationLevel;
        
        // Save game to slot
        return UGameplayStatics::SaveGameToSlot(SaveGameInstance, "ProgressionSave", 0);
    }
    
    return false;
}

bool UProgressionSystem::LoadProgressionData()
{
    if (UGameplayStatics::DoesSaveGameExist("ProgressionSave", 0))
    {
        UProgressionSaveGame* SaveGameInstance = Cast<UProgressionSaveGame>(UGameplayStatics::LoadGameFromSlot("ProgressionSave", 0));
        if (SaveGameInstance)
        {
            // Copy saved data to current state
            DiscoveredLocations = SaveGameInstance->DiscoveredLocations;
            VehicleUnlocks = SaveGameInstance->VehicleUnlocks;
            CustomizationUnlocks = SaveGameInstance->CustomizationUnlocks;
            Achievements = SaveGameInstance->Achievements;
            TotalDistanceTraveled = SaveGameInstance->TotalDistanceTraveled;
            DistanceTraveledByVehicle = SaveGameInstance->DistanceTraveledByVehicle;
            DistanceTraveledOnFoot = SaveGameInstance->DistanceTraveledOnFoot;
            TotalPhotosTaken = SaveGameInstance->TotalPhotosTaken;
            ExplorationPoints = SaveGameInstance->ExplorationPoints;
            ExplorationLevel = SaveGameInstance->ExplorationLevel;
            
            return true;
        }
    }
    
    return false;
}

void UProgressionSystem::RegisterDiscoveredLocation(const FString& LocationName, const FVector& Coordinates)
{
    // Check if this location has already been discovered
    for (FDiscoveredLocation& Location : DiscoveredLocations)
    {
        if (Location.LocationName == LocationName)
        {
            // Already discovered, mark as visited
            Location.bHasBeenVisited = true;
            return;
        }
    }
    
    // New discovery
    FDiscoveredLocation NewLocation;
    NewLocation.LocationName = LocationName;
    NewLocation.LocationCoordinates = Coordinates;
    NewLocation.bHasBeenVisited = true;
    NewLocation.bHasBeenPhotographed = false;
    NewLocation.DiscoveryTime = FDateTime::Now();
    
    DiscoveredLocations.Add(NewLocation);
    
    // Award points for discovery
    AwardExplorationPoints(100);
    
    // Update achievements
    UpdateAchievementProgress("Discoveries", DiscoveredLocations.Num());
    
    // Check for unlocks based on new discovery
    CheckForUnlocks();
    
    // Save progress
    SaveProgressionData();
}

void UProgressionSystem::RegisterLocationPhotographed(const FString& LocationName)
{
    bool bFound = false;
    
    // Find the location in the discovered locations
    for (FDiscoveredLocation& Location : DiscoveredLocations)
    {
        if (Location.LocationName == LocationName)
        {
            // Check if it's already been photographed
            if (!Location.bHasBeenPhotographed)
            {
                Location.bHasBeenPhotographed = true;
                TotalPhotosTaken++;
                
                // Award points for photographing a location
                AwardExplorationPoints(50);
                
                // Update achievement progress
                UpdateAchievementProgress("Photos", TotalPhotosTaken);
                
                // Save progress
                SaveProgressionData();
            }
            
            bFound = true;
            break;
        }
    }
    
    // If the location wasn't in our discovered list, just count the photo
    if (!bFound)
    {
        TotalPhotosTaken++;
        UpdateAchievementProgress("Photos", TotalPhotosTaken);
        SaveProgressionData();
    }
}

void UProgressionSystem::RegisterDistanceTraveled(float DistanceInMeters, bool bInVehicle)
{
    // Update distance statistics
    TotalDistanceTraveled += DistanceInMeters;
    
    if (bInVehicle)
    {
        DistanceTraveledByVehicle += DistanceInMeters;
        UpdateAchievementProgress("VehicleDistance", DistanceTraveledByVehicle);
    }
    else
    {
        DistanceTraveledOnFoot += DistanceInMeters;
        UpdateAchievementProgress("FootDistance", DistanceTraveledOnFoot);
    }
    
    // Update total distance achievement
    UpdateAchievementProgress("TotalDistance", TotalDistanceTraveled);
    
    // Award exploration points (1 point per 10 meters)
    int32 PointsToAward = FMath::FloorToInt(DistanceInMeters / 10.0f);
    if (PointsToAward > 0)
    {
        AwardExplorationPoints(PointsToAward);
    }
    
    // Save periodically (e.g., every 500 meters)
    if (FMath::Fmod(TotalDistanceTraveled, 500.0f) < DistanceInMeters)
    {
        SaveProgressionData();
    }
}

bool UProgressionSystem::IsVehicleUnlocked(const FString& VehicleName) const
{
    for (const FVehicleUnlock& Vehicle : VehicleUnlocks)
    {
        if (Vehicle.VehicleName == VehicleName)
        {
            return Vehicle.bIsUnlocked;
        }
    }
    
    // Vehicle not found in list
    return false;
}

TArray<FVehicleUnlock> UProgressionSystem::GetAllVehicles() const
{
    return VehicleUnlocks;
}

TArray<FVehicleUnlock> UProgressionSystem::GetUnlockedVehicles() const
{
    TArray<FVehicleUnlock> UnlockedVehicles;
    
    for (const FVehicleUnlock& Vehicle : VehicleUnlocks)
    {
        if (Vehicle.bIsUnlocked)
        {
            UnlockedVehicles.Add(Vehicle);
        }
    }
    
    return UnlockedVehicles;
}

bool UProgressionSystem::IsCustomizationUnlocked(const FString& Category, const FString& ItemType, const FString& ItemID) const
{
    for (const FCustomizationUnlock& Customization : CustomizationUnlocks)
    {
        if (Customization.Category == Category && 
            Customization.ItemType == ItemType && 
            Customization.ItemID == ItemID)
        {
            return Customization.bIsUnlocked;
        }
    }
    
    // Item not found in list
    return false;
}

TArray<FCustomizationUnlock> UProgressionSystem::GetCustomizationUnlocks(const FString& Category, const FString& ItemType) const
{
    TArray<FCustomizationUnlock> FilteredUnlocks;
    
    for (const FCustomizationUnlock& Customization : CustomizationUnlocks)
    {
        if (Customization.Category == Category && 
            (ItemType.IsEmpty() || Customization.ItemType == ItemType))
        {
            FilteredUnlocks.Add(Customization);
        }
    }
    
    return FilteredUnlocks;
}

TArray<FDiscoveredLocation> UProgressionSystem::GetDiscoveredLocations() const
{
    return DiscoveredLocations;
}

int32 UProgressionSystem::GetTotalDiscoveries() const
{
    return DiscoveredLocations.Num();
}

float UProgressionSystem::GetTotalDistanceTraveled() const
{
    return TotalDistanceTraveled;
}

float UProgressionSystem::GetTotalDistanceByVehicle() const
{
    return DistanceTraveledByVehicle;
}

float UProgressionSystem::GetTotalDistanceOnFoot() const
{
    return DistanceTraveledOnFoot;
}

int32 UProgressionSystem::GetTotalPhotos() const
{
    return TotalPhotosTaken;
}

int32 UProgressionSystem::GetExplorationLevel() const
{
    return ExplorationLevel;
}

int32 UProgressionSystem::GetCurrentExplorationPoints() const
{
    return ExplorationPoints;
}

int32 UProgressionSystem::GetPointsForNextLevel() const
{
    // If at max level, return 0
    if (ExplorationLevel >= LevelThresholds.Num())
    {
        return 0;
    }
    
    return LevelThresholds[ExplorationLevel];
}

TArray<FAchievement> UProgressionSystem::GetAllAchievements() const
{
    return Achievements;
}

TArray<FAchievement> UProgressionSystem::GetUnlockedAchievements() const
{
    TArray<FAchievement> UnlockedAchievements;
    
    for (const FAchievement& Achievement : Achievements)
    {
        if (Achievement.bIsUnlocked)
        {
            UnlockedAchievements.Add(Achievement);
        }
    }
    
    return UnlockedAchievements;
}

void UProgressionSystem::UpdateAchievementProgress(const FString& AchievementType, float Progress)
{
    bool bAchievementUnlocked = false;
    
    // Update progress for all matching achievements
    for (FAchievement& Achievement : Achievements)
    {
        if (Achievement.AchievementType == AchievementType)
        {
            // Update progress
            Achievement.CurrentProgress = FMath::Max(Achievement.CurrentProgress, Progress);
            
            // Check if achievement is newly completed
            if (!Achievement.bIsUnlocked && Achievement.CurrentProgress >= Achievement.TargetValue)
            {
                Achievement.bIsUnlocked = true;
                bAchievementUnlocked = true;
                
                // Award points for completing achievement
                AwardExplorationPoints(Achievement.RewardPoints);
            }
        }
    }
    
    // Check for unlocks if an achievement was completed
    if (bAchievementUnlocked)
    {
        CheckForUnlocks();
        SaveProgressionData();
    }
    
    // Always check for new achievement progress
    CheckAchievements();
}

void UProgressionSystem::UpdateExplorationLevel()
{
    // Find the level based on current points
    int32 NewLevel = 1;
    for (int32 i = 1; i < LevelThresholds.Num(); ++i)
    {
        if (ExplorationPoints >= LevelThresholds[i])
        {
            NewLevel = i + 1;
        }
        else
        {
            break;
        }
    }
    
    // Check if level has increased
    if (NewLevel > ExplorationLevel)
    {
        ExplorationLevel = NewLevel;
        CheckForUnlocks();
    }
}

void UProgressionSystem::CheckForUnlocks()
{
    bool bUnlocksMade = false;
    
    // Check vehicle unlocks
    for (FVehicleUnlock& Vehicle : VehicleUnlocks)
    {
        if (!Vehicle.bIsUnlocked && ExplorationPoints >= Vehicle.RequiredExplorationPoints)
        {
            // Check if all required discoveries have been made
            bool bAllDiscoveriesFound = true;
            for (const FString& RequiredDiscovery : Vehicle.RequiredDiscoveries)
            {
                bool bFound = false;
                for (const FDiscoveredLocation& Location : DiscoveredLocations)
                {
                    if (Location.LocationName == RequiredDiscovery)
                    {
                        bFound = true;
                        break;
                    }
                }
                
                if (!bFound)
                {
                    bAllDiscoveriesFound = false;
                    break;
                }
            }
            
            if (bAllDiscoveriesFound)
            {
                Vehicle.bIsUnlocked = true;
                bUnlocksMade = true;
            }
        }
    }
    
    // Check customization unlocks
    for (FCustomizationUnlock& Customization : CustomizationUnlocks)
    {
        if (!Customization.bIsUnlocked && ExplorationPoints >= Customization.RequiredExplorationPoints)
        {
            Customization.bIsUnlocked = true;
            bUnlocksMade = true;
        }
    }
    
    // Save if any unlocks were made
    if (bUnlocksMade)
    {
        SaveProgressionData();
    }
}

void UProgressionSystem::AwardExplorationPoints(int32 Points)
{
    ExplorationPoints += Points;
    UpdateExplorationLevel();
}

void UProgressionSystem::CheckAchievements()
{
    // This function could have additional logic for checking achievement progress
}

void UProgressionSystem::CreateDefaultAchievements()
{
    Achievements.Empty();
    
    // Distance-based achievements
    Achievements.Add(CreateAchievement("Road Tripper", "Travel 10 km in vehicles", "VehicleDistance", 10000.0f, 250));
    Achievements.Add(CreateAchievement("Off the Beaten Path", "Travel 5 km on foot", "FootDistance", 5000.0f, 200));
    Achievements.Add(CreateAchievement("Globetrotter", "Travel a total of 50 km", "TotalDistance", 50000.0f, 500));
    Achievements.Add(CreateAchievement("World Explorer", "Travel a total of 100 km", "TotalDistance", 100000.0f, 1000));
    
    // Discovery-based achievements
    Achievements.Add(CreateAchievement("Sightseer", "Discover 5 locations", "Discoveries", 5.0f, 150));
    Achievements.Add(CreateAchievement("Explorer", "Discover 15 locations", "Discoveries", 15.0f, 300));
    Achievements.Add(CreateAchievement("Cartographer", "Discover all locations", "Discoveries", 30.0f, 1000));
    
    // Photography-based achievements
    Achievements.Add(CreateAchievement("Shutterbug", "Take 10 photographs", "Photos", 10.0f, 100));
    Achievements.Add(CreateAchievement("Photographer", "Take 25 photographs", "Photos", 25.0f, 250));
    Achievements.Add(CreateAchievement("Photojournalist", "Photograph 15 different locations", "LocationPhotos", 15.0f, 300));
}

void UProgressionSystem::SetupDefaultVehicles()
{
    FVehicleUnlock DefaultCar;
    DefaultCar.VehicleName = "Standard Sedan";
    DefaultCar.bIsUnlocked = true;
    DefaultCar.RequiredExplorationPoints = 0;
    VehicleUnlocks.Add(DefaultCar);
    
    FVehicleUnlock SUV;
    SUV.VehicleName = "Explorer SUV";
    SUV.bIsUnlocked = false;
    SUV.RequiredExplorationPoints = 2000;
    VehicleUnlocks.Add(SUV);
    
    FVehicleUnlock SportsCar;
    SportsCar.VehicleName = "Sports Coupe";
    SportsCar.bIsUnlocked = false;
    SportsCar.RequiredExplorationPoints = 5000;
    VehicleUnlocks.Add(SportsCar);
}

void UProgressionSystem::SetupDefaultCustomizations()
{
    // Vehicle paint colors
    TArray<FString> PaintColors = { "Red", "Blue", "White", "Black", "Silver" };
    for (int32 i = 0; i < PaintColors.Num(); i++)
    {
        FCustomizationUnlock PaintUnlock;
        PaintUnlock.UnlockName = PaintColors[i] + " Paint";
        PaintUnlock.Category = "Vehicle";
        PaintUnlock.ItemType = "Paint";
        PaintUnlock.ItemID = PaintColors[i];
        PaintUnlock.bIsUnlocked = (i < 3); // First three are unlocked by default
        PaintUnlock.RequiredExplorationPoints = i * 500;
        CustomizationUnlocks.Add(PaintUnlock);
    }
    
    // Vehicle wheels
    TArray<FString> WheelTypes = { "Standard", "Sport", "Offroad", "Luxury" };
    for (int32 i = 0; i < WheelTypes.Num(); i++)
    {
        FCustomizationUnlock WheelUnlock;
        WheelUnlock.UnlockName = WheelTypes[i] + " Wheels";
        WheelUnlock.Category = "Vehicle";
        WheelUnlock.ItemType = "Wheels";
        WheelUnlock.ItemID = WheelTypes[i];
        WheelUnlock.bIsUnlocked = (i < 2); // First two are unlocked by default
        WheelUnlock.RequiredExplorationPoints = i * 800;
        CustomizationUnlocks.Add(WheelUnlock);
    }
    
    // Character outfits
    TArray<FString> OutfitTypes = { "Casual", "Explorer", "Formal", "Sport" };
    for (int32 i = 0; i < OutfitTypes.Num(); i++)
    {
        FCustomizationUnlock OutfitUnlock;
        OutfitUnlock.UnlockName = OutfitTypes[i] + " Outfit";
        OutfitUnlock.Category = "Character";
        OutfitUnlock.ItemType = "Outfit";
        OutfitUnlock.ItemID = OutfitTypes[i];
        OutfitUnlock.bIsUnlocked = (i < 2); // First two are unlocked by default
        OutfitUnlock.RequiredExplorationPoints = i * 1000;
        CustomizationUnlocks.Add(OutfitUnlock);
    }
}

FAchievement UProgressionSystem::CreateAchievement(const FString& Name, const FString& Description, const FString& Type, float Target, int32 Reward)
{
    FAchievement Achievement;
    Achievement.AchievementName = Name;
    Achievement.Description = Description;
    Achievement.AchievementType = Type;
    Achievement.TargetValue = Target;
    Achievement.RewardPoints = Reward;
    Achievement.CurrentProgress = 0.0f;
    Achievement.bIsUnlocked = false;
    
    return Achievement;
}