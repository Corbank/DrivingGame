#include "World/PhotographySystem.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "World/WorldManager.h"
#include "Vehicles/BaseVehicle.h"
#include "World/ProgressionSystem.h"
#include "HighResScreenshot.h"
#include "Engine/GameViewportClient.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

UPhotographySystem::UPhotographySystem()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Default values
    DefaultFOV = 70.0f;
    MinFOV = 15.0f;  // Telephoto/zoom
    MaxFOV = 110.0f; // Wide angle
    PhotoResolution = FIntPoint(1920, 1080);
    CurrentFilter = EPhotoFilter::None;
    bInPhotoMode = false;
    bUIVisible = true;
}

void UPhotographySystem::BeginPlay()
{
    Super::BeginPlay();

    // Find the player camera that we'll use for taking photos
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && PlayerController->PlayerCameraManager)
    {
        // We'll use the actual player camera when entering photo mode
        // rather than creating a new camera
    }
    
    // Create a post process component for photo filters
    PhotoEffects = NewObject<UPostProcessComponent>(GetOwner());
    PhotoEffects->bEnabled = false;
    PhotoEffects->RegisterComponent();
    
    // Set it to unbound so it affects the entire scene when active
    PhotoEffects->bUnbound = true;
    
    // Load the filter materials here if needed
    // FilterMaterials can be set up in Blueprint editor
}

void UPhotographySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bInPhotoMode)
    {
        // Handle any continuous photo mode functionality
        // For example, we could update real-time preview effects
        
        // Check for input here if not handled through input bindings
    }
}

void UPhotographySystem::EnterPhotoMode()
{
    if (bInPhotoMode)
        return;
    
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController)
        return;
    
    // Store original game state
    OriginalGameTimeDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());
    OriginalHUDVisible = PlayerController->GetHUD() ? PlayerController->GetHUD()->bShowHUD : false;
    
    // Store camera transform
    APawn* ControlledPawn = PlayerController->GetPawn();
    UCameraComponent* PlayerCamera = nullptr;
    
    if (ControlledPawn)
    {
        PlayerCamera = ControlledPawn->FindComponentByClass<UCameraComponent>();
        if (PlayerCamera)
        {
            OriginalCameraTransform = PlayerCamera->GetComponentTransform();
        }
    }
    
    // Enable photo mode
    bInPhotoMode = true;
    
    // Slow down game time to make it easier to capture perfect shots
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1f);
    
    // Hide regular HUD
    if (PlayerController->GetHUD())
    {
        PlayerController->GetHUD()->bShowHUD = false;
    }
    
    // Enable post processing for photo effects
    if (PhotoEffects)
    {
        PhotoEffects->bEnabled = true;
        ApplyCurrentFilter();
    }
    
    // Show photography UI (viewfinder)
    if (ViewfinderWidgetClass)
    {
        ViewfinderWidget = CreateWidget<UUserWidget>(PlayerController, ViewfinderWidgetClass);
        if (ViewfinderWidget)
        {
            ViewfinderWidget->AddToViewport();
            bUIVisible = true;
        }
    }
    
    // Set input mode that allows both UI and game
    FInputModeGameAndUI InputMode;
    if (ViewfinderWidget)
    {
        InputMode.SetWidgetToFocus(ViewfinderWidget->TakeWidget());
    }
    InputMode.SetHideCursorDuringCapture(false);
    PlayerController->SetInputMode(InputMode);
    
    // Make cursor visible
    PlayerController->bShowMouseCursor = true;
}

