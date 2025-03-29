#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OpenWorldGameMode.generated.h"

/**
 * Main Game Mode for the Open World Explorer game
 */
UCLASS()
class OPENWORLDEXPLORER_API AOpenWorldGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AOpenWorldGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	// World manager for handling time of day and weather
	UPROPERTY(EditDefaultsOnly, Category = "World")
	TSubclassOf<class AWorldManager> WorldManagerClass;

	UPROPERTY()
	class AWorldManager* WorldManager;

	// Customization manager for player appearance and vehicle customization
	UPROPERTY(EditDefaultsOnly, Category = "Customization")
	TSubclassOf<class UCustomizationDatabase> CustomizationDatabaseClass;

	UPROPERTY()
	class UCustomizationDatabase* CustomizationDatabase;

	// Starting location for player
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	FVector PlayerStartLocation;

	// Available vehicle classes that can spawn in the world
	UPROPERTY(EditDefaultsOnly, Category = "Vehicles")
	TArray<TSubclassOf<class ABaseVehicle>> AvailableVehicleClasses;

	// Available character customization options
	UPROPERTY(EditDefaultsOnly, Category = "Customization")
	TArray<class USkeletalMesh*> CharacterBodyOptions;

	// Photography System
	UPROPERTY(EditDefaultsOnly, Category = "Photography")
	TSubclassOf<class UPhotographySystem> PhotographySystemClass;

public:
	// Get the world manager
	UFUNCTION(BlueprintCallable, Category = "World")
	class AWorldManager* GetWorldManager() const { return WorldManager; }

	// Get the customization database
	UFUNCTION(BlueprintCallable, Category = "Customization")
	class UCustomizationDatabase* GetCustomizationDatabase() const { return CustomizationDatabase; }

	// Spawn a vehicle in the world
	UFUNCTION(BlueprintCallable, Category = "Vehicles")
	class ABaseVehicle* SpawnVehicle(TSubclassOf<class ABaseVehicle> VehicleClass, const FTransform& Transform);

	// Add Photography System to player
	UFUNCTION(BlueprintCallable, Category = "Photography")
	class UPhotographySystem* AddPhotographySystem(class APawn* PlayerPawn);
};