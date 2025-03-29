#pragma once

#include "CoreMinimal.h"
#include "Vehicles/BaseVehicle.h"
#include "CarVehicle.generated.h"

/**
 * CarVehicle class that implements a standard car in the open world
 * with specific car behaviors and customization options
 */
UCLASS()
class OPENWORLDEXPLORER_API ACarVehicle : public ABaseVehicle
{
	GENERATED_BODY()
	
public:
	ACarVehicle();

protected:
	virtual void BeginPlay() override;

	// Car-specific components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	class UAudioComponent* EngineSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	class UStaticMeshComponent* BodyworkMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Accessories")
	class UStaticMeshComponent* SpoilerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Accessories")
	class UStaticMeshComponent* FrontBumperMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle|Accessories")
	class UStaticMeshComponent* RearBumperMesh;

	// Car settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Engine")
	float HorsePower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Engine")
	float MaxRPM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle|Engine")
	float TopSpeed;

public:
	virtual void Tick(float DeltaTime) override;

	// Car-specific customization methods
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Customization")
	void SetBodywork(UStaticMesh* NewBodyworkMesh);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Customization")
	void SetSpoiler(UStaticMesh* NewSpoilerMesh);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Customization")
	void SetFrontBumper(UStaticMesh* NewFrontBumperMesh);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Customization")
	void SetRearBumper(UStaticMesh* NewRearBumperMesh);

	// Override from BaseVehicle
	virtual void SetVehicleColor(const FLinearColor& Color) override;

private:
	// Update engine sound based on RPM
	void UpdateEngineSound(float CurrentRPM);
};