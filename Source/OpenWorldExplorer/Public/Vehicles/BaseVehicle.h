#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseVehicle.generated.h"

/**
 * Base vehicle class for all vehicles in the Open World Explorer game
 */
UCLASS()
class OPENWORLDEXPLORER_API ABaseVehicle : public APawn
{
	GENERATED_BODY()
	
public:	
	ABaseVehicle();

protected:
	virtual void BeginPlay() override;

	// Core vehicle components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	class USkeletalMeshComponent* VehicleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
	class UChaosVehicleMovementComponent* VehicleMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	// Vehicle properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float Acceleration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float BrakingForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	float TurnRate;

	// Input bindings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* VehicleMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* ThrottleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* SteeringAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* BrakeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* HandbrakeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* CameraToggleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* ExitVehicleAction;

public:	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Vehicle control functions
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Control")
	virtual void ApplyThrottle(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Control")
	virtual void ApplySteering(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Control")
	virtual void ApplyBrake(float Value);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Control")
	virtual void ApplyHandbrake(bool bEnabled);

	// Camera functions
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Camera")
	void ToggleCameraView();

	// Exit vehicle function
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Interaction")
	void ExitVehicle();

	// Customization functions
	UFUNCTION(BlueprintCallable, Category = "Vehicle|Customization")
	virtual void SetVehicleColor(const FLinearColor& Color);

	UFUNCTION(BlueprintCallable, Category = "Vehicle|Customization")
	void AddVehicleAccessory(USceneComponent* AccessoryComponent, FName SocketName);

private:
	// Current camera view state
	bool bIsFirstPersonView;

	// Process input for Enhanced Input system
	void ProcessThrottleInput(const struct FInputActionValue& Value);
	void ProcessSteeringInput(const struct FInputActionValue& Value);
	void ProcessBrakeInput(const struct FInputActionValue& Value);
	void ProcessHandbrakeInput(const struct FInputActionValue& Value);
};