void UPhotographySystem::ExitPhotoMode()
{
    if (!bInPhotoMode)
        return;
    
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController)
        return;
    
    // Restore original game state
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), OriginalGameTimeDilation);
    
    if (PlayerController->GetHUD())
    {
        PlayerController->GetHUD()->bShowHUD = OriginalHUDVisible;
    }
    
    // Restore camera if needed
    APawn* ControlledPawn = PlayerController->GetPawn();
    UCameraComponent* PlayerCamera = nullptr;
    
    if (ControlledPawn)
    {
        PlayerCamera = ControlledPawn->FindComponentByClass<UCameraComponent>();
        if (PlayerCamera)
        {
            // We might want to smoothly transition back to normal camera
            // For now, just reset FOV
            PlayerCamera->SetFieldOfView(DefaultFOV);
        }
    }
    
    // Disable photo post processing
    if (PhotoEffects)
    {
        PhotoEffects->bEnabled = false;
    }
    
    // Remove photography UI
    if (ViewfinderWidget)
    {
        ViewfinderWidget->RemoveFromParent();
        ViewfinderWidget = nullptr;
    }
    
    // Reset input mode to game only
    FInputModeGameOnly InputMode;
    PlayerController->SetInputMode(InputMode);
    
    // Hide cursor
    PlayerController->bShowMouseCursor = false;
    
    // Turn off photo mode
    bInPhotoMode = false;
}

void UPhotographySystem::TakePhoto()
{
    if (!bInPhotoMode)
        return;
    
    // Hide UI for the photo if it's currently visible
    bool bWasUIVisible = bUIVisible;
    if (bWasUIVisible && ViewfinderWidget)
    {
        ViewfinderWidget->SetVisibility(ESlateVisibility::Hidden);
        bUIVisible = false;
    }
    
    // Play camera shutter sound
    UGameplayStatics::PlaySound2D(GetWorld(), nullptr); // Add your camera sound effect here
    
    // Take a screenshot
    CaptureScreenshot();
    
    // Generate photo metadata
    FPhotoMetadata Metadata = GeneratePhotoMetadata();
    
    // Register the photo with progression system
    UProgressionSystem* ProgressionSystem = Cast<UProgressionSystem>(UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UProgressionSystem>());
    if (ProgressionSystem && !Metadata.LocationName.IsEmpty())
    {
        ProgressionSystem->RegisterLocationPhotographed(Metadata.LocationName);
    }
    
    // Show UI again if it was visible
    if (bWasUIVisible && ViewfinderWidget)
    {
        ViewfinderWidget->SetVisibility(ESlateVisibility::Visible);
        bUIVisible = true;
    }
}

void UPhotographySystem::SetFilter(EPhotoFilter NewFilter)
{
    if (CurrentFilter == NewFilter)
        return;
    
    CurrentFilter = NewFilter;
    
    if (bInPhotoMode)
    {
        ApplyCurrentFilter();
    }
}

void UPhotographySystem::CycleFilterForward()
{
    // Get the next filter in the enum
    int32 NextFilterIdx = static_cast<int32>(CurrentFilter) + 1;
    
    // Wrap around if we exceed the enum range
    if (NextFilterIdx >= static_cast<int32>(EPhotoFilter::Vibrant) + 1)
    {
        NextFilterIdx = 0; // Reset to None
    }
    
    SetFilter(static_cast<EPhotoFilter>(NextFilterIdx));
}

void UPhotographySystem::CycleFilterBackward()
{
    // Get the previous filter in the enum
    int32 PrevFilterIdx = static_cast<int32>(CurrentFilter) - 1;
    
    // Wrap around if we go below zero
    if (PrevFilterIdx < 0)
    {
        PrevFilterIdx = static_cast<int32>(EPhotoFilter::Vibrant);
    }
    
    SetFilter(static_cast<EPhotoFilter>(PrevFilterIdx));
}

void UPhotographySystem::AdjustZoom(float ZoomAmount)
{
    if (!bInPhotoMode)
        return;
    
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController)
        return;
    
    APawn* ControlledPawn = PlayerController->GetPawn();
    UCameraComponent* PlayerCamera = nullptr;
    
    if (ControlledPawn)
    {
        PlayerCamera = ControlledPawn->FindComponentByClass<UCameraComponent>();
        if (PlayerCamera)
        {
            // Adjust FOV (lower FOV = more zoom)
            float CurrentFOV = PlayerCamera->FieldOfView;
            float NewFOV = FMath::Clamp(CurrentFOV - ZoomAmount, MinFOV, MaxFOV);
            PlayerCamera->SetFieldOfView(NewFOV);
        }
    }
}

