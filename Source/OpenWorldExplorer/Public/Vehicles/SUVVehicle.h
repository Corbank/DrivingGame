#pragma once

#include "CoreMinimal.h"
#include "Vehicles/BaseVehicle.h"
#include "SUVVehicle.generated.h"

/**
 * SUVVehicle class for off-road capabilities with specialized features
 * for exploration and traversing difficult terrain
 */
UCLASS()
class OPENWORLDEXPLORER_API ASUVVehicle : public ABaseVehicle
{
	GENERATED_BODY()
	
public:
	ASUVVehicle();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// SUV-specific functions
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Lights")
	void ToggleSpotlights(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Terrain")
	void ToggleOffroadMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Customization")
	void SetRoofRack(UStaticMesh* NewRoofRackMesh);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Customization")
	void SetBullBar(UStaticMesh* NewBullBarMesh);

	// Override from BaseVehicle for SUV-specific handling
	virtual void ApplyThrottle(float Value) override;
	virtual void ApplySteering(float Value) override;

protected:
	// SUV-specific components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	class UAudioComponent* EngineSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	class UStaticMeshComponent* RoofRackMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	class UStaticMeshComponent* BullBarMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	class USpotLightComponent* LeftSpotlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	class USpotLightComponent* RightSpotlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	class USpotLightComponent* RoofSpotlight;

	// SUV settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Terrain")
	float OffroadTractionMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Terrain")
	float WaterDepthTolerance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Engine")
	float MaxTorque;

private:
	// Terrain detection for SUV-specific handling
	void UpdateTerrainDetection();

	// Current terrain type the vehicle is on
	FName CurrentTerrainType;

	// Is offroad mode enabled (better handling on rough terrain)
	bool bOffroadModeEnabled;

	// Are spotlights enabled
	bool bSpotlightsEnabled;
};