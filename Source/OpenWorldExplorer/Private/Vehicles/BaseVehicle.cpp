#include "Vehicles/BaseVehicle.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ABaseVehicle::ABaseVehicle()
{
    // Set this pawn to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create the vehicle mesh
    VehicleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VehicleMesh"));
    RootComponent = VehicleMesh;

    // Create the Chaos wheeled vehicle movement component
    VehicleMovement = CreateDefaultSubobject<UChaosWheeledVehicleMovementComponent>(TEXT("VehicleMovement"));
    VehicleMovement->SetupAttachment(VehicleMesh);
    VehicleMovement->UpdatedComponent = VehicleMesh;

    // Create a spring arm for the camera
    CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
    CameraSpringArm->SetupAttachment(VehicleMesh);
    CameraSpringArm->TargetArmLength = 600.0f;
    CameraSpringArm->bUsePawnControlRotation = false;
    CameraSpringArm->bInheritPitch = false;
    CameraSpringArm->bInheritYaw = true;
    CameraSpringArm->bInheritRoll = false;
    CameraSpringArm->bEnableCameraLag = true;
    CameraSpringArm->bEnableCameraRotationLag = true;
    CameraSpringArm->CameraLagSpeed = 5.0f;
    CameraSpringArm->CameraRotationLagSpeed = 5.0f;
    CameraSpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));

    // Create the camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;
}

void ABaseVehicle::BeginPlay()
{
    Super::BeginPlay();
}

void ABaseVehicle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABaseVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Set up with enhanced input system when we implement input mapping
    UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
    if (EnhancedInputComponent)
    {
        // Input bindings will be set up here
    }
}

void ABaseVehicle::ApplyThrottle(float Value)
{
    if (VehicleMovement)
    {
        VehicleMovement->SetThrottleInput(Value);
    }
}

void ABaseVehicle::ApplySteering(float Value)
{
    if (VehicleMovement)
    {
        VehicleMovement->SetSteeringInput(Value);
    }
}

void ABaseVehicle::ApplyHandbrake()
{
    if (VehicleMovement)
    {
        VehicleMovement->SetHandbrakeInput(true);
    }
}

void ABaseVehicle::ReleaseHandbrake()
{
    if (VehicleMovement)
    {
        VehicleMovement->SetHandbrakeInput(false);
    }
}

void ABaseVehicle::SetVehicleColor(const FLinearColor& Color)
{
    if (VehicleMesh)
    {
        // Create a dynamic material instance and set the color parameter
        UMaterialInterface* Material = VehicleMesh->GetMaterial(0);
        if (Material)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            DynamicMaterial->SetVectorParameterValue(TEXT("BodyColor"), Color);
            VehicleMesh->SetMaterial(0, DynamicMaterial);
        }
    }
}

void ABaseVehicle::SetWheelType(int32 WheelTypeIndex)
{
    // This will be implemented to swap wheel meshes based on the index
    // Will require wheel mesh data assets to be defined
}

void ABaseVehicle::AddVehicleAccessory(UStaticMeshComponent* AccessoryMesh, FName SocketName)
{
    if (VehicleMesh && AccessoryMesh)
    {
        AccessoryMesh->SetupAttachment(VehicleMesh, SocketName);
        AccessoryMesh->RegisterComponent();
    }
}