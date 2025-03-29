#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldManager.generated.h"

// Enum for different weather types
UENUM(BlueprintType)
enum class EWeatherType : uint8
{
    Clear        UMETA(DisplayName = "Clear"),
    Cloudy       UMETA(DisplayName = "Cloudy"),
    Rain         UMETA(DisplayName = "Rain"),
    Storm        UMETA(DisplayName = "Storm"),
    Fog          UMETA(DisplayName = "Fog"),
    Snow         UMETA(DisplayName = "Snow")
};

UCLASS()
class OPENWORLDEXPLORER_API AWorldManager : public AActor
{
    GENERATED_BODY()
    
public:    
    // Sets default values for this actor's properties
    AWorldManager();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // The directional light representing the sun
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    class UDirectionalLightComponent* SunLight;

    // Sky atmosphere component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    class USkyAtmosphereComponent* SkyAtmosphere;

    // Sky light component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    class USkyLightComponent* SkyLight;
    
    // Volumetric cloud component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    class UVolumetricCloudComponent* VolumetricClouds;

    // Post process component for weather effects
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    class UPostProcessComponent* WeatherPostProcess;

    // Current weather type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EWeatherType CurrentWeather;

    // Current time of day (0.0-24.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TimeOfDay;

    // Time scale (how fast time passes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float TimeScale;

    // Whether to use real-world time
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bUseRealTime;

    // Weather change probability per minute
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WeatherChangeProbability;

public:    
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Set the time of day (0-24)
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetTimeOfDay(float NewTime);

    // Get the current time of day
    UFUNCTION(BlueprintPure, Category = "Environment")
    float GetTimeOfDay() const { return TimeOfDay; }

    // Set the current weather
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetWeather(EWeatherType NewWeather);

    // Get the current weather
    UFUNCTION(BlueprintPure, Category = "Environment")
    EWeatherType GetCurrentWeather() const { return CurrentWeather; }

private:
    // Update the sun position based on time of day
    void UpdateSunPosition();

    // Update the weather effects
    void UpdateWeatherEffects();

    // Try to change weather randomly
    void TryRandomWeatherChange(float DeltaTime);
};