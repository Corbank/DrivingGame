#include "World/WorldManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/DirectionalLight.h"
#include "Kismet/KismetMathLibrary.h"

AWorldManager::AWorldManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create components
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    RootComponent = SunLight;
    
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);
    
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    
    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);
    
    WeatherPostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("WeatherPostProcess"));
    WeatherPostProcess->SetupAttachment(RootComponent);
    
    // Default settings
    TimeOfDay = 12.0f; // Start at noon
    TimeScale = 1.0f;  // 1 minute in real time = 1 hour in game
    bUseRealTime = false;
    CurrentWeather = EWeatherType::Clear;
    WeatherChangeProbability = 0.05f; // 5% chance per minute
}

void AWorldManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initial setup of sun and weather
    UpdateSunPosition();
    UpdateWeatherEffects();
    
    // If using real-time, set the time of day to match the real world
    if (bUseRealTime)
    {
        // Get current system time
        FDateTime RealTime = FDateTime::Now();
        float RealHour = RealTime.GetHour() + RealTime.GetMinute() / 60.0f;
        
        // Set time of day to match real time
        SetTimeOfDay(RealHour);
    }
}

void AWorldManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update time of day
    if (!bUseRealTime)
    {
        // Scale the time passage
        float HoursToAdd = DeltaTime * TimeScale / 60.0f; // Convert minutes to hours
        TimeOfDay += HoursToAdd;
        
        // Wrap around at 24 hours
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay -= 24.0f;
        }
        
        // Update sun and sky
        UpdateSunPosition();
    }
    
    // Try to randomly change weather
    TryRandomWeatherChange(DeltaTime);
}

void AWorldManager::SetTimeOfDay(float NewTime)
{
    // Clamp and set the new time
    TimeOfDay = FMath::Fmod(NewTime, 24.0f);
    if (TimeOfDay < 0.0f)
    {
        TimeOfDay += 24.0f;
    }
    
    // Update sun position and related effects
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
    if (!SunLight)
        return;
    
    // Convert time of day to rotator for sun
    // 0 = midnight, 6 = sunrise, 12 = noon, 18 = sunset
    
    // Calculate sun's pitch and yaw based on time
    // We want the sun to rise in the east (90 degrees) and set in the west (270 degrees)
    float SunYaw = 90.0f + (TimeOfDay / 24.0f) * 360.0f;
    
    // Calculate pitch to simulate sun's arc (highest at noon, lowest at midnight)
    // Using a sinusoidal curve to simulate the sun's arc
    float SunPitch = -90.0f + 180.0f * FMath::Sin(FMath::DegreesToRadians((TimeOfDay / 24.0f) * 360.0f));
    
    // Set the sun's rotation
    FRotator SunRotation(SunPitch, SunYaw, 0.0f);
    SunLight->SetWorldRotation(SunRotation);
    
    // Adjust sun brightness based on time of day
    float SunBrightness = 0.0f;
    
    // Only have sun brightness between 6 AM and 6 PM
    if (TimeOfDay > 6.0f && TimeOfDay < 18.0f)
    {
        // Normalize the time between 6 AM and 6 PM to a 0-1 range
        float NormalizedTime = (TimeOfDay - 6.0f) / 12.0f;
        
        // Use a sine curve to make it peak at noon
        SunBrightness = FMath::Sin(NormalizedTime * PI);
    }
    
    // Scale the brightness to a reasonable range
    SunBrightness = SunBrightness * 10.0f + 0.2f;
    SunLight->SetIntensity(SunBrightness);
    
    // Adjust sun and sky colors based on time of day
    FLinearColor MorningColor = FLinearColor(1.0f, 0.8f, 0.5f); // Warm, golden sunrise
    FLinearColor DayColor = FLinearColor(1.0f, 1.0f, 1.0f);     // Bright, white day
    FLinearColor EveningColor = FLinearColor(1.0f, 0.5f, 0.2f); // Orange, warm sunset
    FLinearColor NightColor = FLinearColor(0.1f, 0.1f, 0.2f);   // Dark blue night
    
    FLinearColor SunColor;
    
    if (TimeOfDay < 6.0f) // Night to sunrise transition
    {
        float Alpha = TimeOfDay / 6.0f;
        SunColor = FLinearColor::LerpUsingHSV(NightColor, MorningColor, Alpha);
    }
    else if (TimeOfDay < 12.0f) // Sunrise to midday
    {
        float Alpha = (TimeOfDay - 6.0f) / 6.0f;
        SunColor = FLinearColor::LerpUsingHSV(MorningColor, DayColor, Alpha);
    }
    else if (TimeOfDay < 18.0f) // Midday to sunset
    {
        float Alpha = (TimeOfDay - 12.0f) / 6.0f;
        SunColor = FLinearColor::LerpUsingHSV(DayColor, EveningColor, Alpha);
    }
    else // Sunset to night
    {
        float Alpha = (TimeOfDay - 18.0f) / 6.0f;
        SunColor = FLinearColor::LerpUsingHSV(EveningColor, NightColor, Alpha);
    }
    
    SunLight->SetLightColor(SunColor);
    
    // Update the sky light to capture the new sun position
    if (SkyLight)
    {
        SkyLight->RecaptureScene();
    }
}

