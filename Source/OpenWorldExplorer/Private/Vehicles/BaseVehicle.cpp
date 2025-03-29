#include "Vehicles/BaseVehicle.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Characters/ExplorerCharacter.h"
#include "GameFramework/PlayerController.h"
#include "World/ProgressionSystem.h"
#include "Kismet/GameplayStatics.h"

ABaseVehicle::ABaseVehicle()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create the mesh component
    VehicleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VehicleMesh"));
    RootComponent = VehicleMesh;
    VehicleMesh->SetCollisionProfileName(TEXT("Vehicle"));

    // Create the vehicle movement component
    VehicleMovement = CreateDefaultSubobject<UChaosWheeledVehicleMovementComponent>(TEXT("VehicleMovement"));
    VehicleMovement->SetIsReplicated(true); // Support multiplayer if needed
    VehicleMovement->UpdatedComponent = VehicleMesh;

    // Create a spring arm component for the camera
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(VehicleMesh);
    CameraBoom->TargetArmLength = 600.0f;
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritYaw = true;
    CameraBoom->bInheritRoll = false;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));

    // Create a camera and attach it to the spring arm
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // Set default properties
    MaxSpeed = 200.0f; // km/h
    Acceleration = 10.0f;
    BrakingForce = 10.0f;
    TurnRate = 5.0f;
    bIsFirstPersonView = false;
    
    // Set this pawn to be controlled by the player
    AutoPossessPlayer = EAutoReceiveInput::Player0;
    
    // Default setup for vehicle simulation
    UChaosWheeledVehicleMovementComponent* WheeledMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement);
    if (WheeledMovement)
    {
        // Set up the chassis
        WheeledMovement->ChassisHeight = 100.0f;
        WheeledMovement->DragCoefficient = 0.3f;
        
        // Set up the engine
        WheeledMovement->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
        WheeledMovement->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 400.0f);
        WheeledMovement->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(2000.0f, 500.0f);
        WheeledMovement->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(4000.0f, 600.0f);
        WheeledMovement->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(6000.0f, 500.0f);
        WheeledMovement->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(8000.0f, 400.0f);
        WheeledMovement->EngineSetup.MaxRPM = 8000.0f;
        
        // Set up transmission
        WheeledMovement->TransmissionSetup.GearSwitchTime = 0.15f;
        WheeledMovement->TransmissionSetup.GearAutoBoxLatency = 1.0f;
        WheeledMovement->TransmissionSetup.FinalRatio = 3.5f;
        
        // Forward gears
        WheeledMovement->TransmissionSetup.ForwardGears.SetNum(6);
        WheeledMovement->TransmissionSetup.ForwardGears[0].Ratio = 4.25f;
        WheeledMovement->TransmissionSetup.ForwardGears[1].Ratio = 2.52f;
        WheeledMovement->TransmissionSetup.ForwardGears[2].Ratio = 1.66f;
        WheeledMovement->TransmissionSetup.ForwardGears[3].Ratio = 1.22f;
        WheeledMovement->TransmissionSetup.ForwardGears[4].Ratio = 1.0f;
        WheeledMovement->TransmissionSetup.ForwardGears[5].Ratio = 0.82f;
        
        // Set up the steering
        WheeledMovement->SteeringSetup.SteeringCurve.GetRichCurve()->Reset();
        WheeledMovement->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
        WheeledMovement->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(100.0f, 0.8f);
        WheeledMovement->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(200.0f, 0.4f);
    }
}

void ABaseVehicle::BeginPlay()
{
    Super::BeginPlay();
    
    // Set up Enhanced Input for the player controller
    if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->ClearMappingContext(VehicleMappingContext);
            Subsystem->AddMappingContext(VehicleMappingContext, 0);
        }
    }
}

void ABaseVehicle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Track distance traveled for progression system
    static FVector LastLocation = GetActorLocation();
    float DistanceTraveled = FVector::Distance(GetActorLocation(), LastLocation) / 100.0f; // Convert to meters
    
    // Update progression system with distance traveled
    if (DistanceTraveled > 0.01f) // Only register meaningful movement
    {
        UProgressionSystem* ProgressionSystem = Cast<UProgressionSystem>(UGameplayStatics::GetGameInstance(this)->GetSubsystem<UProgressionSystem>());
        if (ProgressionSystem)
        {
            ProgressionSystem->RegisterDistanceTraveled(DistanceTraveled, true);
        }
        
        LastLocation = GetActorLocation();
    }
}

void ABaseVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Set up Enhanced Input component
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (ThrottleAction)
        {
            EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ABaseVehicle::ProcessThrottleInput);
            EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ABaseVehicle::ProcessThrottleInput);
        }
        
        if (SteeringAction)
        {
            EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ABaseVehicle::ProcessSteeringInput);
            EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ABaseVehicle::ProcessSteeringInput);
        }
        
        if (BrakeAction)
        {
            EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ABaseVehicle::ProcessBrakeInput);
            EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ABaseVehicle::ProcessBrakeInput);
        }
        
        if (HandbrakeAction)
        {
            EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Triggered, this, &ABaseVehicle::ProcessHandbrakeInput);
            EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ABaseVehicle::ProcessHandbrakeInput);
        }
        
        if (CameraToggleAction)
        {
            EnhancedInputComponent->BindAction(CameraToggleAction, ETriggerEvent::Triggered, this, &ABaseVehicle::ToggleCameraView);
        }
        
        if (ExitVehicleAction)
        {
            EnhancedInputComponent->BindAction(ExitVehicleAction, ETriggerEvent::Triggered, this, &ABaseVehicle::ExitVehicle);
        }
    }
}

