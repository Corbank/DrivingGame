#include "Vehicles/SUVVehicle.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Sound/SoundCue.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"

ASUVVehicle::ASUVVehicle()
{
    // Create SUV-specific components
    RoofRackMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoofRackMesh"));
    RoofRackMesh->SetupAttachment(VehicleMesh);
    
    BullBarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BullBarMesh"));
    BullBarMesh->SetupAttachment(VehicleMesh);
    
    EngineSound = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSound"));
    EngineSound->SetupAttachment(VehicleMesh);
    EngineSound->bAutoActivate = false;
    
    // Create spotlight components
    LeftSpotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("LeftSpotlight"));
    LeftSpotlight->SetupAttachment(VehicleMesh);
    LeftSpotlight->SetRelativeLocation(FVector(200.0f, -100.0f, 50.0f));
    LeftSpotlight->SetVisibility(false);
    LeftSpotlight->Intensity = 5000.0f;
    LeftSpotlight->OuterConeAngle = 30.0f;
    
    RightSpotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("RightSpotlight"));
    RightSpotlight->SetupAttachment(VehicleMesh);
    RightSpotlight->SetRelativeLocation(FVector(200.0f, 100.0f, 50.0f));
    RightSpotlight->SetVisibility(false);
    RightSpotlight->Intensity = 5000.0f;
    RightSpotlight->OuterConeAngle = 30.0f;
    
    RoofSpotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("RoofSpotlight"));
    RoofSpotlight->SetupAttachment(RoofRackMesh);
    RoofSpotlight->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    RoofSpotlight->SetVisibility(false);
    RoofSpotlight->Intensity = 8000.0f;
    RoofSpotlight->OuterConeAngle = 60.0f;
    
    // Set default SUV properties
    OffroadTractionMultiplier = 1.5f;
    WaterDepthTolerance = 75.0f; // cm
    MaxTorque = 2500.0f;
    
    // Initialize state variables
    bOffroadModeEnabled = false;
    bSpotlightsEnabled = false;
    CurrentTerrainType = NAME_None;
}

void ASUVVehicle::BeginPlay()
{
    Super::BeginPlay();
    
    // Configure chaos vehicle movement for SUV
    UChaosWheeledVehicleMovementComponent* SUVMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement);
    if (SUVMovement)
    {
        // Set engine torque for SUV
        SUVMovement->EngineSetup.MaxTorque = MaxTorque;
        
        // Configure wheel setup for off-road
        for (int32 WheelIdx = 0; WheelIdx < SUVMovement->WheelSetups.Num(); WheelIdx++)
        {
            // Increase suspension to handle rough terrain
            SUVMovement->WheelSetups[WheelIdx].SuspensionMaxRaise = 15.0f;
            SUVMovement->WheelSetups[WheelIdx].SuspensionMaxDrop = 15.0f;
            
            // Adjust damping for off-road
            SUVMovement->WheelSetups[WheelIdx].SuspensionDampingRatio = 0.7f;
        }
        
        // Start the engine sound
        if (EngineSound)
        {
            EngineSound->Play();
        }
    }
}

void ASUVVehicle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update terrain detection for vehicle handling
    UpdateTerrainDetection();
    
    // Update engine sound based on RPM
    UChaosWheeledVehicleMovementComponent* SUVMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement);
    if (SUVMovement && EngineSound)
    {
        float CurrentRPM = SUVMovement->GetEngineRotationSpeed();
        float RPMRatio = FMath::Clamp(CurrentRPM / 7000.0f, 0.0f, 1.0f);
        
        // Adjust pitch based on RPM
        float BasePitch = 0.8f;
        float MaxPitchMultiplier = 2.5f;
        float CurrentPitch = BasePitch + (MaxPitchMultiplier - BasePitch) * RPMRatio;
        
        EngineSound->SetPitchMultiplier(CurrentPitch);
        
        // Adjust volume based on RPM
        float MinVolume = 0.4f;
        float MaxVolume = 1.0f;
        float CurrentVolume = MinVolume + (MaxVolume - MinVolume) * RPMRatio;
        
        EngineSound->SetVolumeMultiplier(CurrentVolume);
    }
}

void ASUVVehicle::ToggleSpotlights(bool bEnabled)
{
    bSpotlightsEnabled = bEnabled;
    
    // Toggle all spotlight visibility
    LeftSpotlight->SetVisibility(bEnabled);
    RightSpotlight->SetVisibility(bEnabled);
    RoofSpotlight->SetVisibility(bEnabled);
    
    // Play sound effect
    if (bEnabled)
    {
        UGameplayStatics::PlaySoundAtLocation(this, nullptr, GetActorLocation(), 1.0f);
    }
}

