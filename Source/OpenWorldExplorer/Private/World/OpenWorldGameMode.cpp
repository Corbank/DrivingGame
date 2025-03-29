#include "World/OpenWorldGameMode.h"
#include "World/WorldManager.h"
#include "Customization/CustomizationManager.h"
#include "Customization/CustomizationTypes.h"
#include "Characters/ExplorerCharacter.h"
#include "Vehicles/BaseVehicle.h"
#include "World/PhotographySystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

AOpenWorldGameMode::AOpenWorldGameMode()
{
    // Set default classes
    DefaultPawnClass = AExplorerCharacter::StaticClass();
    
    // Enable ticking
    PrimaryActorTick.bCanEverTick = true;
    
    // Set default player start location
    PlayerStartLocation = FVector(0, 0, 200);
}

void AOpenWorldGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    
    // Load any game settings from saved data
    // This would typically load player progress, unlocked vehicles, etc.
}

void AOpenWorldGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Spawn the world manager if we have a class set
    if (WorldManagerClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        WorldManager = GetWorld()->SpawnActor<AWorldManager>(WorldManagerClass, FTransform::Identity, SpawnParams);
    }
    
    // Load the customization database if we have a class set
    if (CustomizationDatabaseClass)
    {
        CustomizationDatabase = NewObject<UCustomizationDatabase>(this, CustomizationDatabaseClass);
    }
    
    // Add photography system to the player character
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (PlayerPawn)
    {
        AddPhotographySystem(PlayerPawn);
    }
    
    // Spawn some initial vehicles in the world
    SpawnStartingVehicles();
}

void AOpenWorldGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    
    // Handle any game mode level updates here
}

ABaseVehicle* AOpenWorldGameMode::SpawnVehicle(TSubclassOf<ABaseVehicle> VehicleClass, const FTransform& Transform)
{
    if (!VehicleClass)
    {
        return nullptr;
    }
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    ABaseVehicle* SpawnedVehicle = GetWorld()->SpawnActor<ABaseVehicle>(VehicleClass, Transform, SpawnParams);
    
    // Apply default customization if we have a database
    if (SpawnedVehicle && CustomizationDatabase)
    {
        // In a complete implementation, we would apply default customization here
        // or randomly select from available options
    }
    
    return SpawnedVehicle;
}

UPhotographySystem* AOpenWorldGameMode::AddPhotographySystem(APawn* PlayerPawn)
{
    if (!PlayerPawn || !PhotographySystemClass)
    {
        return nullptr;
    }
    
    // Check if the pawn already has a photography system
    UPhotographySystem* ExistingSystem = PlayerPawn->FindComponentByClass<UPhotographySystem>();
    if (ExistingSystem)
    {
        return ExistingSystem;
    }
    
    // Create and attach a new photography system
    UPhotographySystem* PhotoSystem = NewObject<UPhotographySystem>(PlayerPawn, PhotographySystemClass);
    if (PhotoSystem)
    {
        PhotoSystem->RegisterComponent();
    }
    
    return PhotoSystem;
}

void AOpenWorldGameMode::SpawnStartingVehicles()
{
    // In a complete implementation, we would use a data-driven approach to place vehicles
    // For now, we'll just place a few vehicles around the player start
    
    if (AvailableVehicleClasses.Num() == 0)
    {
        return;
    }
    
    // Find the player start location
    FVector StartLocation = PlayerStartLocation;
    TArray<AActor*> PlayerStartActors;
    UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStartActors);
    if (PlayerStartActors.Num() > 0)
    {
        StartLocation = PlayerStartActors[0]->GetActorLocation();
    }
    
    // Spawn a few vehicles around the player start
    const int32 NumVehiclesToSpawn = FMath::Min(3, AvailableVehicleClasses.Num());
    for (int32 i = 0; i < NumVehiclesToSpawn; ++i)
    {
        // Get a random vehicle class
        const int32 VehicleIndex = FMath::RandRange(0, AvailableVehicleClasses.Num() - 1);
        TSubclassOf<ABaseVehicle> VehicleClass = AvailableVehicleClasses[VehicleIndex];
        
        // Calculate spawn position
        const float Angle = 2.0f * PI * i / NumVehiclesToSpawn;
        const float Distance = 500.0f;
        const FVector Offset(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
        const FVector SpawnLocation = StartLocation + Offset;
        
        // Calculate rotation to face inward
        const FRotator SpawnRotation(0.0f, FMath::RadiansToDegrees(Angle) + 180.0f, 0.0f);
        
        // Create transform and spawn the vehicle
        const FTransform SpawnTransform(SpawnRotation, SpawnLocation);
        SpawnVehicle(VehicleClass, SpawnTransform);
    }
}