#include "World/WorldManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"

AWorldManager::AWorldManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create the directional light (sun)
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->Intensity = 10.0f;
    SunLight->LightColor = FColor(255, 250, 240);
    SunLight->SetCastShadows(true);
    SunLight->SetDynamicShadowCascades(4);
    SunLight->CascadeDistributionExponent = 3.0f;
    SunLight->DynamicShadowDistanceStationaryLight = 20000.0f;
    SunLight->DynamicShadowDistanceMovableLight = 20000.0f;
    SunLight->AtmosphereSunLightIndex = 0;

    // Create the sky atmosphere
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);

    // Create the sky light
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SourceType = ESkyLightSourceType::SLS_CapturedScene;
    SkyLight->bRealTimeCapture = true;
    SkyLight->Intensity = 1.0f;

    // Create the volumetric clouds
    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);

    // Create the post process component for weather effects
    WeatherPostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("WeatherPostProcess"));
    WeatherPostProcess->SetupAttachment(RootComponent);
    WeatherPostProcess->bUnbound = true;

    // Set default values
    TimeOfDay = 12.0f; // Start at noon
    TimeScale = 0.05f; // 1 real second = 0.05 game hours (1440x real time)
    bUseRealTime = false;
    WeatherChangeProbability = 0.05f; // 5% chance per minute
    CurrentWeather = EWeatherType::Clear;
}

void AWorldManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize sun position
    UpdateSunPosition();
    
    // Initialize weather effects
    UpdateWeatherEffects();
}

void AWorldManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bUseRealTime)
    {
        // Update time of day
        TimeOfDay += DeltaTime * TimeScale;
        
        // Wrap around to keep between 0-24
        while (TimeOfDay >= 24.0f)
        {
            TimeOfDay -= 24.0f;
        }
        
        // Update sun position based on new time
        UpdateSunPosition();
    }
    else
    {
        // Get the real-world time and set the game time accordingly
        FDateTime RealWorldTime = FDateTime::Now();
        float Hours = RealWorldTime.GetHour();
        float Minutes = RealWorldTime.GetMinute();
        float Seconds = RealWorldTime.GetSecond();
        
        TimeOfDay = Hours + (Minutes / 60.0f) + (Seconds / 3600.0f);
        
        // Update sun position
        UpdateSunPosition();
    }
    
    // Check for random weather changes
    TryRandomWeatherChange(DeltaTime);
}

void AWorldManager::SetTimeOfDay(float NewTime)
{
    TimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateSunPosition();
}

void AWorldManager::SetWeather(EWeatherType NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        CurrentWeather = NewWeather;
        UpdateWeatherEffects();
    }
}

void AWorldManager::UpdateSunPosition()
{
    // Convert time of day to sun rotation
    // At 6 AM (6.0), sun is at horizon (270 degrees)
    // At noon (12.0), sun is at zenith (0 degrees)
    // At 6 PM (18.0), sun is at horizon (90 degrees)
    // At midnight (0.0 or 24.0), sun is at nadir (180 degrees)
    
    float SunAngle = FMath::DegreesToRadians((TimeOfDay - 12.0f) * 15.0f);
    
    // Calculate sun rotation
    float SunPitch = -FMath::Sin(SunAngle) * 90.0f;
    float SunYaw = 0.0f; // East-West axis
    
    FRotator SunRotation(SunPitch, SunYaw, 0.0f);
    SunLight->SetWorldRotation(SunRotation);
    
    // Adjust sun intensity and color based on time of day
    float SunHeight = FMath::Sin((TimeOfDay - 6.0f) / 12.0f * PI);
    float SunIntensity = FMath::Max(SunHeight, 0.0f) * 10.0f + 0.1f;
    
    SunLight->SetIntensity(SunIntensity);
    
    // Adjust sun color - warmer at sunrise/sunset, cooler at midday
    float DayCycle = FMath::Sin((TimeOfDay - 6.0f) / 12.0f * PI);
    FLinearColor SunColor;
    
    if (DayCycle > 0.0f)
    {
        // Day time
        float TimeFactor = FMath::Clamp(DayCycle, 0.0f, 0.5f) * 2.0f; // 0-1 from sunrise to noon
        if (TimeFactor < 0.5f)
        {
            // Sunrise - orange to white
            SunColor = FLinearColor::LerpUsingHSV(
                FLinearColor(1.0f, 0.6f, 0.3f), // Warm orange
                FLinearColor(1.0f, 1.0f, 0.95f), // Almost white
                TimeFactor * 2.0f);
        }
        else
        {
            // Mid-day to sunset - white to orange
            SunColor = FLinearColor::LerpUsingHSV(
                FLinearColor(1.0f, 1.0f, 0.95f), // Almost white
                FLinearColor(1.0f, 0.5f, 0.2f), // Deep orange
                (TimeFactor - 0.5f) * 2.0f);
        }
    }
    else
    {
        // Night time - moonlight is blueish
        SunColor = FLinearColor(0.7f, 0.8f, 1.0f);
    }
    
    SunLight->SetLightColor(SunColor);
    
    // Update skylight intensity based on time of day
    float SkyIntensity = FMath::Max(SunHeight, 0.0f) * 0.8f + 0.2f;
    SkyLight->SetIntensity(SkyIntensity);
    
    // At night, generate a recapture to get correct night sky
    if (DayCycle <= 0 && TimeOfDay != 0.0f)
    {
        SkyLight->RecaptureSky();
    }
}