void ABaseVehicle::ProcessThrottleInput(const FInputActionValue& Value)
{
    // Get value from input (1.0 = forward, -1.0 = reverse)
    float ThrottleValue = Value.Get<float>();
    ApplyThrottle(ThrottleValue);
}

void ABaseVehicle::ProcessSteeringInput(const FInputActionValue& Value)
{
    // Get value from input (1.0 = right, -1.0 = left)
    float SteeringValue = Value.Get<float>();
    ApplySteering(SteeringValue);
}

void ABaseVehicle::ProcessBrakeInput(const FInputActionValue& Value)
{
    // Get value from input (1.0 = full brake, 0.0 = no brake)
    float BrakeValue = Value.Get<float>();
    ApplyBrake(BrakeValue);
}

void ABaseVehicle::ProcessHandbrakeInput(const FInputActionValue& Value)
{
    // Get value from input (1.0 = handbrake on, 0.0 = handbrake off)
    bool bHandbrakeOn = Value.Get<bool>();
    ApplyHandbrake(bHandbrakeOn);
}

void ABaseVehicle::ApplyThrottle(float Value)
{
    if (VehicleMovement)
    {
        UChaosWheeledVehicleMovementComponent* WheeledMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement);
        if (WheeledMovement)
        {
            WheeledMovement->SetThrottleInput(Value);
        }
    }
}

void ABaseVehicle::ApplySteering(float Value)
{
    if (VehicleMovement)
    {
        UChaosWheeledVehicleMovementComponent* WheeledMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement);
        if (WheeledMovement)
        {
            WheeledMovement->SetSteeringInput(Value);
        }
    }
}

void ABaseVehicle::ApplyBrake(float Value)
{
    if (VehicleMovement)
    {
        UChaosWheeledVehicleMovementComponent* WheeledMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement);
        if (WheeledMovement)
        {
            WheeledMovement->SetBrakeInput(Value);
        }
    }
}

void ABaseVehicle::ApplyHandbrake(bool bEnabled)
{
    if (VehicleMovement)
    {
        UChaosWheeledVehicleMovementComponent* WheeledMovement = Cast<UChaosWheeledVehicleMovementComponent>(VehicleMovement);
        if (WheeledMovement)
        {
            WheeledMovement->SetHandbrakeInput(bEnabled);
        }
    }
}

void ABaseVehicle::ToggleCameraView()
{
    bIsFirstPersonView = !bIsFirstPersonView;
    
    if (bIsFirstPersonView)
    {
        // Switch to first-person view
        CameraBoom->TargetArmLength = 0.0f;
        CameraBoom->SetRelativeLocation(FVector(20.0f, 0.0f, 150.0f)); // Position for driver's perspective
        CameraBoom->bUsePawnControlRotation = true;
    }
    else
    {
        // Switch to third-person view
        CameraBoom->TargetArmLength = 600.0f;
        CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
        CameraBoom->bUsePawnControlRotation = false;
    }
}

void ABaseVehicle::ExitVehicle()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (!PlayerController)
    {
        return;
    }
    
    // Create and possess a new character
    FVector SpawnLocation = GetActorLocation() + GetActorRightVector() * 200.0f;
    FRotator SpawnRotation = GetActorRotation();
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AExplorerCharacter* Character = GetWorld()->SpawnActor<AExplorerCharacter>(AExplorerCharacter::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
    if (Character)
    {
        // Unpossess the vehicle and possess the character
        PlayerController->UnPossess();
        PlayerController->Possess(Character);
    }
}

void ABaseVehicle::SetVehicleColor(const FLinearColor& Color)
{
    if (VehicleMesh)
    {
        UMaterialInterface* Material = VehicleMesh->GetMaterial(0);
        if (Material)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
            DynamicMaterial->SetVectorParameterValue(TEXT("BodyColor"), Color);
            VehicleMesh->SetMaterial(0, DynamicMaterial);
        }
    }
}

void ABaseVehicle::AddVehicleAccessory(USceneComponent* AccessoryComponent, FName SocketName)
{
    if (AccessoryComponent && VehicleMesh && VehicleMesh->DoesSocketExist(SocketName))
    {
        AccessoryComponent->AttachToComponent(VehicleMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
        AccessoryComponent->RegisterComponent();
    }
    else if (AccessoryComponent)
    {
        // If the socket doesn't exist, just attach to the vehicle mesh
        AccessoryComponent->AttachToComponent(VehicleMesh, FAttachmentTransformRules::SnapToTargetIncludingScale);
        AccessoryComponent->RegisterComponent();
    }
}