void AWorldManager::UpdateWeatherEffects()
{
    // Set weather-specific effects based on the current weather type
    if (!WeatherPostProcess || !VolumetricClouds)
        return;
    
    // Set volumetric cloud density and coverage based on weather
    switch (CurrentWeather)
    {
        case EWeatherType::Clear:
            VolumetricClouds->LayerBottomAltitude = 5000.0f;
            VolumetricClouds->LayerHeight = 2000.0f;
            VolumetricClouds->SetVolumetricCloudSettings(0.5f, 0.2f, 0.5f);  // Low density and coverage
            
            // Reset post-process effects
            WeatherPostProcess->Settings.bOverride_DepthOfFieldMethod = false;
            WeatherPostProcess->Settings.bOverride_SceneFringeIntensity = false;
            break;
            
        case EWeatherType::Cloudy:
            VolumetricClouds->LayerBottomAltitude = 4000.0f;
            VolumetricClouds->LayerHeight = 3000.0f;
            VolumetricClouds->SetVolumetricCloudSettings(0.7f, 0.6f, 0.5f);  // Medium density, high coverage
            
            // No special post-process effects for cloudy
            WeatherPostProcess->Settings.bOverride_DepthOfFieldMethod = false;
            WeatherPostProcess->Settings.bOverride_SceneFringeIntensity = false;
            break;
            
        case EWeatherType::Rain:
            VolumetricClouds->LayerBottomAltitude = 2000.0f;
            VolumetricClouds->LayerHeight = 4000.0f;
            VolumetricClouds->SetVolumetricCloudSettings(0.8f, 0.8f, 0.7f);  // High density and coverage
            
            // Rain effects (slight blur and dark tint)
            WeatherPostProcess->Settings.bOverride_ColorSaturation = true;
            WeatherPostProcess->Settings.ColorSaturation = FVector4(0.8f, 0.8f, 0.8f, 1.0f);
            
            // Spawn particle systems for rain if we have them
            SpawnWeatherParticles(true, false, false);
            break;
            
        case EWeatherType::Storm:
            VolumetricClouds->LayerBottomAltitude = 1000.0f;
            VolumetricClouds->LayerHeight = 5000.0f;
            VolumetricClouds->SetVolumetricCloudSettings(0.9f, 0.9f, 0.8f);  // Very high density and coverage
            
            // Storm effects (darker, more contrast)
            WeatherPostProcess->Settings.bOverride_ColorContrast = true;
            WeatherPostProcess->Settings.ColorContrast = FVector4(1.2f, 1.2f, 1.2f, 1.0f);
            WeatherPostProcess->Settings.bOverride_ColorSaturation = true;
            WeatherPostProcess->Settings.ColorSaturation = FVector4(0.7f, 0.7f, 0.7f, 1.0f);
            
            // Spawn particle systems for heavy rain and lightning if we have them
            SpawnWeatherParticles(true, true, false);
            break;
            
        case EWeatherType::Fog:
            VolumetricClouds->LayerBottomAltitude = 0.0f;
            VolumetricClouds->LayerHeight = 2000.0f;
            VolumetricClouds->SetVolumetricCloudSettings(0.2f, 0.3f, 0.5f);  // Low density, medium coverage
            
            // Fog effects (blur for distance)
            WeatherPostProcess->Settings.bOverride_DepthOfFieldMethod = true;
            WeatherPostProcess->Settings.DepthOfFieldMethod = EDepthOfFieldMethod::DOFM_Gaussian;
            WeatherPostProcess->Settings.bOverride_DepthOfFieldFocalDistance = true;
            WeatherPostProcess->Settings.DepthOfFieldFocalDistance = 5000.0f;
            WeatherPostProcess->Settings.bOverride_DepthOfFieldFstop = true;
            WeatherPostProcess->Settings.DepthOfFieldFstop = 2.0f;
            
            // Spawn fog particle systems if we have them
            SpawnWeatherParticles(false, false, true);
            break;
            
        case EWeatherType::Snow:
            VolumetricClouds->LayerBottomAltitude = 3000.0f;
            VolumetricClouds->LayerHeight = 3000.0f;
            VolumetricClouds->SetVolumetricCloudSettings(0.8f, 0.7f, 0.6f);  // High density, medium coverage
            
            // Snow effects (bright, slightly blue tint)
            WeatherPostProcess->Settings.bOverride_ColorGain = true;
            WeatherPostProcess->Settings.ColorGain = FVector4(0.9f, 0.95f, 1.1f, 1.0f);
            
            // Spawn snow particle systems if we have them
            SpawnWeatherParticles(false, false, false, true);
            break;
    }
}

