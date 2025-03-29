#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhotographySystem.generated.h"

// Filter types for photography
UENUM(BlueprintType)
enum class EPhotoFilter : uint8
{
	None,
	Warm,
	Cool,
	Vintage,
	BlackAndWhite,
	Sepia,
	HighContrast,
	Dramatic,
	Vibrant
};

// Photo metadata saved with each photo
USTRUCT(BlueprintType)
struct FPhotoMetadata
{
	GENERATED_BODY()

	// Location where photo was taken
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	// Date and time photo was taken
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	// Weather condition during photo capture
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WeatherCondition;

	// Time of day (in-game hours)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeOfDay;

	// Filter applied to the photo
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPhotoFilter AppliedFilter;

	// Location name if taken at a point of interest
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LocationName;

	// Vehicles captured in the photo (if any)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> CapturedVehicles;
};

/**
 * Photography system component that allows players to take photos in-game
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OPENWORLDEXPLORER_API UPhotographySystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPhotographySystem();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	// Camera components
	UPROPERTY()
	class UCameraComponent* PhotoCamera;

	// Post process component for applying photo effects
	UPROPERTY()
	class UPostProcessComponent* PhotoEffects;

	// Materials for different filters
	UPROPERTY(EditDefaultsOnly, Category = "Photography|Effects")
	TMap<EPhotoFilter, class UMaterialInterface*> FilterMaterials;

	// Current filter selection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photography|Settings")
	EPhotoFilter CurrentFilter;

	// Camera FOV settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photography|Settings", meta = (ClampMin = "10.0", ClampMax = "170.0"))
	float DefaultFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photography|Settings", meta = (ClampMin = "10.0", ClampMax = "170.0"))
	float MinFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photography|Settings", meta = (ClampMin = "10.0", ClampMax = "170.0"))
	float MaxFOV;

	// Photo resolution
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photography|Settings")
	FIntPoint PhotoResolution;

	// Photography UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photography|UI")
	TSubclassOf<class UUserWidget> ViewfinderWidgetClass;

	UPROPERTY()
	class UUserWidget* ViewfinderWidget;

	// Is currently in photography mode
	UPROPERTY(BlueprintReadOnly, Category = "Photography")
	bool bInPhotoMode;

	// Original game state before entering photo mode
	FTransform OriginalCameraTransform;
	float OriginalGameTimeDilation;
	bool bOriginalHUDVisible;

public:
	// Enter/exit photography mode
	UFUNCTION(BlueprintCallable, Category = "Photography")
	void EnterPhotoMode();

	UFUNCTION(BlueprintCallable, Category = "Photography")
	void ExitPhotoMode();

	// Take a photo with current settings
	UFUNCTION(BlueprintCallable, Category = "Photography")
	void TakePhoto();

	// Change the current filter
	UFUNCTION(BlueprintCallable, Category = "Photography")
	void SetFilter(EPhotoFilter NewFilter);

	// Cycle to the next/previous filter
	UFUNCTION(BlueprintCallable, Category = "Photography")
	void CycleFilterForward();

	UFUNCTION(BlueprintCallable, Category = "Photography")
	void CycleFilterBackward();

	// Adjust camera zoom (FOV)
	UFUNCTION(BlueprintCallable, Category = "Photography")
	void AdjustZoom(float ZoomAmount);

	// Toggle UI visibility in photo mode
	UFUNCTION(BlueprintCallable, Category = "Photography")
	void ToggleUI();
	
	// Get current active filter
	UFUNCTION(BlueprintPure, Category = "Photography")
	EPhotoFilter GetCurrentFilter() const { return CurrentFilter; }
	
	// Check if in photo mode
	UFUNCTION(BlueprintPure, Category = "Photography")
	bool IsInPhotoMode() const { return bInPhotoMode; }

private:
	// Apply current filter to the post process material
	void ApplyCurrentFilter();

	// Capture the photo and save it to disk
	void CaptureScreenshot();

	// Generate metadata for the current photo
	FPhotoMetadata GeneratePhotoMetadata();

	// Check for nearby points of interest
	FString DetectNearbyLocationName();

	// Get the current weather condition from the world manager
	FString GetCurrentWeatherCondition();

	// Check for vehicles in the photo frame
	TArray<FString> DetectVehiclesInFrame();

	// Is UI currently visible in photo mode
	bool bUIVisible;
};