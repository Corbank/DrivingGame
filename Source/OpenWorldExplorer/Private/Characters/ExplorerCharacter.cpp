#include "Characters/ExplorerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Vehicles/BaseVehicle.h"

AExplorerCharacter::AExplorerCharacter()
{
    // Set this character to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
    // Don't rotate when the controller rotates. Let the controller only affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // Rotation rate
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Initialize member variables
    bIsFirstPersonView = false;
    CurrentVehicle = nullptr;
}

void AExplorerCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->ClearMappingContext(DefaultMappingContext);
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void AExplorerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AExplorerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Set up Enhanced Input bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AExplorerCharacter::Move);

        // Looking
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AExplorerCharacter::Look);

        // Interacting
        EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AExplorerCharacter::Interact);

        // Toggle Camera
        EnhancedInputComponent->BindAction(CameraToggleAction, ETriggerEvent::Triggered, this, &AExplorerCharacter::ToggleCameraView);
    }
}

void AExplorerCharacter::Move(const FInputActionValue& Value)
{
    // ignore input if we're in a vehicle
    if (CurrentVehicle)
        return;

    // input is a Vector2D
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // get forward vector
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
        // get right vector 
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // add movement 
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AExplorerCharacter::Look(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void AExplorerCharacter::Interact()
{
    // If in a vehicle, exit it
    if (CurrentVehicle)
    {
        ExitVehicle();
        return;
    }

    // Line trace to find interactable objects
    FHitResult HitResult;
    FVector Start = FollowCamera->GetComponentLocation();
    FVector End = Start + FollowCamera->GetForwardVector() * 500.f;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

    if (HitResult.bBlockingHit)
    {
        // If we hit a vehicle, enter it
        ABaseVehicle* Vehicle = Cast<ABaseVehicle>(HitResult.GetActor());
        if (Vehicle)
        {
            EnterVehicle(Vehicle);
        }
        // Other interactable objects can be handled here
    }
}

void AExplorerCharacter::ToggleCameraView()
{
    bIsFirstPersonView = !bIsFirstPersonView;

    if (bIsFirstPersonView)
    {
        // Switch to first person view
        CameraBoom->TargetArmLength = 0.0f;
        CameraBoom->SetRelativeLocation(FVector(0, 0, 50.0f)); // Position at head level
        CameraBoom->bUsePawnControlRotation = true;
    }
    else
    {
        // Switch to third person view
        CameraBoom->TargetArmLength = 400.0f;
        CameraBoom->SetRelativeLocation(FVector(0, 0, 0));
        CameraBoom->bUsePawnControlRotation = true;
    }
}

void AExplorerCharacter::EnterVehicle(ABaseVehicle* Vehicle)
{
    if (!Vehicle)
        return;

    CurrentVehicle = Vehicle;

    // Disable character movement and collision
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Attach character to vehicle
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    AttachToComponent(Vehicle->GetRootComponent(), AttachRules, NAME_None);

    // Set vehicle as the possessed pawn
    AController* CharacterController = GetController();
    if (CharacterController)
    {
        CharacterController->UnPossess();
        CharacterController->Possess(Vehicle);
    }

    // Hide character mesh
    GetMesh()->SetVisibility(false);
}

void AExplorerCharacter::ExitVehicle()
{
    if (!CurrentVehicle)
        return;

    // Get exit location
    FVector ExitLocation = CurrentVehicle->GetActorLocation() + CurrentVehicle->GetActorRightVector() * 200.0f;
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(CurrentVehicle);
    QueryParams.AddIgnoredActor(this);

    // Find a suitable exit point
    GetWorld()->LineTraceSingleByChannel(HitResult, ExitLocation + FVector(0, 0, 200.0f), ExitLocation - FVector(0, 0, 200.0f), ECC_Visibility, QueryParams);
    if (HitResult.bBlockingHit)
    {
        ExitLocation = HitResult.Location + FVector(0, 0, 100.0f);
    }

    // Detach from vehicle
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // Enable character movement and collision
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Show character mesh
    GetMesh()->SetVisibility(true);

    // Set location
    SetActorLocation(ExitLocation);

    // Repossess character
    AController* VehicleController = CurrentVehicle->GetController();
    if (VehicleController)
    {
        VehicleController->UnPossess();
        VehicleController->Possess(this);
    }

    CurrentVehicle = nullptr;
}

void AExplorerCharacter::SetCharacterAppearance(USkeletalMesh* HeadMesh, USkeletalMesh* BodyMesh)
{
    // This would be implemented to change different character component meshes
    // For simplicity, we're just updating the main mesh here
    if (BodyMesh)
    {
        GetMesh()->SetSkeletalMesh(BodyMesh);
    }
}

void AExplorerCharacter::SetCharacterOutfit(UMaterialInstance* OutfitMaterial)
{
    if (OutfitMaterial)
    {
        GetMesh()->SetMaterial(0, OutfitMaterial);
    }
}