#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "Settings/AlsMovementSettings.h"
#include "AlsCharacterMovementComponent.generated.h"

using FAlsPhysicsRotationDelegate = TMulticastDelegate<void(float DeltaTime)>;

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None		UMETA(Hidden),
	CMOVE_Slide		UMETA(DisplayName="Slide"),
	CMOVE_MAX		UMETA(Hidden),
};

UENUM(BlueprintType)
enum class ESlideTriggerType : uint8
{
	ESTT_DoubleTap		UMETA(DisplayName="Double Tap"),
	ESTT_SingleTap		UMETA(DisplayName="Single Tap")
};

// Controls which sources are allowed to start a slide.
UENUM(BlueprintType)
enum class ESlideTriggerMode : uint8
{
	// Slide starts automatically when the crouch input is tapped while moving fast enough (default behavior).
	Automatic	UMETA(DisplayName="Automatic (crouch while sprinting)"),
	// Slide only starts when explicitly requested via TryStartSlide()/SetWantsToSlide() (e.g. from a Gameplay Ability).
	Manual		UMETA(DisplayName="Manual (ability-driven)"),
	// Slide starts from either the automatic crouch trigger or an explicit request.
	Hybrid		UMETA(DisplayName="Hybrid (either)")
};

class ALS_API FAlsCharacterNetworkMoveData : public FCharacterNetworkMoveData
{
private:
	using Super = FCharacterNetworkMoveData;

public:
	FGameplayTag RotationMode{AlsRotationModeTags::ViewDirection};

	FGameplayTag Stance{AlsStanceTags::Standing};

	FGameplayTag MaxAllowedGait{AlsGaitTags::Running};

public:
	virtual void ClientFillNetworkMoveData(const FSavedMove_Character& Move, ENetworkMoveType MoveType) override;

	virtual bool Serialize(UCharacterMovementComponent& Movement, FArchive& Archive, UPackageMap* Map, ENetworkMoveType MoveType) override;
};

class ALS_API FAlsCharacterNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
{
public:
	TStaticArray<FAlsCharacterNetworkMoveData, 3> MoveData;

public:
	FAlsCharacterNetworkMoveDataContainer();
};

class ALS_API FAlsSavedMove : public FSavedMove_Character
{
private:
	using Super = FSavedMove_Character;

public:
	FGameplayTag RotationMode{AlsRotationModeTags::ViewDirection};

	FGameplayTag Stance{AlsStanceTags::Standing};

	FGameplayTag MaxAllowedGait{AlsGaitTags::Running};

	uint8 bWantsToProne : 1;

	uint8 bWantsToSlide : 1;
	uint8 bSlideStartedManually : 1;

	uint8 Saved_bPrevWantsToCrouch : 1;
	uint8 bSavedIsProned : 1;

	float SavedSlideTime;
	float SavedSlideCooldownRemaining;

public:
	virtual void Clear() override;

	virtual void SetMoveFor(ACharacter* Character, float NewDeltaTime, const FVector& NewAcceleration,
	                        FNetworkPredictionData_Client_Character& PredictionData) override;

	virtual bool CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, float MaxDeltaTime) const override;

	virtual void CombineWith(const FSavedMove_Character* PreviousMove, ACharacter* Character,
	                         APlayerController* Player, const FVector& PreviousStartLocation) override;

	virtual void PrepMoveFor(ACharacter* Character) override;

	virtual uint8 GetCompressedFlags() const override;
};

class ALS_API FAlsNetworkPredictionData : public FNetworkPredictionData_Client_Character
{
private:
	using Super = FNetworkPredictionData_Client_Character;

public:
	explicit FAlsNetworkPredictionData(const UCharacterMovementComponent& Movement);

	virtual FSavedMovePtr AllocateNewMove() override;
};

UCLASS(ClassGroup = "ALS")
class ALS_API UAlsCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend FAlsSavedMove;

public:
	// If checked, improves the response to interaction from moving kinematic physical
	// bodies, but may cause some issues when interacting with simulated physical bodies.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	uint8 bAllowImprovedPenetrationAdjustment : 1 {true};

