#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhotographySystem.generated.h"

// Enum for different photo filters
UENUM(BlueprintType)
enum class EPhotoFilter : uint8
{
	None            UMETA(DisplayName = "None"),
	Vintage         UMETA(DisplayName = "Vintage"),
	BlackAndWhite   UMETA(DisplayName = "Black and White"),
	Sepia           UMETA(DisplayName = "Sepia"),
	Dramatic        UMETA(DisplayName = "Dramatic"),
	Cinematic       UMETA(DisplayName = "Cinematic")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class OPENWORLDEXPLORER_API UPhotographySystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPhotographySystem();

protected:
	virtual void BeginPlay() override;
    
    // Camera components
    UPROPERTY()
    class UCameraComponent* PlayerCamera;
    
    // Current filter applied
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photography")
    EPhotoFilter CurrentFilter;
    
    // Post process component for photo effects
    UPROPERTY()
    class UPostProcessComponent* PhotoPostProcess;
    
    // Material instance for filters
    UPROPERTY()
    class UMaterialInstanceDynamic* FilterMaterial;
    
    // UI Widget for photography mode
    UPROPERTY(EditDefaultsOnly, Category = "Photography")
    TSubclassOf<class UUserWidget> PhotographyWidgetClass;
    
    UPROPERTY()
    class UUserWidget* PhotographyWidget;
    
    // Properties for photo framing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photography")
    float FocalLength;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photography")
    float Aperture;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photography")
    float ShutterSpeed;
    
    // Is in photography mode
    bool bIsInPhotographyMode;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    // Enter photography mode
    UFUNCTION(BlueprintCallable, Category = "Photography")
    void EnterPhotographyMode();
    
    // Exit photography mode
    UFUNCTION(BlueprintCallable, Category = "Photography")
    void ExitPhotographyMode();
    
    // Toggle photography mode
    UFUNCTION(BlueprintCallable, Category = "Photography")
    void TogglePhotographyMode();
    
    // Take a photo and save to disk
    UFUNCTION(BlueprintCallable, Category = "Photography")
    void TakePhoto();
    
    // Apply a photo filter
    UFUNCTION(BlueprintCallable, Category = "Photography")
    void ApplyFilter(EPhotoFilter Filter);
    
    // Adjust focal length
    UFUNCTION(BlueprintCallable, Category = "Photography")
    void SetFocalLength(float NewFocalLength);
    
    // Adjust aperture
    UFUNCTION(BlueprintCallable, Category = "Photography")
    void SetAperture(float NewAperture);
    
    // Adjust shutter speed
    UFUNCTION(BlueprintCallable, Category = "Photography")
    void SetShutterSpeed(float NewShutterSpeed);
    
    // Is in photography mode
    UFUNCTION(BlueprintPure, Category = "Photography")
    bool IsInPhotographyMode() const { return bIsInPhotographyMode; }
    
private:
    // Setup post-process for photography mode
    void SetupPostProcess();
    
    // Update camera settings
    void UpdateCameraSettings();
    
    // Save screenshot to file
    void SaveScreenshot();
    
    // Helper function for screenshot capture
    void CaptureScreenshot(const FString& FileName);
};