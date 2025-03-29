#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ChaosVehicleMovementComponent.h"
#include "BaseVehicle.generated.h"

UCLASS(Abstract, BlueprintType)
class OPENWORLDEXPLORER_API ABaseVehicle : public APawn
{
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    ABaseVehicle();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Vehicle mesh component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
    class USkeletalMeshComponent* VehicleMesh;

    // Vehicle movement component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vehicle")
    class UChaosVehicleMovementComponent* VehicleMovement;

    // Camera spring arm component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* CameraSpringArm;

    // Camera component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* Camera;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Input functions
    void ApplyThrottle(float Value);
    void ApplySteering(float Value);
    void ApplyHandbrake();
    void ReleaseHandbrake();

    // Customization functions
    UFUNCTION(BlueprintCallable, Category = "Vehicle|Customization")
    virtual void SetVehicleColor(const FLinearColor& Color);

    UFUNCTION(BlueprintCallable, Category = "Vehicle|Customization")
    virtual void SetWheelType(int32 WheelTypeIndex);

    UFUNCTION(BlueprintCallable, Category = "Vehicle|Customization")
    virtual void AddVehicleAccessory(class UStaticMeshComponent* AccessoryMesh, FName SocketName);
};