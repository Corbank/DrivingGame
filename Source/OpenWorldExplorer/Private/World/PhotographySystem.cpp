#include "World/PhotographySystem.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HighResScreenshot.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Characters/ExplorerCharacter.h"
#include "Engine/Canvas.h"

UPhotographySystem::UPhotographySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsInPhotographyMode = false;
    CurrentFilter = EPhotoFilter::None;
    
    // Default photography settings
    FocalLength = 35.0f; // 35mm lens
    Aperture = 4.0f;     // f/4.0
    ShutterSpeed = 1.0f / 125.0f; // 1/125 sec
}

void UPhotographySystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the player character and its camera
    AExplorerCharacter* PlayerCharacter = Cast<AExplorerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (PlayerCharacter)
    {
        // Find the player's camera component
        TArray<UCameraComponent*> Cameras;
        PlayerCharacter->GetComponents<UCameraComponent>(Cameras);
        
        if (Cameras.Num() > 0)
        {
            PlayerCamera = Cameras[0];
        }
    }
    
    // Create post process component for photo effects
    AActor* Owner = GetOwner();
    if (Owner)
    {
        PhotoPostProcess = NewObject<UPostProcessComponent>(Owner);
        PhotoPostProcess->bEnabled = false;
        PhotoPostProcess->RegisterComponent();
        
        // Set up the post-process effects
        SetupPostProcess();
    }
}

void UPhotographySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsInPhotographyMode)
    {
        // Update camera settings while in photography mode
        UpdateCameraSettings();
    }
}

void UPhotographySystem::SetupPostProcess()
{
    if (PhotoPostProcess)
    {
        // Create a dynamic material instance for post-processing
        static ConstructorHelpers::FObjectFinder<UMaterial> DefaultFilterMaterial(TEXT("/Game/Materials/M_PhotoFilter"));
        if (DefaultFilterMaterial.Succeeded())
        {
            FilterMaterial = UMaterialInstanceDynamic::Create(DefaultFilterMaterial.Object, this);
            
            if (FilterMaterial)
            {
                // Add the material to post process
                FWeightedBlendable Blendable;
                Blendable.Object = FilterMaterial;
                Blendable.Weight = 1.0f;
                
                PhotoPostProcess->Settings.WeightedBlendables.Array.Add(Blendable);
            }
        }
    }
}

void UPhotographySystem::EnterPhotographyMode()
{
    if (!bIsInPhotographyMode)
    {
        bIsInPhotographyMode = true;
        
        // Enable photo post-process effects
        if (PhotoPostProcess)
        {
            PhotoPostProcess->bEnabled = true;
        }
        
        // Show photography UI
        if (PhotographyWidgetClass)
        {
            APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
            if (PlayerController)
            {
                PhotographyWidget = CreateWidget<UUserWidget>(PlayerController, PhotographyWidgetClass);
                if (PhotographyWidget)
                {
                    PhotographyWidget->AddToViewport();
                }
            }
        }
        
        // Adjust time dilation to slow down the game for photo mode
        UGameplayStatics::SetGlobalTimeDilation(this, 0.1f);
    }
}

void UPhotographySystem::ExitPhotographyMode()
{
    if (bIsInPhotographyMode)
    {
        bIsInPhotographyMode = false;
        
        // Disable photo post-process effects
        if (PhotoPostProcess)
        {
            PhotoPostProcess->bEnabled = false;
        }
        
        // Hide photography UI
        if (PhotographyWidget)
        {
            PhotographyWidget->RemoveFromParent();
            PhotographyWidget = nullptr;
        }
        
        // Reset time dilation
        UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
    }
}

void UPhotographySystem::TogglePhotographyMode()
{
    if (bIsInPhotographyMode)
    {
        ExitPhotographyMode();
    }
    else
    {
        EnterPhotographyMode();
    }
}

void UPhotographySystem::TakePhoto()
{
    // Get the current date/time for unique file name
    FDateTime Now = FDateTime::Now();
    FString FileName = FString::Printf(TEXT("OpenWorldExplorer_Photo_%s"), *Now.ToString(TEXT("%Y%m%d_%H%M%S")));
    
    // Capture and save the screenshot
    CaptureScreenshot(FileName);
    
    // Add visual/audio feedback that photo was taken
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController)
    {
        // Feedback that photo was taken (e.g., flash effect)
        FLinearColor OriginalColor = PlayerController->GetHUD()->GetBaseColor();
        PlayerController->GetHUD()->SetBaseColor(FLinearColor::White);
        
        // Play camera shutter sound
        static ConstructorHelpers::FObjectFinder<USoundBase> ShutterSound(TEXT("/Game/Sounds/CameraShutter"));
        if (ShutterSound.Succeeded())
        {
            UGameplayStatics::PlaySound2D(this, ShutterSound.Object);
        }
        
        // Reset HUD color after a short delay
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [PlayerController, OriginalColor]()
        {
            PlayerController->GetHUD()->SetBaseColor(OriginalColor);
        }, 0.1f, false);
    }
}