void ASUVVehicle::ToggleOffroadMode(bool bEnabled)
{
    bOffroadModeEnabled = bEnabled;
    
    UChaosWheeledVehicleMovementComponent* SUVMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement);
    if (SUVMovement)
    {
        if (bEnabled)
        {
            // Enhance traction for off-road
            for (int32 WheelIdx = 0; WheelIdx < SUVMovement->WheelSetups.Num(); WheelIdx++)
            {
                // Adjust tire friction for off-road
                SUVMovement->WheelSetups[WheelIdx].TireConfig->TireFriction = 3.0f;
            }
            
            // Adjust suspension for more off-road travel
            SUVMovement->SuspensionForceOffset = 8.0f;
            SUVMovement->SuspensionMaxRaise = 15.0f;
            SUVMovement->SuspensionMaxDrop = 15.0f;
        }
        else
        {
            // Return to normal road settings
            for (int32 WheelIdx = 0; WheelIdx < SUVMovement->WheelSetups.Num(); WheelIdx++)
            {
                // Standard tire friction
                SUVMovement->WheelSetups[WheelIdx].TireConfig->TireFriction = 2.0f;
            }
            
            // Standard suspension
            SUVMovement->SuspensionForceOffset = 4.0f;
            SUVMovement->SuspensionMaxRaise = 10.0f;
            SUVMovement->SuspensionMaxDrop = 10.0f;
        }
    }
    
    // Play mode change sound effect
    UGameplayStatics::PlaySoundAtLocation(this, nullptr, GetActorLocation(), 1.0f);
}

void ASUVVehicle::SetRoofRack(UStaticMesh* NewRoofRackMesh)
{
    if (RoofRackMesh && NewRoofRackMesh)
    {
        RoofRackMesh->SetStaticMesh(NewRoofRackMesh);
        RoofRackMesh->SetVisibility(true);
    }
    else if (RoofRackMesh)
    {
        // No mesh provided, hide the roof rack
        RoofRackMesh->SetVisibility(false);
    }
}

void ASUVVehicle::SetBullBar(UStaticMesh* NewBullBarMesh)
{
    if (BullBarMesh && NewBullBarMesh)
    {
        BullBarMesh->SetStaticMesh(NewBullBarMesh);
        BullBarMesh->SetVisibility(true);
    }
    else if (BullBarMesh)
    {
        // No mesh provided, hide the bull bar
        BullBarMesh->SetVisibility(false);
    }
}

void ASUVVehicle::ApplyThrottle(float Value)
{
    // Modify throttle response based on terrain and offroad mode
    float ModifiedThrottle = Value;
    
    if (CurrentTerrainType == TEXT("Dirt") || 
        CurrentTerrainType == TEXT("Grass") || 
        CurrentTerrainType == TEXT("Sand"))
    {
        // Off-road surfaces
        if (bOffroadModeEnabled)
        {
            // Better response in off-road mode
            ModifiedThrottle = Value * OffroadTractionMultiplier;
        }
        else
        {
            // Reduced response on off-road without off-road mode
            ModifiedThrottle = Value * 0.8f;
        }
    }
    else if (CurrentTerrainType == TEXT("Snow") || 
             CurrentTerrainType == TEXT("Ice"))
    {
        // Slippery surfaces - reduce throttle to prevent wheel spin
        ModifiedThrottle = Value * 0.6f;
    }
    else if (CurrentTerrainType == TEXT("Water"))
    {
        // Water crossing - maintain momentum but don't allow too much power
        ModifiedThrottle = FMath::Clamp(Value, -0.5f, 0.5f);
    }
    
    // Apply modified throttle
    Super::ApplyThrottle(ModifiedThrottle);
}

void ASUVVehicle::ApplySteering(float Value)
{
    // Modify steering response based on terrain and offroad mode
    float ModifiedSteering = Value;
    
    if (CurrentTerrainType == TEXT("Dirt") || 
        CurrentTerrainType == TEXT("Grass") || 
        CurrentTerrainType == TEXT("Sand"))
    {
        // Off-road surfaces
        if (bOffroadModeEnabled)
        {
            // More controlled steering in off-road mode
            ModifiedSteering = Value * 0.85f;
        }
    }
    else if (CurrentTerrainType == TEXT("Snow") || 
             CurrentTerrainType == TEXT("Ice"))
    {
        // Slippery surfaces - reduce steering sensitivity
        ModifiedSteering = Value * 0.7f;
    }
    
    // Apply modified steering
    Super::ApplySteering(ModifiedSteering);
}

void ASUVVehicle::UpdateTerrainDetection()
{
    // Use line trace to detect terrain type below the vehicle
    FHitResult HitResult;
    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0, 0, 200);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, 
        Start, 
        End, 
        ECC_Visibility, 
        QueryParams
    );
    
    if (bHit)
    {
        // Check the physical material to determine surface type
        UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.Get();
        if (PhysMat)
        {
            // In a full implementation, we'd check the physical material
            // properties to determine the surface type
            // For now, we'll use the actor's tag as a placeholder
            AActor* HitActor = HitResult.GetActor();
            if (HitActor && HitActor->Tags.Num() > 0)
            {
                CurrentTerrainType = HitActor->Tags[0];
            }
            else
            {
                CurrentTerrainType = TEXT("Road"); // Default to road
            }
        }
    }
}