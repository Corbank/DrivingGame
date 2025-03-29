#include "Vehicles/CarVehicle.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Sound/SoundCue.h"

ACarVehicle::ACarVehicle()
{
    // Create car-specific components
    BodyworkMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyworkMesh"));
    BodyworkMesh->SetupAttachment(VehicleMesh);
    
    SpoilerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpoilerMesh"));
    SpoilerMesh->SetupAttachment(BodyworkMesh);
    
    FrontBumperMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontBumperMesh"));
    FrontBumperMesh->SetupAttachment(BodyworkMesh);
    
    RearBumperMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RearBumperMesh"));
    RearBumperMesh->SetupAttachment(BodyworkMesh);
    
    EngineSound = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSound"));
    EngineSound->SetupAttachment(BodyworkMesh);
    EngineSound->bAutoActivate = false;
    
    // Set default car properties
    HorsePower = 350.0f;
    MaxRPM = 7500.0f;
    TopSpeed = 200.0f; // km/h
}

void ACarVehicle::BeginPlay()
{
    Super::BeginPlay();
    
    // Configure chaos vehicle movement based on car properties
    UChaosWheeledVehicleMovementComponent* CarMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement);
    if (CarMovement)
    {
        // Set engine torque based on horsepower
        // 1 HP = ~0.75 kW, and we need to convert to Nm for the engine curve
        const float TorqueMultiplier = HorsePower * 0.75f;
        CarMovement->EngineSetup.MaxTorque = TorqueMultiplier;
        
        // Set max RPM
        CarMovement->EngineSetup.MaxRPM = MaxRPM;
        
        // Configure transmission based on car performance
        CarMovement->TransmissionSetup.GearAutoBoxLatency = 0.1f;
        CarMovement->TransmissionSetup.FinalRatio = 3.5f;
        
        // Start the engine sound
        if (EngineSound)
        {
            EngineSound->Play();
        }
    }
}

void ACarVehicle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update the engine sound based on current RPM
    UChaosWheeledVehicleMovementComponent* CarMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement);
    if (CarMovement && EngineSound)
    {
        float CurrentRPM = CarMovement->GetEngineRotationSpeed();
        UpdateEngineSound(CurrentRPM);
    }
}

void ACarVehicle::UpdateEngineSound(float CurrentRPM)
{
    if (EngineSound)
    {
        // Calculate RPM ratio (0.0 to 1.0)
        float RPMRatio = FMath::Clamp(CurrentRPM / MaxRPM, 0.0f, 1.0f);
        
        // Adjust pitch based on RPM
        float BasePitch = 0.8f; // Base pitch at idle
        float MaxPitchMultiplier = 3.0f; // Maximum pitch at redline
        
        float CurrentPitch = BasePitch + (MaxPitchMultiplier - BasePitch) * RPMRatio;
        EngineSound->SetPitchMultiplier(CurrentPitch);
        
        // Adjust volume based on RPM
        float MinVolume = 0.3f; // Volume at idle
        float MaxVolume = 1.0f; // Volume at high RPM
        
        float CurrentVolume = MinVolume + (MaxVolume - MinVolume) * RPMRatio;
        EngineSound->SetVolumeMultiplier(CurrentVolume);
    }
}

void ACarVehicle::SetBodywork(UStaticMesh* NewBodyworkMesh)
{
    if (BodyworkMesh && NewBodyworkMesh)
    {
        BodyworkMesh->SetStaticMesh(NewBodyworkMesh);
    }
}

void ACarVehicle::SetSpoiler(UStaticMesh* NewSpoilerMesh)
{
    if (SpoilerMesh)
    {
        if (NewSpoilerMesh)
        {
            SpoilerMesh->SetStaticMesh(NewSpoilerMesh);
            SpoilerMesh->SetVisibility(true);
        }
        else
        {
            // No spoiler selected, hide it
            SpoilerMesh->SetVisibility(false);
        }
    }
}

void ACarVehicle::SetFrontBumper(UStaticMesh* NewFrontBumperMesh)
{
    if (FrontBumperMesh && NewFrontBumperMesh)
    {
        FrontBumperMesh->SetStaticMesh(NewFrontBumperMesh);
    }
}

void ACarVehicle::SetRearBumper(UStaticMesh* NewRearBumperMesh)
{
    if (RearBumperMesh && NewRearBumperMesh)
    {
        RearBumperMesh->SetStaticMesh(NewRearBumperMesh);
    }
}

void ACarVehicle::SetVehicleColor(const FLinearColor& Color)
{
    Super::SetVehicleColor(Color);
    
    // Additionally apply to bodywork if using different materials
    if (BodyworkMesh)
    {
        UMaterialInterface* Material = BodyworkMesh->GetMaterial(0);
        if (Material)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            DynamicMaterial->SetVectorParameterValue(TEXT("BodyColor"), Color);
            BodyworkMesh->SetMaterial(0, DynamicMaterial);
        }
    }
}