void UPhotographySystem::ApplyFilter(EPhotoFilter Filter)
{
    CurrentFilter = Filter;
    
    if (FilterMaterial)
    {
        // Apply filter settings to material instance
        switch (Filter)
        {
            case EPhotoFilter::None:
                FilterMaterial->SetScalarParameterValue(TEXT("FilterIntensity"), 0.0f);
                break;
                
            case EPhotoFilter::Vintage:
                FilterMaterial->SetScalarParameterValue(TEXT("FilterIntensity"), 1.0f);
                FilterMaterial->SetVectorParameterValue(TEXT("FilterTint"), FLinearColor(1.0f, 0.9f, 0.7f));
                FilterMaterial->SetScalarParameterValue(TEXT("Contrast"), 1.2f);
                FilterMaterial->SetScalarParameterValue(TEXT("Saturation"), 0.8f);
                break;
                
            case EPhotoFilter::BlackAndWhite:
                FilterMaterial->SetScalarParameterValue(TEXT("FilterIntensity"), 1.0f);
                FilterMaterial->SetScalarParameterValue(TEXT("Saturation"), 0.0f);
                FilterMaterial->SetScalarParameterValue(TEXT("Contrast"), 1.3f);
                break;
                
            case EPhotoFilter::Sepia:
                FilterMaterial->SetScalarParameterValue(TEXT("FilterIntensity"), 1.0f);
                FilterMaterial->SetVectorParameterValue(TEXT("FilterTint"), FLinearColor(1.0f, 0.8f, 0.5f));
                FilterMaterial->SetScalarParameterValue(TEXT("Saturation"), 0.2f);
                break;
                
            case EPhotoFilter::Dramatic:
                FilterMaterial->SetScalarParameterValue(TEXT("FilterIntensity"), 1.0f);
                FilterMaterial->SetScalarParameterValue(TEXT("Contrast"), 1.5f);
                FilterMaterial->SetScalarParameterValue(TEXT("Saturation"), 1.2f);
                break;
                
            case EPhotoFilter::Cinematic:
                FilterMaterial->SetScalarParameterValue(TEXT("FilterIntensity"), 1.0f);
                FilterMaterial->SetVectorParameterValue(TEXT("FilterTint"), FLinearColor(0.9f, 1.0f, 1.1f));
                FilterMaterial->SetScalarParameterValue(TEXT("Contrast"), 1.1f);
                FilterMaterial->SetScalarParameterValue(TEXT("LetterboxRatio"), 0.1f);
                break;
        }
    }
}

void UPhotographySystem::SetFocalLength(float NewFocalLength)
{
    FocalLength = FMath::Clamp(NewFocalLength, 18.0f, 200.0f);
    UpdateCameraSettings();
}

void UPhotographySystem::SetAperture(float NewAperture)
{
    Aperture = FMath::Clamp(NewAperture, 1.4f, 22.0f);
    UpdateCameraSettings();
}

void UPhotographySystem::SetShutterSpeed(float NewShutterSpeed)
{
    ShutterSpeed = FMath::Clamp(NewShutterSpeed, 1.0f/8000.0f, 1.0f);
    UpdateCameraSettings();
}

void UPhotographySystem::UpdateCameraSettings()
{
    if (PlayerCamera)
    {
        // Convert focal length to field of view
        // FOV = 2 * atan(sensorWidth / (2 * focalLength))
        // Using 35mm full frame sensor width of ~36mm
        const float SensorWidth = 36.0f;
        float FOV = 2.0f * FMath::Atan(SensorWidth / (2.0f * FocalLength));
        FOV = FMath::RadiansToDegrees(FOV);
        
        PlayerCamera->SetFieldOfView(FOV);
        
        // Apply depth of field effect based on aperture
        if (PhotoPostProcess)
        {
            // Lower aperture = shallower depth of field
            bool bEnableDoF = Aperture < 8.0f;
            PhotoPostProcess->Settings.bOverride_DepthOfFieldFstop = bEnableDoF;
            PhotoPostProcess->Settings.DepthOfFieldFstop = Aperture;
            
            // Calculate focus distance - in a full implementation this would
            // be based on what's in the center of the screen
            PhotoPostProcess->Settings.bOverride_DepthOfFieldFocalDistance = bEnableDoF;
            PhotoPostProcess->Settings.DepthOfFieldFocalDistance = 1000.0f; // 1 meter default
            
            // Set up motion blur based on shutter speed
            bool bEnableMotionBlur = ShutterSpeed > 1.0f/500.0f;
            PhotoPostProcess->Settings.bOverride_MotionBlurAmount = bEnableMotionBlur;
            
            // Map shutter speed to motion blur amount (longer shutter = more blur)
            float MotionBlurAmount = FMath::GetMappedRangeValueClamped(
                FVector2D(1.0f/500.0f, 1.0f/30.0f),
                FVector2D(0.1f, 1.0f),
                ShutterSpeed
            );
            PhotoPostProcess->Settings.MotionBlurAmount = MotionBlurAmount;
        }
    }
}

void UPhotographySystem::CaptureScreenshot(const FString& FileName)
{
    // Get the player controller
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        return;
    }
    
    // Hide UI for screenshot
    bool bShowHUD = PlayerController->GetHUD()->bShowHUD;
    PlayerController->GetHUD()->bShowHUD = false;
    
    if (PhotographyWidget)
    {
        PhotographyWidget->SetVisibility(ESlateVisibility::Hidden);
    }
    
    // Take a high-quality screenshot
    FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
    
    // Set screenshot quality options
    HighResScreenshotConfig.SetHDRCapture(true);
    HighResScreenshotConfig.ResolutionMultiplier = 2; // 2x supersampling
    
    // Capture the screenshot
    FString ScreenshotPath = FPaths::ScreenShotDir() / FileName;
    FScreenshotRequest::RequestScreenshot(ScreenshotPath, false, false);
    
    // Restore UI after capture
    PlayerController->GetHUD()->bShowHUD = bShowHUD;
    
    if (PhotographyWidget)
    {
        PhotographyWidget->SetVisibility(ESlateVisibility::Visible);
    }
}