#pragma once

#include "AlsInAirRotationMode.h"
#include "AlsMantlingSettings.h"
#include "AlsRagdollingSettings.h"
#include "AlsRollingSettings.h"
#include "AlsViewSettings.h"
#include "GameplayTagContainer.h"
#include "AlsCharacterSettings.generated.h"

USTRUCT(BlueprintType)
struct ALS_API FAlsCameraAngleLimits
{
    GENERATED_BODY()

    // Enable/disable limits for this configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS")
    bool bEnableLimits = false;

    // Horizontal (Yaw) limits relative to actor forward direction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS", 
        Meta = (EditCondition = "bEnableLimits", ClampMin = -180, ClampMax = 180))
    float MinYawAngle = -90.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS",
        Meta = (EditCondition = "bEnableLimits", ClampMin = -180, ClampMax = 180))
    float MaxYawAngle = 90.0f;

    // Vertical (Pitch) limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS",
        Meta = (EditCondition = "bEnableLimits", ClampMin = -90, ClampMax = 90))
    float MinPitchAngle = -60.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS",
        Meta = (EditCondition = "bEnableLimits", ClampMin = -90, ClampMax = 90))
    float MaxPitchAngle = 60.0f;

    // Interpolation settings for smooth transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS",
        Meta = (EditCondition = "bEnableLimits", ClampMin = 0))
    float InterpolationSpeed = 5.0f;

    // Use hard clamp vs soft elastic boundary
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS",
        Meta = (EditCondition = "bEnableLimits"))
    bool bUseSoftLimits = true;

    // Elastic force when using soft limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS",
        Meta = (EditCondition = "bEnableLimits && bUseSoftLimits", ClampMin = 0, ClampMax = 10))
    float ElasticStrength = 2.0f;

    // Dead zone before soft limits kick in (degrees from limit)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS",
        Meta = (EditCondition = "bEnableLimits && bUseSoftLimits", ClampMin = 0, ClampMax = 10))
    float SoftLimitDeadZone = 5.0f;
};

UCLASS(BlueprintType)
class ALS_API UAlsCameraLimitSettings : public UDataAsset
{
    GENERATED_BODY()

public:
    // Map of locomotion action tags to camera limits
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion Actions")
    TMap<FGameplayTag, FAlsCameraAngleLimits> LocomotionActionLimits;

    // Map of stance tags to camera limits (applied when no locomotion action is active)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stances")
    TMap<FGameplayTag, FAlsCameraAngleLimits> StanceLimits;

    // Priority system: Locomotion actions override stance limits
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
    bool bLocomotionActionOverridesStance = true;

    // Global interpolation for transitioning between different limit sets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings",
        Meta = (ClampMin = 0))
    float LimitTransitionSpeed = 3.0f;
};

UCLASS(Blueprintable, BlueprintType)
class ALS_API UAlsCharacterSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	// Actor will be considered as teleported if he has moved farther than this distance in 1 frame.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm"))
	float TeleportDistanceThreshold{50.0f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float MovingSpeedThreshold{50.0f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	EAlsInAirRotationMode InAirRotationMode{EAlsInAirRotationMode::RotateToVelocityOnJump};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bAllowAimingWhenInAir : 1 {true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bSprintHasPriorityOverAiming : 1 {false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bRotateToVelocityWhenSprinting : 1 {false};

	// If checked, the character will rotate relative to the object it is standing on in the velocity
	// direction rotation mode, otherwise the character will ignore that object and keep its world rotation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bInheritMovementBaseRotationInVelocityDirectionRotationMode : 1 {false};

	// If checked, the character will rotate towards the direction they want to move, but is not always able to due to obstacles.
	// This setting is only used if the bIgnoreBaseRotation setting from the character movement component is set to true.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bRotateTowardsDesiredVelocityInVelocityDirectionRotationMode : 1 {true};

	// If checked, the character will automatically rotate towards the view direction on any movement input while the actor is not moving.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	uint8 bAutoRotateOnAnyInputWhileNotMovingInViewDirectionRotationMode : 1 {true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsViewSettings View;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsGeneralMantlingSettings Mantling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsRagdollingSettings Ragdolling;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	FAlsRollingSettings Rolling;

public:
	UAlsCharacterSettings();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& ChangedEvent) override;
#endif
};