protected:
	FAlsCharacterNetworkMoveDataContainer MoveDataContainer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<UAlsMovementSettings> MovementSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FAlsMovementGaitSettings GaitSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag RotationMode{AlsRotationModeTags::ViewDirection};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Stance{AlsStanceTags::Standing};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag MaxAllowedGait{AlsGaitTags::Running};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ClampMax = 3))
	float GaitAmount{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient, Meta = (ClampMin = 0, ForceUnits = "cm/s^2"))
	float MaxAccelerationWalking{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bMovementModeLocked : 1 {false};

	// Used to temporarily prohibit the player from moving the character. Also works for AI-controlled characters.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bInputBlocked : 1 {false};

	// Valid only on locally controlled characters.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FRotator PreviousControlRotation{ForceInit};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FVector PrePenetrationAdjustmentVelocity{ForceInit};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	uint8 bPrePenetrationAdjustmentVelocityValid : 1 {false};

public:
	FAlsPhysicsRotationDelegate OnPhysicsRotation;

public:
	UAlsCharacterMovementComponent();

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif

	virtual void BeginPlay() override;

	virtual FVector ConsumeInputVector() override;

	virtual void SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode = 0) override;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual bool IsMovingOnGround() const override;

	virtual bool ShouldPerformAirControlForPathFollowing() const override;

	virtual void UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation) override;

	virtual bool ApplyRequestedMove(float DeltaTime, float CurrentMaxAcceleration, float MaxSpeed, float Friction,
	                                float BrakingDeceleration, FVector& RequestedAcceleration, float& RequestedSpeed) override;

	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

	virtual float GetMaxSpeed() const override;

	virtual float GetMaxAcceleration() const override;

	virtual float GetMaxBrakingDeceleration() const override;

	virtual bool CanAttemptJump() const override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	virtual bool CanWalkOffLedges() const override;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

protected:
	virtual void ControlledCharacterMove(const FVector& InputVector, float DeltaTime) override;

public:
	virtual void PhysicsRotation(float DeltaTime) override;

	// ReSharper disable once CppRedefinitionOfDefaultArgumentInOverrideFunction
	virtual void MoveSmooth(const FVector& InVelocity, float DeltaTime, FStepDownResult* StepDownResult = nullptr) override;

protected:
	virtual void PhysWalking(float DeltaTime, int32 IterationsCount) override;

	virtual void PhysNavWalking(float DeltaTime, int32 IterationsCount) override;

	virtual void PhysCustom(float DeltaTime, int32 IterationsCount) override;

	virtual bool ClientUpdatePositionAfterServerUpdate() override;

public:
	virtual void ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance, float SweepDistance, FFindFloorResult& OutFloorResult,
	                              float SweepRadius, const FHitResult* DownwardSweepResult) const override;

protected:
	virtual void PerformMovement(float DeltaTime) override;

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
	virtual void SmoothClientPosition(float DeltaTime) override;

	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAcceleration) override;

	UFUNCTION(BlueprintCallable, Category = "ALS|Character Movement")
	virtual void RefreshGaitSettings();

public:
	UFUNCTION(BlueprintCallable, Category = "ALS|Character Movement")
	void SetMovementSettings(UAlsMovementSettings* NewMovementSettings);

	const FAlsMovementGaitSettings& GetGaitSettings() const;

	UFUNCTION(BlueprintPure, Category = "ALS|Character Movement")
	bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;

public:
	const FGameplayTag& GetRotationMode() const;

	void SetRotationMode(const FGameplayTag& NewRotationMode);

	const FGameplayTag& GetStance() const;

	void SetStance(const FGameplayTag& NewStance);

	const FGameplayTag& GetMaxAllowedGait() const;

	void SetMaxAllowedGait(const FGameplayTag& NewMaxAllowedGait);

	// Returns the character's current speed, mapped to the speed ranges from the movement settings.
	// Varies from 0 to 3, where 0 is stopped, 1 is walking, 2 is running, and 3 is sprinting.
	float GetGaitAmount() const;

protected:
	virtual void RefreshGroundedMovementSettings();

public:
	void SetMovementModeLocked(bool bNewMovementModeLocked);

	void SetInputBlocked(bool bNewInputBlocked);

	bool TryConsumePrePenetrationAdjustmentVelocity(FVector& OutVelocity);

public:
	UPROPERTY(Category="Als Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxWalkSpeedProned;

protected:
	UPROPERTY(Category="Character Movement (General Settings)", EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetPronedHalfHeight, BlueprintGetter=GetPronedHalfHeight, meta=(ClampMin="0", UIMin="0", ForceUnits=cm))
	float PronedHalfHeight;

public:
	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite)
	uint8 bCanWalkOffLedgesWhenProning:1;

public:
	UPROPERTY(Category="Character Movement (General Settings)", VisibleInstanceOnly, BlueprintReadOnly)
	uint8 bWantsToProne :1;

	UPROPERTY(Category="Character Movement (General Settings)", VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay)
	uint8 bProneMaintainsBaseLocation:1;

public:
	virtual void Prone(bool bClientSimulation = false);
	
	virtual void UnProne(bool bClientSimulation = false);

	virtual bool CanProneInCurrentState() const;

	UFUNCTION(BlueprintSetter)
	void SetPronedHalfHeight(const float NewValue);

	UFUNCTION(BlueprintGetter)
	float GetPronedHalfHeight() const;

