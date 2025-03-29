#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ExplorerCharacter.generated.h"

UCLASS(Blueprintable)
class OPENWORLDEXPLORER_API AExplorerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AExplorerCharacter();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Camera boom positioning the camera behind the character
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class USpringArmComponent* CameraBoom;

    // Follow camera
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    class UCameraComponent* FollowCamera;

    // Input mapping context
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* DefaultMappingContext;

    // Jump input action
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* JumpAction;

    // Move input action
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* MoveAction;

    // Look input action
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* LookAction;

    // Interact input action
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* InteractAction;

    // Camera toggle action
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    class UInputAction* CameraToggleAction;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Move input handler
    void Move(const struct FInputActionValue& Value);

    // Look input handler
    void Look(const struct FInputActionValue& Value);

    // Interact with world objects
    void Interact();

    // Toggle between first and third person cameras
    void ToggleCameraView();

    // Function to get in and out of vehicles
    UFUNCTION(BlueprintCallable, Category = "Character|Vehicle")
    void EnterVehicle(class ABaseVehicle* Vehicle);

    UFUNCTION(BlueprintCallable, Category = "Character|Vehicle")
    void ExitVehicle();

    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character|Customization")
    void SetCharacterAppearance(class USkeletalMesh* HeadMesh, class USkeletalMesh* BodyMesh);

    UFUNCTION(BlueprintCallable, Category = "Character|Customization")
    void SetCharacterOutfit(class UMaterialInstance* OutfitMaterial);

private:
    // Reference to current vehicle
    UPROPERTY()
    class ABaseVehicle* CurrentVehicle;

    // Is in first person view
    bool bIsFirstPersonView;
};