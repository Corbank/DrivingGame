#include "World/WorldManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/ExponentialHeightFog.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

AWorldManager::AWorldManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create directional sun light
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(20.0f);
    SunLight->SetLightColor(FColor(255, 250, 235));
    SunLight->SetTemperature(6500.0f);
    SunLight->SetDynamicShadowCascades(4);
    SunLight->SetCascadeDistributionExponent(3.0f);
    SunLight->SetAtmosphereSunLight(true);

    // Create sky atmosphere
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);
    
    // Create sky light
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SourceType = ESkyLightSourceType::SLS_CapturedScene;
    SkyLight->bRealTimeCapture = true;
    
    // Create volumetric clouds
    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);
    
    // Create post process component for weather effects
    WeatherPostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("WeatherPostProcess"));
    WeatherPostProcess->SetupAttachment(RootComponent);
    WeatherPostProcess->bUnbound = true;
    WeatherPostProcess->Priority = 1.0f;
    
    // Set default values
    TimeOfDay = 12.0f;  // Start at noon
    TimeScale = 1.0f;   // Normal time speed
    bUseRealTime = false;
    CurrentWeather = EWeatherType::Clear;
    WeatherChangeProbability = 0.05f; // 5% chance per minute
}

void AWorldManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize the sun position and weather effects
    UpdateSunPosition();
    UpdateWeatherEffects();
}

void AWorldManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bUseRealTime)
    {
        // Get real-world time
        FDateTime Now = FDateTime::Now();
        TimeOfDay = Now.GetHour() + Now.GetMinute() / 60.0f;
    }
    else
    {
        // Update time of day
        TimeOfDay += DeltaTime * TimeScale / 60.0f; // Convert from real seconds to game minutes
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay -= 24.0f;
        }
    }

    // Update the sun position
    UpdateSunPosition();
    
    // Try random weather change
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
    // This is a simplified model where noon = sun directly overhead
    float SunAngle = ((TimeOfDay - 12.0f) / 12.0f) * 180.0f;
    FRotator SunRotation(-SunAngle, 0.0f, 0.0f);
    SunLight->SetRelativeRotation(SunRotation);
    
    // Adjust light intensity based on time of day
    float DayNightAlpha = FMath::Cos(FMath::DegreesToRadians(SunAngle));
    DayNightAlpha = FMath::Clamp((DayNightAlpha + 1.0f) / 2.0f, 0.0f, 1.0f);
    
    // Intensity ranges from 0.1 (night) to 20.0 (day)
    float Intensity = FMath::Lerp(0.1f, 20.0f, DayNightAlpha);
    SunLight->SetIntensity(Intensity);
    
    // Adjust light color based on time of day (warmer at sunrise/sunset)
    float ColorTemp = FMath::Lerp(2500.0f, 6500.0f, DayNightAlpha);
    SunLight->SetTemperature(ColorTemp);
    
    // Update sky light intensity
    SkyLight->SetIntensity(FMath::Lerp(0.02f, 1.0f, DayNightAlpha));
    
    // Need to recapture sky when sun position changes significantly
    static float LastCaptureTime = -1.0f;
    if (FMath::Abs(TimeOfDay - LastCaptureTime) > 0.5f) // Every half hour of game time
    {
        SkyLight->RecaptureSky();
        LastCaptureTime = TimeOfDay;
    }
}

void AWorldManager::UpdateWeatherEffects()
{
    // This would normally involve parameters for various effects like
    // fog density, cloud coverage, rain particle systems, etc.
    
    // For demonstration purposes, let's adjust cloud coverage and fog based on weather type
    switch (CurrentWeather)
    {
        case EWeatherType::Clear:
            // Lower cloud coverage
            VolumetricClouds->LayerBottomAltitude = 5000.0f;
            VolumetricClouds->LayerHeight = 5000.0f;
            VolumetricClouds->SetCoverage(0.2f);
            break;
            
        case EWeatherType::Cloudy:
            // Increase cloud coverage
            VolumetricClouds->LayerBottomAltitude = 3000.0f;
            VolumetricClouds->LayerHeight = 8000.0f;
            VolumetricClouds->SetCoverage(0.7f);
            break;
            
        case EWeatherType::Rain:
            // Heavy cloud coverage and potentially rain particles
            VolumetricClouds->LayerBottomAltitude = 2000.0f;
            VolumetricClouds->LayerHeight = 10000.0f;
            VolumetricClouds->SetCoverage(0.9f);
            
            // Additional rain effects would be spawned here
            // SpawnRainParticleSystem();
            break;
            
        case EWeatherType::Storm:
            // Very heavy clouds, rain, and lightning
            VolumetricClouds->LayerBottomAltitude = 1500.0f;
            VolumetricClouds->LayerHeight = 12000.0f;
            VolumetricClouds->SetCoverage(1.0f);
            
            // Additional storm effects would be spawned here
            // SpawnStormEffects();
            break;
            
        case EWeatherType::Fog:
            // Add height fog
            VolumetricClouds->SetCoverage(0.4f);
            
            // Additional fog would be manipulated through an ExponentialHeightFog actor
            break;
            
        case EWeatherType::Snow:
            // Snow clouds and particles
            VolumetricClouds->LayerBottomAltitude = 2000.0f;
            VolumetricClouds->LayerHeight = 9000.0f;
            VolumetricClouds->SetCoverage(0.8f);
            
            // Additional snow effects would be spawned here
            // SpawnSnowParticleSystem();
            break;
    }
    
    // For a complete implementation, we would also:
    // 1. Adjust post process settings to match weather
    // 2. Spawn and control particle systems for precipitation
    // 3. Update material parameters for wet/snowy surfaces
    // 4. Adjust sound effects for ambient weather
}

void AWorldManager::TryRandomWeatherChange(float DeltaTime)
{
    // Calculate probability of weather changing this frame
    float ProbabilityThisFrame = WeatherChangeProbability * (DeltaTime / 60.0f);
    
    if (FMath::FRand() < ProbabilityThisFrame)
    {
        // Select a new random weather type, excluding the current one
        TArray<EWeatherType> PossibleWeathers;
        for (int32 i = 0; i < (int32)EWeatherType::Snow + 1; i++)
        {
            EWeatherType WeatherType = (EWeatherType)i;
            if (WeatherType != CurrentWeather)
            {
                PossibleWeathers.Add(WeatherType);
            }
        }
        
        if (PossibleWeathers.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, PossibleWeathers.Num() - 1);
            SetWeather(PossibleWeathers[RandomIndex]);
        }
    }
}