void UPhotographySystem::ToggleUI()
{
    if (!bInPhotoMode || !ViewfinderWidget)
        return;
    
    bUIVisible = !bUIVisible;
    
    if (bUIVisible)
    {
        ViewfinderWidget->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        ViewfinderWidget->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPhotographySystem::ApplyCurrentFilter()
{
    if (!PhotoEffects)
        return;
    
    UMaterialInterface* FilterMaterial = nullptr;
    
    // Get the material for the current filter
    if (FilterMaterials.Contains(CurrentFilter))
    {
        FilterMaterial = FilterMaterials[CurrentFilter];
    }
    
    // If we have a valid material, apply it as a post process effect
    if (FilterMaterial)
    {
        // Create a dynamic instance of the material
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(FilterMaterial, this);
        
        // Apply the material to the post process component
        PhotoEffects->Settings.WeightedBlendables.Array.Empty();
        
        FWeightedBlendable Blendable;
        Blendable.Object = DynamicMaterial;
        Blendable.Weight = 1.0f;
        
        PhotoEffects->Settings.WeightedBlendables.Array.Add(Blendable);
    }
    else
    {
        // No filter, clear any post process effects
        PhotoEffects->Settings.WeightedBlendables.Array.Empty();
    }
}

void UPhotographySystem::CaptureScreenshot()
{
    // Ensure screenshot directory exists
    FString ScreenshotDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Screenshots"));
    IFileManager::Get().MakeDirectory(*ScreenshotDir, true);
    
    // Generate a unique filename with timestamp
    FDateTime Now = FDateTime::Now();
    FString Timestamp = Now.ToString(TEXT("%Y%m%d_%H%M%S"));
    FString FileName = FString::Printf(TEXT("OpenWorldExplorer_Photo_%s.png"), *Timestamp);
    FString FilePath = ScreenshotDir / FileName;
    
    // Setup high-resolution screenshot parameters
    GetHighResScreenshotConfig().FilenameOverride = FilePath;
    GetHighResScreenshotConfig().SetResolution(PhotoResolution.X, PhotoResolution.Y);
    GetHighResScreenshotConfig().bMaskEnabled = false;
    
    // Take the screenshot
    FScreenshotRequest::RequestScreenshot(FilePath, false, false);
    
    // Log the capture for debugging
    UE_LOG(LogTemp, Log, TEXT("Photo captured to: %s"), *FilePath);
}

FPhotoMetadata UPhotographySystem::GeneratePhotoMetadata()
{
    FPhotoMetadata Metadata;
    
    // Get player location
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController && PlayerController->GetPawn())
    {
        Metadata.Location = PlayerController->GetPawn()->GetActorLocation();
    }
    
    // Set current time
    Metadata.Timestamp = FDateTime::Now();
    
    // Get weather condition
    Metadata.WeatherCondition = GetCurrentWeatherCondition();
    
    // Get time of day from world manager
    AWorldManager* WorldManager = Cast<AWorldManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWorldManager::StaticClass()));
    if (WorldManager)
    {
        Metadata.TimeOfDay = WorldManager->GetTimeOfDay();
    }
    
    // Set applied filter
    Metadata.AppliedFilter = CurrentFilter;
    
    // Detect location name
    Metadata.LocationName = DetectNearbyLocationName();
    
    // Detect vehicles in frame
    Metadata.CapturedVehicles = DetectVehiclesInFrame();
    
    return Metadata;
}