public:
	virtual bool IsProning() const;

public:
	// Which sources are allowed to start a slide. Defaults to Automatic (tap crouch while sprinting).
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite)
	ESlideTriggerMode SlideTriggerMode = ESlideTriggerMode::Automatic;

	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite)
	ESlideTriggerType SlideTriggerType = ESlideTriggerType::ESTT_SingleTap;

	// Minimum gait amount (0..3) required for the automatic crouch trigger to start a slide.
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite)
	float MinSlideGaitAmount = 2.5f;

	// Slide ends once horizontal speed drops below this value.
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite)
	float MinSlideSpeed = 400.f;

	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite)
	float MaxSlideSpeed = 800.f;

	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite)
	float SlideEnterImpulse = 400.f;

	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite)
	float SlideGravityForce = 4000.f;

	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite)
	float SlideFrictionFactor = .06f;

	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite)
	float BrakingDecelerationSliding = 1000.f;

	// Maximum slide duration in seconds. After this the slide ends regardless of speed. 0 disables the
	// time limit, making the slide purely speed/cancel driven (the legacy behavior).
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ForceUnits="s"))
	float MaxSlideDuration = 1.0f;

	// Minimum time in seconds before another slide can start after one ends. 0 disables the cooldown.
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ForceUnits="s"))
	float SlideCooldown = 0.0f;

	// Fraction (0..1) of SlideMaxSteerAngle that lateral input can rotate the slide velocity by.
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ClampMax="1.0"))
	float SlideSteeringStrength = 0.5f;

	// Maximum angular speed (deg/s) the slide direction can be steered at full SlideSteeringStrength.
	UPROPERTY(Category = "Character Movement: Sliding", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", ForceUnits="deg/s"))
	float SlideMaxSteerAngle = 25.f;

protected:
	bool Safe_bPrevWantsToCrouch;

	// Runtime slide state. Replayed/restored via FAlsSavedMove so it stays client-predicted.

	// Set by TryStartSlide()/SetWantsToSlide(). Replicated to the autonomous proxy via FLAG_Custom_1.
	UPROPERTY(Category = "Character Movement: Sliding", VisibleInstanceOnly, BlueprintReadOnly)
	uint8 bWantsToSlide : 1;

	// True when the current slide was started by an explicit request rather than the crouch trigger.
	bool Safe_bSlideStartedManually;

	// Elapsed time of the current slide, compared against MaxSlideDuration.
	float Safe_SlideTime;

	// Counts down while not sliding; a new slide cannot start until it reaches zero.
	float Safe_SlideCooldownRemaining;

public:
	virtual float GetMaxSlideSpeed() const { return MaxSlideSpeed; }
	virtual float GetSlideEnterImpulse() const { return SlideEnterImpulse; }

	// Requests a slide. Honors SlideTriggerMode (no effect in Automatic mode). Returns whether the
	// slide can start right now (the actual transition happens on the next movement update so it stays
	// network-predicted). Intended to be called from a Gameplay Ability or any external system.
	UFUNCTION(BlueprintCallable, Category = "ALS|Character Movement")
	bool TryStartSlide();

	// Clears the manual slide request and ends the slide if it was started manually.
	UFUNCTION(BlueprintCallable, Category = "ALS|Character Movement")
	void StopSlide();

	// Low-level setter for the manual slide request flag.
	UFUNCTION(BlueprintCallable, Category = "ALS|Character Movement")
	void SetWantsToSlide(bool bNewWantsToSlide);

	UFUNCTION(BlueprintPure, Category = "ALS|Character Movement")
	bool IsSliding() const;

protected:
	bool IsSlideTriggered() const;
	bool CanStartSlide() const;
	bool ShouldEnterSlide() const;
	bool ShouldStopSlide() const;
	void EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode);
	void ExitSlide();
	bool CanSlide(bool bCheckSpeed = true) const;
	void PhysSlide(float deltaTime, int32 Iterations);
};

inline const FAlsMovementGaitSettings& UAlsCharacterMovementComponent::GetGaitSettings() const
{
	return GaitSettings;
}

inline const FGameplayTag& UAlsCharacterMovementComponent::GetRotationMode() const
{
	return RotationMode;
}

inline const FGameplayTag& UAlsCharacterMovementComponent::GetStance() const
{
	return Stance;
}

inline const FGameplayTag& UAlsCharacterMovementComponent::GetMaxAllowedGait() const
{
	return MaxAllowedGait;
}

inline void UAlsCharacterMovementComponent::SetMaxAllowedGait(const FGameplayTag& NewMaxAllowedGait)
{
	MaxAllowedGait = NewMaxAllowedGait;
}

inline float UAlsCharacterMovementComponent::GetGaitAmount() const
{
	return GaitAmount;
}