void AWorldManager::TryRandomWeatherChange(float DeltaTime)
{
    // Calculate chance of weather change this frame
    float ChanceThisFrame = WeatherChangeProbability * DeltaTime / 60.0f; // Adjust to per-frame probability
    
    // Roll random chance
    if (FMath::FRand() < ChanceThisFrame)
    {
        // Determine next weather type
        // More likely to go to related weather states (e.g., Clear->Cloudy, Cloudy->Rain, etc.)
        
        EWeatherType NextWeather = CurrentWeather;
        float RandomValue = FMath::FRand();
        
        switch (CurrentWeather)
        {
            case EWeatherType::Clear:
                // From clear, most likely to go to cloudy, small chance for fog
                if (RandomValue < 0.8f)
                    NextWeather = EWeatherType::Cloudy;
                else
                    NextWeather = EWeatherType::Fog;
                break;
                
            case EWeatherType::Cloudy:
                // From cloudy, can go to clear, rain, or rarely snow (if cold)
                if (RandomValue < 0.3f)
                    NextWeather = EWeatherType::Clear;
                else if (RandomValue < 0.8f)
                    NextWeather = EWeatherType::Rain;
                else
                    NextWeather = EWeatherType::Snow;
                break;
                
            case EWeatherType::Rain:
                // From rain, can go to cloudy or escalate to storm
                if (RandomValue < 0.6f)
                    NextWeather = EWeatherType::Cloudy;
                else
                    NextWeather = EWeatherType::Storm;
                break;
                
            case EWeatherType::Storm:
                // Storm usually calms to rain
                NextWeather = EWeatherType::Rain;
                break;
                
            case EWeatherType::Fog:
                // Fog usually clears up or turns cloudy
                if (RandomValue < 0.7f)
                    NextWeather = EWeatherType::Clear;
                else
                    NextWeather = EWeatherType::Cloudy;
                break;
                
            case EWeatherType::Snow:
                // Snow usually goes to cloudy or clear
                if (RandomValue < 0.7f)
                    NextWeather = EWeatherType::Cloudy;
                else
                    NextWeather = EWeatherType::Clear;
                break;
        }
        
        // Apply the new weather
        if (NextWeather != CurrentWeather)
        {
            SetWeather(NextWeather);
        }
    }
}

void AWorldManager::SpawnWeatherParticles(bool bRain, bool bLightning, bool bFog, bool bSnow)
{
    // Find the player pawn to attach weather particles to it
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;
    
    // This is a placeholder function that would spawn particle systems for the different weather effects
    // In a real implementation, you would spawn and attach appropriate particle systems to the camera
    
    // For example:
    // if (bRain && RainParticleSystem)
    // {
    //     UGameplayStatics::SpawnEmitterAttached(
    //         RainParticleSystem,
    //         PlayerPawn->GetRootComponent(),
    //         NAME_None,
    //         FVector(0, 0, 500),  // Offset above player
    //         FRotator::ZeroRotator,
    //         FVector(1, 1, 1),
    //         EAttachLocation::KeepRelativeOffset,
    //         true
    //     );
    // }
    
    // Similar code would be used for lightning, fog, and snow effects
}