FString UPhotographySystem::DetectNearbyLocationName()
{
    FString LocationName;
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    
    if (PlayerController && PlayerController->GetPawn())
    {
        FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
        
        // Get progression system to check discovered locations
        UProgressionSystem* ProgressionSystem = Cast<UProgressionSystem>(UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UProgressionSystem>());
        if (ProgressionSystem)
        {
            // Get all discovered locations
            TArray<FDiscoveredLocation> Locations = ProgressionSystem->GetDiscoveredLocations();
            
            // Find the closest one within a certain range
            float ClosestDistanceSq = 1000000.0f * 1000000.0f; // 1000m squared
            for (const FDiscoveredLocation& Location : Locations)
            {
                float DistanceSq = FVector::DistSquared(PlayerLocation, Location.LocationCoordinates);
                
                if (DistanceSq < ClosestDistanceSq)
                {
                    ClosestDistanceSq = DistanceSq;
                    LocationName = Location.LocationName;
                }
            }
            
            // Only return the location if we're within a reasonable distance (500m)
            if (ClosestDistanceSq > 500.0f * 500.0f)
            {
                LocationName.Empty();
            }
        }
    }
    
    return LocationName;
}

FString UPhotographySystem::GetCurrentWeatherCondition()
{
    FString WeatherCondition = TEXT("Clear");
    
    // Get the world manager to check current weather
    AWorldManager* WorldManager = Cast<AWorldManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AWorldManager::StaticClass()));
    if (WorldManager)
    {
        // Convert enum to string
        EWeatherType CurrentWeather = WorldManager->GetCurrentWeather();
        switch (CurrentWeather)
        {
            case EWeatherType::Clear:
                WeatherCondition = TEXT("Clear");
                break;
            case EWeatherType::Cloudy:
                WeatherCondition = TEXT("Cloudy");
                break;
            case EWeatherType::Rain:
                WeatherCondition = TEXT("Rainy");
                break;
            case EWeatherType::Storm:
                WeatherCondition = TEXT("Stormy");
                break;
            case EWeatherType::Fog:
                WeatherCondition = TEXT("Foggy");
                break;
            case EWeatherType::Snow:
                WeatherCondition = TEXT("Snowy");
                break;
            default:
                WeatherCondition = TEXT("Clear");
                break;
        }
    }
    
    return WeatherCondition;
}

TArray<FString> UPhotographySystem::DetectVehiclesInFrame()
{
    TArray<FString> DetectedVehicles;
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    
    if (!PlayerController)
        return DetectedVehicles;
    
    // Get player camera
    UCameraComponent* PlayerCamera = nullptr;
    if (PlayerController->GetPawn())
    {
        PlayerCamera = PlayerController->GetPawn()->FindComponentByClass<UCameraComponent>();
    }
    
    if (!PlayerCamera)
        return DetectedVehicles;
    
    // Get all vehicle actors
    TArray<AActor*> Vehicles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseVehicle::StaticClass(), Vehicles);
    
    // Check which ones are visible from the camera
    for (AActor* Vehicle : Vehicles)
    {
        ABaseVehicle* BaseVehicle = Cast<ABaseVehicle>(Vehicle);
        if (!BaseVehicle)
            continue;
        
        FVector CameraLocation = PlayerCamera->GetComponentLocation();
        FRotator CameraRotation = PlayerCamera->GetComponentRotation();
        
        // Check if vehicle is in front of the camera
        FVector DirectionToVehicle = Vehicle->GetActorLocation() - CameraLocation;
        DirectionToVehicle.Normalize();
        
        FVector CameraForward = CameraRotation.Vector();
        
        float DotProduct = FVector::DotProduct(CameraForward, DirectionToVehicle);
        
        // If dot product is positive, the vehicle is in front of the camera
        if (DotProduct > 0.0f)
        {
            // Check if vehicle is within camera FOV
            float AngleToDegrees = FMath::Acos(DotProduct) * 180.0f / PI;
            
            if (AngleToDegrees < PlayerCamera->FieldOfView * 0.5f)
            {
                // Check line of sight
                FHitResult HitResult;
                FCollisionQueryParams QueryParams;
                QueryParams.AddIgnoredActor(PlayerController->GetPawn());
                
                if (!GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, Vehicle->GetActorLocation(), 
                                                         ECC_Visibility, QueryParams) ||
                    HitResult.GetActor() == Vehicle)
                {
                    // Vehicle is visible, add its name
                    DetectedVehicles.Add(Vehicle->GetName());
                }
            }
        }
    }
    
    return DetectedVehicles;
}