void AWorldManager::UpdateWeatherEffects()
{
    // Configure weather effects based on current weather type
    if (!WeatherPostProcess)
    {
        return;
    }
    
    // Reset all weather-related settings
    WeatherPostProcess->Settings.bOverride_BloomIntensity = false;
    WeatherPostProcess->Settings.bOverride_AutoExposureBias = false;
    WeatherPostProcess->Settings.bOverride_VignetteIntensity = false;
    WeatherPostProcess->Settings.bOverride_ColorGamma = false;
    
    // Update volumetric clouds based on weather
    if (VolumetricClouds)
    {
        // Default cloud settings
        VolumetricClouds->SetLayerBottomAltitude(5000.0f);
        VolumetricClouds->SetLayerHeight(10000.0f);
        VolumetricClouds->SetCoverageType(0);
    }
    
    // Apply settings based on weather type
    switch (CurrentWeather)
    {
        case EWeatherType::Clear:
            if (VolumetricClouds)
            {
                VolumetricClouds->SetCoverageType(0);
            }
            break;
            
        case EWeatherType::Cloudy:
            if (VolumetricClouds)
            {
                VolumetricClouds->SetCoverageType(2); // More cloud coverage
            }
            
            // Slight exposure adjustment
            WeatherPostProcess->Settings.bOverride_AutoExposureBias = true;
            WeatherPostProcess->Settings.AutoExposureBias = -0.5f;
            break;
            
        case EWeatherType::Rain:
            if (VolumetricClouds)
            {
                VolumetricClouds->SetCoverageType(3); // Heavy cloud coverage
            }
            
            // Darker, cooler rain settings
            WeatherPostProcess->Settings.bOverride_AutoExposureBias = true;
            WeatherPostProcess->Settings.AutoExposureBias = -1.0f;
            
            WeatherPostProcess->Settings.bOverride_ColorGamma = true;
            WeatherPostProcess->Settings.ColorGamma = FVector4(0.9f, 0.95f, 1.05f, 1.0f);
            
            // Start rain particle effect if we had one
            
            break;
            
        case EWeatherType::Storm:
            if (VolumetricClouds)
            {
                VolumetricClouds->SetCoverageType(4); // Maximum cloud coverage
                VolumetricClouds->SetLayerBottomAltitude(2000.0f); // Lower clouds
            }
            
            // Dark, dramatic storm settings
            WeatherPostProcess->Settings.bOverride_AutoExposureBias = true;
            WeatherPostProcess->Settings.AutoExposureBias = -1.5f;
            
            WeatherPostProcess->Settings.bOverride_VignetteIntensity = true;
            WeatherPostProcess->Settings.VignetteIntensity = 0.5f;
            
            WeatherPostProcess->Settings.bOverride_ColorGamma = true;
            WeatherPostProcess->Settings.ColorGamma = FVector4(0.85f, 0.9f, 1.1f, 1.0f);
            
            // Start storm particle effects and lightning
            
            break;
            
        case EWeatherType::Fog:
            if (VolumetricClouds)
            {
                VolumetricClouds->SetCoverageType(1); // Light cloud coverage
                VolumetricClouds->SetLayerBottomAltitude(0.0f); // Ground-level clouds
                VolumetricClouds->SetLayerHeight(5000.0f); // Thinner layer
            }
            
            // Muted, hazy fog settings
            WeatherPostProcess->Settings.bOverride_BloomIntensity = true;
            WeatherPostProcess->Settings.BloomIntensity = 1.5f;
            
            WeatherPostProcess->Settings.bOverride_AutoExposureBias = true;
            WeatherPostProcess->Settings.AutoExposureBias = 0.5f;
            
            // Apply fog effect
            
            break;
            
        case EWeatherType::Snow:
            if (VolumetricClouds)
            {
                VolumetricClouds->SetCoverageType(3); // Heavy cloud coverage
            }
            
            // Bright, cool snow settings
            WeatherPostProcess->Settings.bOverride_AutoExposureBias = true;
            WeatherPostProcess->Settings.AutoExposureBias = 0.2f;
            
            WeatherPostProcess->Settings.bOverride_ColorGamma = true;
            WeatherPostProcess->Settings.ColorGamma = FVector4(0.95f, 1.0f, 1.05f, 1.0f);
            
            // Start snow particle effect
            
            break;
    }
}

void AWorldManager::TryRandomWeatherChange(float DeltaTime)
{
    // Calculate chance of weather change this frame
    // Convert probability per minute to probability per second to per frame
    float ProbabilityPerFrame = WeatherChangeProbability / 60.0f * DeltaTime;
    
    // Check for weather change
    if (FMath::FRand() < ProbabilityPerFrame)
    {
        // Select a new random weather type
        EWeatherType NewWeather = static_cast<EWeatherType>(FMath::RandRange(0, static_cast<int32>(EWeatherType::Snow)));
        
        // Don't pick the same weather
        if (NewWeather != CurrentWeather)
        {
            SetWeather(NewWeather);
        }
    }
}