#include "AlsCharacterMovementComponent.h"

#include "AlsCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveVector.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Utility/AlsMacros.h"
#include "Utility/AlsRotation.h"
#include "Utility/AlsUtility.h"
#include "Utility/AlsVector.h"
#include "Engine/ScopedMovementUpdate.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AlsCharacterMovementComponent)

DECLARE_CYCLE_STAT(TEXT("AlsChar ClientUpdatePositionAfterServerUpdate"), STAT_AlsCharacterMovementClientUpdatePositionAfterServerUpdate, STATGROUP_Als);

struct FAlsScopedMeshMovementUpdate
{
	FAlsScopedMeshMovementUpdate(USkeletalMeshComponent* Mesh, bool bEnabled = true)
		: ScopedMoveUpdate(bEnabled && /*CharacterMovementCVars::bDeferCharacterMeshMovement*/ false ? Mesh : nullptr, EScopedUpdate::DeferredUpdates)
	{
	}

private:
	FScopedMovementUpdate ScopedMoveUpdate;
};

void FAlsCharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& Move, const ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(Move, MoveType);

	const auto& SavedMove{static_cast<const FAlsSavedMove&>(Move)}; // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

	RotationMode = SavedMove.RotationMode;
	Stance = SavedMove.Stance;
	MaxAllowedGait = SavedMove.MaxAllowedGait;
}

bool FAlsCharacterNetworkMoveData::Serialize(UCharacterMovementComponent& Movement, FArchive& Archive,
                                             UPackageMap* Map, const ENetworkMoveType MoveType)
{
	Super::Serialize(Movement, Archive, Map, MoveType);

	NetSerializeOptionalValue(Archive.IsSaving(), Archive, RotationMode, AlsRotationModeTags::ViewDirection.GetTag(), Map);
	NetSerializeOptionalValue(Archive.IsSaving(), Archive, Stance, AlsStanceTags::Standing.GetTag(), Map);
	NetSerializeOptionalValue(Archive.IsSaving(), Archive, MaxAllowedGait, AlsGaitTags::Running.GetTag(), Map);

	return !Archive.IsError();
}

FAlsCharacterNetworkMoveDataContainer::FAlsCharacterNetworkMoveDataContainer()
{
	NewMoveData = &MoveData[0];
	PendingMoveData = &MoveData[1];
	OldMoveData = &MoveData[2];
}

void FAlsSavedMove::Clear()
{
	Super::Clear();

	RotationMode = AlsRotationModeTags::ViewDirection;
	Stance = AlsStanceTags::Standing;
	MaxAllowedGait = AlsGaitTags::Running;

	bWantsToProne = false;

	Saved_bPrevWantsToCrouch = false;
}

void FAlsSavedMove::SetMoveFor(ACharacter* Character, const float NewDeltaTime, const FVector& NewAcceleration,
                               FNetworkPredictionData_Client_Character& PredictionData)
{
	Super::SetMoveFor(Character, NewDeltaTime, NewAcceleration, PredictionData);

	const auto* Movement{Cast<UAlsCharacterMovementComponent>(Character->GetCharacterMovement())};
	if (IsValid(Movement))
	{
		RotationMode = Movement->RotationMode;
		Stance = Movement->Stance;
		MaxAllowedGait = Movement->MaxAllowedGait;

		bWantsToProne = Movement->bWantsToProne;

		Saved_bPrevWantsToCrouch = Movement->Safe_bPrevWantsToCrouch;
	}
}

bool FAlsSavedMove::CanCombineWith(const FSavedMovePtr& NewMovePtr, ACharacter* Character, const float MaxDeltaTime) const
{
	const auto* NewMove{static_cast<FAlsSavedMove*>(NewMovePtr.Get())}; // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

	return RotationMode == NewMove->RotationMode &&
	       Stance == NewMove->Stance &&
	       MaxAllowedGait == NewMove->MaxAllowedGait &&
	       Super::CanCombineWith(NewMovePtr, Character, MaxDeltaTime);
}

void FAlsSavedMove::CombineWith(const FSavedMove_Character* PreviousMove, ACharacter* Character,
                                APlayerController* Player, const FVector& PreviousStartLocation)
{
	// Calling Super::CombineWith() will force change the character's rotation to the rotation from the previous move, which is
	// undesirable because it will erase our rotation changes made in the AAlsCharacter class. So, to keep the rotation unchanged,
	// we simply override the saved rotations with the current rotation, and after calling Super::CombineWith() we restore them.

	const auto OriginalRotation{PreviousMove->StartRotation};
	const auto OriginalRelativeRotation{PreviousMove->StartAttachRelativeRotation};

	const auto* UpdatedComponent{Character->GetCharacterMovement()->UpdatedComponent.Get()};

	auto* MutablePreviousMove{const_cast<FSavedMove_Character*>(PreviousMove)};

	MutablePreviousMove->StartRotation = UpdatedComponent->GetComponentRotation();
	MutablePreviousMove->StartAttachRelativeRotation = UpdatedComponent->GetRelativeRotation();

	Super::CombineWith(PreviousMove, Character, Player, PreviousStartLocation);

	MutablePreviousMove->StartRotation = OriginalRotation;
	MutablePreviousMove->StartAttachRelativeRotation = OriginalRelativeRotation;
}

void FAlsSavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	auto* Movement{Cast<UAlsCharacterMovementComponent>(Character->GetCharacterMovement())};
	if (IsValid(Movement))
	{
		Movement->RotationMode = RotationMode;
		Movement->Stance = Stance;
		Movement->MaxAllowedGait = MaxAllowedGait;

		Movement->RefreshGaitSettings();

		Movement->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;
	}
}

uint8 FAlsSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bWantsToProne)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

FAlsNetworkPredictionData::FAlsNetworkPredictionData(const UCharacterMovementComponent& Movement) : Super{Movement} {}

FSavedMovePtr FAlsNetworkPredictionData::AllocateNewMove()
{
	return MakeShared<FAlsSavedMove>();
}

UAlsCharacterMovementComponent::UAlsCharacterMovementComponent()
{
	SetNetworkMoveDataContainer(MoveDataContainer);

	bRunPhysicsWithNoController = true;
	bAllowPhysicsRotationDuringAnimRootMotion = true;       // Required to be able to manually rotate the actor while rolling.
	bNetworkAlwaysReplicateTransformUpdateTimestamp = true; // Required for view network smoothing.

	SetCrouchedHalfHeight(56.0f);
	SetPronedHalfHeight(28.0f);

	// Default values for standing walking movement.

	MinAnalogWalkSpeed = 25.0f;
	MaxWalkSpeed = 375.0f;
	MaxWalkSpeedCrouched = 150.0f;
	MaxWalkSpeedProned = 90.0f;
	MaxAccelerationWalking = 2000.0f;
	BrakingDecelerationWalking = 1500.0f;
	GroundFriction = 4.0f;

	AirControl = 0.15f;

	// This value is only used when the actor is in the air, since when moving on the ground
	// the value from the AccelerationAndDecelerationAndGroundFriction curve is used instead.
	MaxAcceleration = 2000.0f;

	// Makes GroundFriction and FallingLateralFriction used for both acceleration and deceleration.
	bUseSeparateBrakingFriction = false;

	// Makes friction does not affect deceleration by default. Greater than zero only for a short period of time after landing.
	BrakingFrictionFactor = 0.0f;

	bCanWalkOffLedgesWhenCrouching = true;
	bCanWalkOffLedgesWhenProning = true;

	// Subtracted from the capsule radius to check how far the actor is allowed to
	// perch on the edge of a surface. Currently this is half the capsule radius.
	PerchRadiusThreshold = 15.0f;

	// This value allows the actor to perch a ledge whose height is close to MaxStepHeight.
	PerchAdditionalHeight = 0.0f;

	JumpOffJumpZFactor = 0.0f; // Makes the actor slide down instead of bouncing on a surface it can't stand on.

	// bImpartBaseVelocityX = false;
	// bImpartBaseVelocityY = false;
	// bImpartBaseVelocityZ = false;
	// bImpartBaseAngularVelocity = false;
	bIgnoreBaseRotation = true;

	// bStayBasedInAir = true;

	// These values prohibit the character movement component from affecting the actor's rotation.

	RotationRate = FRotator::ZeroRotator;
	bUseControllerDesiredRotation = false;
	bOrientRotationToMovement = false;

	NavAgentProps.bCanCrouch = true;
	NavMovementProperties.bUseAccelerationForPaths = true;
}

#if WITH_EDITOR
bool UAlsCharacterMovementComponent::CanEditChange(const FProperty* Property) const
{
	return Super::CanEditChange(Property) &&
	       Property->GetFName() != GET_MEMBER_NAME_STRING_VIEW_CHECKED(ThisClass, RotationRate) &&
	       Property->GetFName() != GET_MEMBER_NAME_STRING_VIEW_CHECKED(ThisClass, bUseControllerDesiredRotation) &&
	       Property->GetFName() != GET_MEMBER_NAME_STRING_VIEW_CHECKED(ThisClass, bOrientRotationToMovement);
}
#endif

void UAlsCharacterMovementComponent::BeginPlay()
{
	ALS_ENSURE_MESSAGE(!bUseControllerDesiredRotation && !bOrientRotationToMovement, // NOLINT(clang-diagnostic-unused-value)
	                   TEXT("These settings are not allowed and must be turned off!"));

	Super::BeginPlay();
}

FVector UAlsCharacterMovementComponent::ConsumeInputVector()
{
	auto InputVector{Super::ConsumeInputVector()};

	if (bInputBlocked)
	{
		return FVector::ZeroVector;
	}

	FRotator BaseRotationSpeed;
	if (!bIgnoreBaseRotation && UAlsUtility::TryGetMovementBaseRotationSpeed(CharacterOwner->GetBasedMovement(), BaseRotationSpeed))
	{
		// Offset the input vector to keep it relative to the movement base.
		InputVector = (BaseRotationSpeed * GetWorld()->GetDeltaSeconds()).RotateVector(InputVector);
	}

	return InputVector;
}

void UAlsCharacterMovementComponent::SetMovementMode(const EMovementMode NewMovementMode, const uint8 NewCustomMode)
{
	if (!bMovementModeLocked)
	{
		Super::SetMovementMode(NewMovementMode, NewCustomMode);
	}
}

void UAlsCharacterMovementComponent::OnMovementModeChanged(const EMovementMode PreviousMovementMode, const uint8 PreviousCustomMode)
{
	if (!HasValidData())
	{
		return;
	}

	if (MovementMode == MOVE_Walking)
	{
		bProneMaintainsBaseLocation = true;
	}
	else
	{
		bProneMaintainsBaseLocation = false;
	}

	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Slide)
	{
		ExitSlide();
	}

	if (IsCustomMovementMode(CMOVE_Slide))
	{
		EnterSlide(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);
	}

	// This removes some very noticeable changes in the mesh location when the
	// character automatically uncrouches at the end of the roll in the air.

	bCrouchMaintainsBaseLocation = true;
	bProneMaintainsBaseLocation = true;
}

void UAlsCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

bool UAlsCharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool UAlsCharacterMovementComponent::ShouldPerformAirControlForPathFollowing() const
{
	return !bInputBlocked && Super::ShouldPerformAirControlForPathFollowing();
}

void UAlsCharacterMovementComponent::UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation)
{
	// Ignore the parent implementation of this function and provide our own, because the parent
	// implementation has no effect when we ignore rotation changes in AAlsCharacter::FaceRotation().

	const auto& BasedMovement{CharacterOwner->GetBasedMovement()};

	FVector MovementBaseLocation;
	FQuat MovementBaseRotation;

	MovementBaseUtility::GetMovementBaseTransform(BasedMovement.MovementBase, BasedMovement.BoneName,
	                                              MovementBaseLocation, MovementBaseRotation);

	if (!OldBaseQuat.Equals(MovementBaseRotation, UE_SMALL_NUMBER))
	{
		const auto DeltaRotation{(MovementBaseRotation * OldBaseQuat.Inverse()).Rotator()};
		auto NewControlRotation{CharacterOwner->GetController()->GetControlRotation()};

		NewControlRotation.Pitch += DeltaRotation.Pitch;
		NewControlRotation.Yaw += DeltaRotation.Yaw;
		NewControlRotation.Normalize();

		CharacterOwner->GetController()->SetControlRotation(NewControlRotation);
	}
}

bool UAlsCharacterMovementComponent::ApplyRequestedMove(const float DeltaTime, const float CurrentMaxAcceleration,
                                                        const float MaxSpeed, const float Friction, const float BrakingDeceleration,
                                                        FVector& RequestedAcceleration, float& RequestedSpeed)
{
	return !bInputBlocked && Super::ApplyRequestedMove(DeltaTime, CurrentMaxAcceleration, MaxSpeed, Friction,
	                                                   BrakingDeceleration, RequestedAcceleration, RequestedSpeed);
}

void UAlsCharacterMovementComponent::CalcVelocity(const float DeltaTime, const float Friction,
                                                  const bool bFluid, const float BrakingDeceleration)
{
	FRotator BaseRotationSpeed;
	if (!bIgnoreBaseRotation && UAlsUtility::TryGetMovementBaseRotationSpeed(CharacterOwner->GetBasedMovement(), BaseRotationSpeed))
	{
		// Offset the velocity to keep it relative to the movement base.
		Velocity = (BaseRotationSpeed * DeltaTime).RotateVector(Velocity);
	}

	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}

float UAlsCharacterMovementComponent::GetMaxSpeed() const
{
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		return IsCrouching() ? MaxWalkSpeedCrouched : (IsProning() ? MaxWalkSpeedProned : MaxWalkSpeed);
	case MOVE_Falling:
	case MOVE_Swimming:
	case MOVE_Flying:
	case MOVE_Custom:
		switch (CustomMovementMode)
		{
		case CMOVE_Slide:
			return MaxSlideSpeed;
		}
	case MOVE_None:
	default:
		return Super::GetMaxSpeed();
	}
}

float UAlsCharacterMovementComponent::GetMaxAcceleration() const
{
	if (IsMovingOnGround())
	{
		return MaxAccelerationWalking;
	}

	return Super::GetMaxAcceleration();
}

float UAlsCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (MovementMode != MOVE_Custom) return Super::GetMaxBrakingDeceleration();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return BrakingDecelerationSliding;
	default:
		return Super::GetMaxBrakingDeceleration();
	}
}

bool UAlsCharacterMovementComponent::CanAttemptJump() const
{
	return Super::CanAttemptJump() && !bWantsToProne;
}

void UAlsCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (MovementMode == MOVE_Walking && IsSlideTriggered())
	{
		if (CanSlide(false) && GaitAmount >= MinSlideGaitAmount)
		{
			SetMovementMode(MOVE_Custom, CMOVE_Slide);
		}
	}
	else if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
	{
		SetMovementMode(MOVE_Walking);
	}

	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Cache input intents for this tick.
		const bool bRequestCrouch = bWantsToCrouch;
		const bool bRequestProne = bWantsToProne;

		// 1) Exit invalid or undesired states first.
		if (IsCrouching() && (!bRequestCrouch || !CanCrouchInCurrentState()))
		{
			UnCrouch(false);
		}
		if (IsProning() && (!bRequestProne || !CanProneInCurrentState()))
		{
			UnProne(false);
		}

		// 2) Enforce mutual exclusivity: leave the opposite state before entering the new one.
		if (bRequestProne && IsCrouching())
		{
			UnCrouch(false);
		}
		if (bRequestCrouch && IsProning())
		{
			UnProne(false);
		}

		// 3) Enter desired state. Prone has precedence if both are requested.
		if (bRequestProne && !IsProning() && CanProneInCurrentState())
		{
			Prone(false);
		}
		else if (bRequestCrouch && !IsCrouching() && CanCrouchInCurrentState())
		{
			Crouch(false);
		}
	}
}

void UAlsCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Uncrouch if no longer allowed to be crouched
		if (IsCrouching() && !CanCrouchInCurrentState())
		{
			UnCrouch(false);
		}
	}

	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		// Unprone if no longer allowed to be proned
		if (IsProning() && !CanProneInCurrentState())
		{
			UnProne(false);
		}
	}
}

bool UAlsCharacterMovementComponent::CanWalkOffLedges() const
{
	if (!Super::CanWalkOffLedges())
	{
		return false;
	}

	if (!bCanWalkOffLedgesWhenProning && IsProning())
	{
		return false;
	}

	return bCanWalkOffLedges;
}

void UAlsCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	if (!CharacterOwner)
	{
		return;
	}

	bWantsToProne = ((Flags & FSavedMove_Character::FLAG_Custom_0) != 0);
}

void UAlsCharacterMovementComponent::ControlledCharacterMove(const FVector& InputVector, const float DeltaTime)
{
	Super::ControlledCharacterMove(InputVector, DeltaTime);

	const auto* Controller{CharacterOwner->GetController()};
	if (IsValid(Controller))
	{
		PreviousControlRotation = Controller->GetControlRotation();
	}
}

void UAlsCharacterMovementComponent::PhysicsRotation(const float DeltaTime)
{
	Super::PhysicsRotation(DeltaTime);

	if (HasValidData() && (bRunPhysicsWithNoController || IsValid(CharacterOwner->GetController())))
	{
		OnPhysicsRotation.Broadcast(DeltaTime);
	}
}

void UAlsCharacterMovementComponent::MoveSmooth(const FVector& InVelocity, const float DeltaTime, FStepDownResult* StepDownResult)
{
	if (IsMovingOnGround())
	{
		RefreshGroundedMovementSettings();
	}

	Super::MoveSmooth(InVelocity, DeltaTime, StepDownResult);
}

void UAlsCharacterMovementComponent::PhysWalking(const float DeltaTime, int32 IterationsCount)
{
	RefreshGroundedMovementSettings();

	auto Iterations{IterationsCount};

	// TODO Copied with modifications from UCharacterMovementComponent::PhysWalking(). After the
	// TODO release of a new engine version, this code should be updated to match the source code.

	// ReSharper disable All

	// SCOPE_CYCLE_COUNTER(STAT_CharPhysWalking);

	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->GetController() && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	if (!UpdatedComponent->IsQueryCollisionEnabled())
	{
		SetMovementMode(MOVE_Walking);
		return;
	}

	// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN before Iteration (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = DeltaTime;

	const EMovementMode StartingMovementMode = MovementMode;
	const uint8 StartingCustomMovementMode = CustomMovementMode;

	// Perform the move
	while ( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->GetController() || bRunPhysicsWithNoController || HasAnimRootMotion() || CurrentRootMotion.HasOverrideVelocity() || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

#if UE_WITH_REMOTE_OBJECT_HANDLE
		//Scale down impact force if CharacterMoveComponent is taking multiple substeps.
		const float LastFrameDt = GetWorld()->GetDeltaSeconds();
		PhysicsForceSubsteppingFactor = timeTick / LastFrameDt;
#endif

		// Save current values
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		RestorePreAdditiveRootMotionVelocity();

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration = FVector::VectorPlaneProject(Acceleration, -GetGravityDirection());

		static const auto* EnsureAlwaysEnabledConsoleVariable{
			IConsoleManager::Get().FindConsoleVariable(TEXT("p.LedgeMovement.ApplyDirectMove"))
		};
		check(EnsureAlwaysEnabledConsoleVariable != nullptr)

		// Apply acceleration
		const bool bSkipForLedgeMove = bTriedLedgeMove && EnsureAlwaysEnabledConsoleVariable->GetBool();
		if( !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && !bSkipForLedgeMove )
		{
			CalcVelocity(timeTick, GroundFriction, false, GetMaxBrakingDeceleration());
			// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after CalcVelocity (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));
		}

		ApplyRootMotionToVelocity(timeTick);
		// devCode(ensureMsgf(!Velocity.ContainsNaN(), TEXT("PhysWalking: Velocity contains NaN after Root Motion application (%s)\n%s"), *GetPathNameSafe(this), *Velocity.ToString()));

		if (MovementMode != StartingMovementMode || CustomMovementMode != StartingCustomMovementMode)
		{
			// Root motion could have taken us out of our current mode
			// No movement has taken place this movement tick so we pass on full time/past iteration count
			StartNewPhysics(remainingTime+timeTick, Iterations-1);
			return;
		}

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if (IsSwimming()) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
			else if (MovementMode != StartingMovementMode || CustomMovementMode != StartingCustomMovementMode)
			{
				// pawn ended up in a different mode, probably due to the step-up-and-over flow
				// let's refund the estimated unused time (if any) and keep moving in the new mode
				const float DesiredDist = UE_REAL_TO_FLOAT(Delta.Size());
				if (DesiredDist > UE_KINDA_SMALL_NUMBER)
				{
					const float ActualDist = UE_REAL_TO_FLOAT(ProjectToGravityFloor(UpdatedComponent->GetComponentLocation() - OldLocation).Size());
					remainingTime += timeTick * (1.f - FMath::Min(1.f,ActualDist/DesiredDist));
				}
				StartNewPhysics(remainingTime,Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if ( bCheckLedges && !CurrentFloor.IsWalkableFloor() )
		{
			// calculate possible alternate movement
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, OldFloor);
			if ( !NewDelta.IsZero() )
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta/timeTick;
				remainingTime += timeTick;
				Iterations--;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBaseFromFloor(CurrentFloor);
			}

			// Always resolve penetration, even if the floor is walkable.

			if ((bAllowImprovedPenetrationAdjustment || !CurrentFloor.IsWalkableFloor()) &&
			    CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + MAX_FLOOR_DIST * -GetGravityDirection();
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if ( IsSwimming() )
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump) )
				{
					return;
				}
				bCheckedFall = true;
			}
		}


		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround())
		{
			// Make velocity reflect actual move
			if( !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO Start of custom ALS code block.

				PrePenetrationAdjustmentVelocity = MoveVelocity;
				bPrePenetrationAdjustmentVelocityValid = true;

				// TODO End of custom ALS code block.

				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}

	if (IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}

	// ReSharper restore All
}

void UAlsCharacterMovementComponent::PhysNavWalking(const float DeltaTime, const int32 IterationsCount)
{
	RefreshGroundedMovementSettings();

	Super::PhysNavWalking(DeltaTime, IterationsCount);
}

void UAlsCharacterMovementComponent::PhysCustom(const float DeltaTime, int32 IterationsCount)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		Super::PhysCustom(DeltaTime, IterationsCount);
		return;
	}

	Super::PhysCustom(DeltaTime, IterationsCount);

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(DeltaTime, IterationsCount);
		return;
	}

	IterationsCount += 1;
	bJustTeleported = false;

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = FVector::ZeroVector;
	}

	ApplyRootMotionToVelocity(DeltaTime);

	MoveUpdatedComponent(Velocity * DeltaTime, UpdatedComponent->GetComponentQuat(), false);
}

bool UAlsCharacterMovementComponent::ClientUpdatePositionAfterServerUpdate()
{
	SCOPE_CYCLE_COUNTER(STAT_AlsCharacterMovementClientUpdatePositionAfterServerUpdate);
	if (!HasValidData())
	{
		return false;
	}

	FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
	check(ClientData);

	if (!ClientData->bUpdatePosition)
	{
		return false;
	}

	ClientData->bUpdatePosition = false;

	// Don't do any network position updates on things running PHYS_RigidBody
	if (CharacterOwner->GetRootComponent() && CharacterOwner->GetRootComponent()->IsSimulatingPhysics())
	{
		return false;
	}

	if (ClientData->SavedMoves.Num() == 0)
	{
		UE_LOG(LogNetPlayerMovement, Verbose, TEXT("ClientUpdatePositionAfterServerUpdate No saved moves to replay"), ClientData->SavedMoves.Num());

		// With no saved moves to resimulate, the move the server updated us with is the last move we've done, no resimulation needed.
		CharacterOwner->bClientResimulateRootMotion = false;
		if (CharacterOwner->bClientResimulateRootMotionSources)
		{
			// With no resimulation, we just update our current root motion to what the server sent us
			UE_LOG(LogRootMotion, VeryVerbose, TEXT("CurrentRootMotion getting updated to ServerUpdate state: %s"), *CharacterOwner->GetName());
			CurrentRootMotion.UpdateStateFrom(CharacterOwner->SavedRootMotion);
			CharacterOwner->bClientResimulateRootMotionSources = false;
		}
		CharacterOwner->SavedRootMotion.Clear();

		return false;
	}

	// Save important values that might get affected by the replay.
	const float SavedAnalogInputModifier = AnalogInputModifier;
	const FRootMotionMovementParams BackupRootMotionParams = RootMotionParams; // For animation root motion
	const FRootMotionSourceGroup BackupRootMotion = CurrentRootMotion;
	const bool bRealPressedJump = CharacterOwner->bPressedJump;
	const float RealJumpMaxHoldTime = CharacterOwner->JumpMaxHoldTime;
	const int32 RealJumpMaxCount = CharacterOwner->JumpMaxCount;
	const bool bRealProne = bWantsToProne;
	const bool bRealForceMaxAccel = bForceMaxAccel;
	CharacterOwner->bClientWasFalling = (MovementMode == MOVE_Falling);
	CharacterOwner->bClientUpdating = true;
	bForceNextFloorCheck = true;

	// Defer all mesh child updates until all movement completes.
	FAlsScopedMeshMovementUpdate ScopedMeshUpdate(CharacterOwner->GetMesh(), /*CharacterMovementCVars::bDeferCharacterMeshMovementForAllCorrections*/ true);

	// Replay moves that have not yet been acked.
	UE_LOG(LogNetPlayerMovement, Verbose, TEXT("ClientUpdatePositionAfterServerUpdate Replaying %d Moves, starting at Timestamp %f"), ClientData->SavedMoves.Num(), ClientData->SavedMoves[0]->TimeStamp);
	for (int32 i = 0; i < ClientData->SavedMoves.Num(); i++)
	{
		FSavedMove_Character* const CurrentMove = ClientData->SavedMoves[i].Get();
		checkSlow(CurrentMove != nullptr);

		// Make current SavedMove accessible to any functions that might need it.
		SetCurrentReplayedSavedMove(CurrentMove);

		CurrentMove->PrepMoveFor(CharacterOwner);

		if (ShouldUsePackedMovementRPCs())
		{
			// Make current move data accessible to MoveAutonomous or any other functions that might need it.
			if (FCharacterNetworkMoveData* NewMove = GetNetworkMoveDataContainer().GetNewMoveData())
			{
				SetCurrentNetworkMoveData(NewMove);
				NewMove->ClientFillNetworkMoveData(*CurrentMove, FCharacterNetworkMoveData::ENetworkMoveType::NewMove);
			}
		}

		MoveAutonomous(CurrentMove->TimeStamp, CurrentMove->DeltaTime, CurrentMove->GetCompressedFlags(), CurrentMove->Acceleration);

		CurrentMove->PostUpdate(CharacterOwner, FSavedMove_Character::PostUpdate_Replay);
		SetCurrentNetworkMoveData(nullptr);
		SetCurrentReplayedSavedMove(nullptr);
	}
	const bool bPostReplayPressedJump = CharacterOwner->bPressedJump;

	if (FSavedMove_Character* const PendingMove = ClientData->PendingMove.Get())
	{
		PendingMove->bForceNoCombine = true;
	}

	// Restore saved values.
	AnalogInputModifier = SavedAnalogInputModifier;
	RootMotionParams = BackupRootMotionParams;
	CurrentRootMotion = BackupRootMotion;
	if (CharacterOwner->bClientResimulateRootMotionSources)
	{
		// If we were resimulating root motion sources, it's because we had mismatched state
		// with the server - we just resimulated our SavedMoves and now need to restore
		// CurrentRootMotion with the latest "good state"
		UE_LOG(LogRootMotion, VeryVerbose, TEXT("CurrentRootMotion getting updated after ServerUpdate replays: %s"), *CharacterOwner->GetName());
		CurrentRootMotion.UpdateStateFrom(CharacterOwner->SavedRootMotion);
		CharacterOwner->bClientResimulateRootMotionSources = false;
	}
	CharacterOwner->SavedRootMotion.Clear();
	CharacterOwner->bClientResimulateRootMotion = false;
	CharacterOwner->bClientUpdating = false;
	CharacterOwner->bPressedJump = bRealPressedJump || bPostReplayPressedJump;
	CharacterOwner->JumpMaxHoldTime = RealJumpMaxHoldTime;
	CharacterOwner->JumpMaxCount = RealJumpMaxCount;
	bWantsToProne = bRealProne;
	bForceMaxAccel = bRealForceMaxAccel;
	bForceNextFloorCheck = true;

	return (ClientData->SavedMoves.Num() > 0);
}

void UAlsCharacterMovementComponent::ComputeFloorDist(const FVector& CapsuleLocation, float LineDistance,
                                                      float SweepDistance, FFindFloorResult& OutFloorResult,
                                                      float SweepRadius, const FHitResult* DownwardSweepResult) const
{
	// TODO Copied with modifications from UCharacterMovementComponent::ComputeFloorDist().
	// TODO After the release of a new engine version, this code should be updated to match the source code.

	// ReSharper disable All

	// UE_LOG(LogCharacterMovement, VeryVerbose, TEXT("[Role:%d] ComputeFloorDist: %s at location %s"), (int32)CharacterOwner->GetLocalRole(), *GetNameSafe(CharacterOwner), *CapsuleLocation.ToString());
	OutFloorResult.Clear();

	float PawnRadius, PawnHalfHeight;
	CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);

	bool bSkipSweep = false;
	if (DownwardSweepResult != NULL && DownwardSweepResult->IsValidBlockingHit())
	{
		// Only if the supplied sweep was vertical and downward.
		const bool bIsDownward = GetGravitySpaceZ(DownwardSweepResult->TraceStart - DownwardSweepResult->TraceEnd) > 0;
		const bool bIsVertical = ProjectToGravityFloor(DownwardSweepResult->TraceStart - DownwardSweepResult->TraceEnd).SizeSquared() <= UE_KINDA_SMALL_NUMBER;
		if (bIsDownward && bIsVertical)
		{
			// Reject hits that are barely on the cusp of the radius of the capsule
			if (IsWithinEdgeTolerance(DownwardSweepResult->Location, DownwardSweepResult->ImpactPoint, PawnRadius))
			{
				// Don't try a redundant sweep, regardless of whether this sweep is usable.
				bSkipSweep = true;

				const bool bIsWalkable = IsWalkable(*DownwardSweepResult);
				const float FloorDist = UE_REAL_TO_FLOAT(GetGravitySpaceZ(CapsuleLocation - DownwardSweepResult->Location));
				OutFloorResult.SetFromSweep(*DownwardSweepResult, FloorDist, bIsWalkable);

				if (bIsWalkable)
				{
					// Use the supplied downward sweep as the floor hit result.
					return;
				}
			}
		}
	}

	// We require the sweep distance to be >= the line distance, otherwise the HitResult can't be interpreted as the sweep result.
	if (SweepDistance < LineDistance)
	{
		ensure(SweepDistance >= LineDistance);
		return;
	}

	bool bBlockingHit = false;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ComputeFloorDist), false, CharacterOwner);
	// Having a character base on a component within a cluster union will cause replication problems.
	// The issue is that ACharacter::SetBase() gets a GeometryCollectionComponent passed to it when standing on the DynamicPlatform
	// and that GC is never simulating, and since it's not simulating it's stopping the based movement flow there for simulated proxies.
	QueryParams.bTraceIntoSubComponents = true;
	QueryParams.bReplaceHitWithSubComponents = false;

	FCollisionResponseParams ResponseParam;
	InitCollisionParams(QueryParams, ResponseParam);
	const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();

	// Sweep test
	if (!bSkipSweep && SweepDistance > 0.f && SweepRadius > 0.f)
	{
		// Use a shorter height to avoid sweeps giving weird results if we start on a surface.
		// This also allows us to adjust out of penetrations.
		const float ShrinkScale = 0.9f;
		const float ShrinkScaleOverlap = 0.1f;
		float ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScale);
		float TraceDist = SweepDistance + ShrinkHeight;
		FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(SweepRadius, PawnHalfHeight - ShrinkHeight);

		FHitResult Hit(1.f);
		bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + TraceDist * GetGravityDirection(), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);

		if (bBlockingHit)
		{
			// Reject hits adjacent to us, we only care about hits on the bottom portion of our capsule.
			// Check 2D distance to impact point, reject if within a tolerance from radius.
			if (Hit.bStartPenetrating || !IsWithinEdgeTolerance(CapsuleLocation, Hit.ImpactPoint, CapsuleShape.Capsule.Radius))
			{
				// Use a capsule with a slightly smaller radius and shorter height to avoid the adjacent object.
				// Capsule must not be nearly zero or the trace will fall back to a line trace from the start point and have the wrong length.
				CapsuleShape.Capsule.Radius = FMath::Max(0.f, CapsuleShape.Capsule.Radius - SWEEP_EDGE_REJECT_DISTANCE - UE_KINDA_SMALL_NUMBER);
				if (!CapsuleShape.IsNearlyZero())
				{
					ShrinkHeight = (PawnHalfHeight - PawnRadius) * (1.f - ShrinkScaleOverlap);
					TraceDist = SweepDistance + ShrinkHeight;
					CapsuleShape.Capsule.HalfHeight = FMath::Max(PawnHalfHeight - ShrinkHeight, CapsuleShape.Capsule.Radius);
					Hit.Reset(1.f, false);

					bBlockingHit = FloorSweepTest(Hit, CapsuleLocation, CapsuleLocation + TraceDist * GetGravityDirection(), CollisionChannel, CapsuleShape, QueryParams, ResponseParam);
				}
			}

			// Reduce hit distance by ShrinkHeight because we shrank the capsule for the trace.
			// We allow negative distances here, because this allows us to pull out of penetrations.
			const float MaxPenetrationAdjust = FMath::Max(MAX_FLOOR_DIST, PawnRadius);
			const float SweepResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);

			OutFloorResult.SetFromSweep(Hit, SweepResult, false);
			if (Hit.IsValidBlockingHit() && IsWalkable(Hit))
			{
				if (SweepResult <= SweepDistance)
				{
					// Hit within test distance.
					OutFloorResult.bWalkableFloor = true;
					return;
				}
			}
		}
	}

	// Since we require a longer sweep than line trace, we don't want to run the line trace if the sweep missed everything.
	// We do however want to try a line trace if the sweep was stuck in penetration.
	if (!OutFloorResult.bBlockingHit && !OutFloorResult.HitResult.bStartPenetrating)
	{
		OutFloorResult.FloorDist = SweepDistance;
		return;
	}

	// Line trace
	if (LineDistance > 0.f)
	{
		const float ShrinkHeight = PawnHalfHeight;
		const FVector LineTraceStart = CapsuleLocation;
		const float TraceDist = LineDistance + ShrinkHeight;
		const FVector Down = TraceDist * GetGravityDirection();
		QueryParams.TraceTag = SCENE_QUERY_STAT_NAME_ONLY(FloorLineTrace);

		FHitResult Hit(1.f);
		bBlockingHit = GetWorld()->LineTraceSingleByChannel(Hit, LineTraceStart, LineTraceStart + Down, CollisionChannel, QueryParams, ResponseParam);

		if (bBlockingHit)
		{
			if (Hit.Time > 0.f)
			{
				// Reduce hit distance by ShrinkHeight because we started the trace higher than the base.
				// We allow negative distances here, because this allows us to pull out of penetrations.
				const float MaxPenetrationAdjust = FMath::Max(MAX_FLOOR_DIST, PawnRadius);
				const float LineResult = FMath::Max(-MaxPenetrationAdjust, Hit.Time * TraceDist - ShrinkHeight);

				OutFloorResult.bBlockingHit = true;
				if (LineResult <= LineDistance && IsWalkable(Hit))
				{
					// Keep the data from the previous sweep test, which will be required later to properly resolve
					// penetration in the UCharacterMovementComponent::PhysWalking() function. By default, penetration
					// is only resolved when line trace starts in penetration and the sweep test is ignored.

					const auto NormalPrevious{OutFloorResult.HitResult.Normal};
					const auto PenetrationDepthPrevious{OutFloorResult.HitResult.PenetrationDepth};
					const auto bStartPenetratingPrevious{OutFloorResult.HitResult.bStartPenetrating};
					const auto HitObjectHandlePrevious{OutFloorResult.HitResult.HitObjectHandle};

					OutFloorResult.SetFromLineTrace(Hit, OutFloorResult.FloorDist, LineResult, true);

					OutFloorResult.HitResult.Normal = NormalPrevious;
					OutFloorResult.HitResult.PenetrationDepth = PenetrationDepthPrevious;
					OutFloorResult.HitResult.bStartPenetrating = bStartPenetratingPrevious;
					OutFloorResult.HitResult.HitObjectHandle = HitObjectHandlePrevious;

					return;
				}
			}
		}
	}

	// No hits were acceptable.
	OutFloorResult.bWalkableFloor = false;

	// ReSharper restore All
}

void UAlsCharacterMovementComponent::PerformMovement(const float DeltaTime)
{
	Super::PerformMovement(DeltaTime);

	// Update the ServerLastTransformUpdateTimeStamp when the control rotation
	// changes. This is required for the view network smoothing to work properly.

	const auto* Controller{HasValidData() ? CharacterOwner->GetController() : nullptr};

	if (IsValid(Controller) && CharacterOwner->GetLocalRole() >= ROLE_Authority &&
	    PreviousControlRotation != Controller->GetControlRotation())
	{
		if (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
		{
			ServerLastTransformUpdateTimeStamp = UE_REAL_TO_FLOAT(GetPredictionData_Server_Character()->ServerAccumulatedClientTimeStamp);
		}
		else
		{
			ServerLastTransformUpdateTimeStamp = UE_REAL_TO_FLOAT(GetWorld()->GetTimeSeconds());
		}
	}
}

FNetworkPredictionData_Client* UAlsCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		auto* MutableThis{const_cast<ThisClass*>(this)};

		MutableThis->ClientPredictionData = new FAlsNetworkPredictionData{*this};
	}

	return ClientPredictionData;
}

void UAlsCharacterMovementComponent::SmoothClientPosition(const float DeltaTime)
{
	auto* PredictionData{GetPredictionData_Client_Character()};
	const auto* Mesh{HasValidData() ? CharacterOwner->GetMesh() : nullptr};

	if (PredictionData != nullptr && IsValid(Mesh) && Mesh->IsUsingAbsoluteRotation())
	{
		// Calling Super::SmoothClientPosition() will change the mesh's rotation, which is undesirable when using
		// absolute mesh rotation since we're manually updating the mesh's rotation from the animation instance. So,
		// to keep the rotation unchanged, we simply override the predicted rotations with the mesh's current rotation.

		const auto NewRotation{Mesh->GetComponentQuat() * CharacterOwner->GetBaseRotationOffset().Inverse()};

		PredictionData->OriginalMeshRotationOffset = NewRotation;
		PredictionData->MeshRotationOffset = NewRotation;
		PredictionData->MeshRotationTarget = NewRotation;
	}

	Super::SmoothClientPosition(DeltaTime);
}

void UAlsCharacterMovementComponent::MoveAutonomous(const float ClientTimeStamp, const float DeltaTime,
                                                    const uint8 CompressedFlags, const FVector& NewAcceleration)
{
	const auto* MoveData{static_cast<FAlsCharacterNetworkMoveData*>(GetCurrentNetworkMoveData())}; // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
	if (MoveData != nullptr)
	{
		RotationMode = MoveData->RotationMode;
		Stance = MoveData->Stance;
		MaxAllowedGait = MoveData->MaxAllowedGait;

		RefreshGaitSettings();
	}

	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAcceleration);

	// Process view network smoothing on the listen server.

	const auto* Controller{HasValidData() ? CharacterOwner->GetController() : nullptr};

	if (IsValid(Controller) && IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
	{
		const auto NewControlRotation{Controller->GetControlRotation()};

		auto* Character{Cast<AAlsCharacter>(CharacterOwner)};
		if (IsValid(Character))
		{
			Character->CorrectViewNetworkSmoothing(NewControlRotation, false);
		}

		PreviousControlRotation = NewControlRotation;
	}
}

void UAlsCharacterMovementComponent::SetMovementSettings(UAlsMovementSettings* NewMovementSettings)
{
	ALS_ENSURE(IsValid(NewMovementSettings));

	MovementSettings = NewMovementSettings;

	RefreshGaitSettings();
}

bool UAlsCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

void UAlsCharacterMovementComponent::RefreshGaitSettings()
{
	if (!ALS_ENSURE(IsValid(MovementSettings)))
	{
		return;
	}

	const auto* StanceSettings{MovementSettings->RotationModes.Find(RotationMode)};
	const auto* NewGaitSettings{ALS_ENSURE(StanceSettings != nullptr) ? StanceSettings->Stances.Find(Stance) : nullptr};

	GaitSettings = ALS_ENSURE(NewGaitSettings != nullptr) ? *NewGaitSettings : FAlsMovementGaitSettings{};
}

void UAlsCharacterMovementComponent::SetRotationMode(const FGameplayTag& NewRotationMode)
{
	if (RotationMode != NewRotationMode)
	{
		RotationMode = NewRotationMode;

		RefreshGaitSettings();
	}
}

void UAlsCharacterMovementComponent::SetStance(const FGameplayTag& NewStance)
{
	if (Stance != NewStance)
	{
		Stance = NewStance;

		RefreshGaitSettings();
	}
}

void UAlsCharacterMovementComponent::RefreshGroundedMovementSettings()
{
	auto WalkSpeed{GaitSettings.WalkForwardSpeed};
	auto RunSpeed{GaitSettings.RunForwardSpeed};

	if (GaitSettings.bAllowDirectionDependentMovementSpeed &&
	    Velocity.SizeSquared() > UE_KINDA_SMALL_NUMBER &&
	    IsValid(MovementSettings))
	{
		const auto* Controller{GetController()};

		const auto ViewRotation{
			IsValid(Controller)
				? GetController()->GetControlRotation()
				: GetCharacterOwner()->GetViewRotation()
		};

		// Ideally we should use actor rotation here instead of view rotation, but we can't do that because ALS has
		// full control over actor rotation and it is not synchronized over the network, so it would cause jitter.

		const auto RelativeViewRotation{UAlsRotation::GetTwist(ViewRotation.Quaternion(), -GetGravityDirection())};

		const FVector2D RelativeVelocity{RelativeViewRotation.UnrotateVector(Velocity)};
		const auto VelocityAngle{UAlsVector::DirectionToAngle(RelativeVelocity)};

		const auto ForwardSpeedAmount{
			1.0f - UAlsMath::Clamp01(MovementSettings->VelocityAngleToSpeedInterpolationRange
			                                         .GetRangePct(static_cast<float>(FMath::Abs(VelocityAngle))))
		};

		WalkSpeed = FMath::Lerp(GaitSettings.WalkBackwardSpeed, GaitSettings.WalkForwardSpeed, ForwardSpeedAmount);
		RunSpeed = FMath::Lerp(GaitSettings.RunBackwardSpeed, GaitSettings.RunForwardSpeed, ForwardSpeedAmount);
	}

	// Map the character's current speed to the to the speed ranges from the movement settings. This allows
	// us to vary movement speeds but still use the mapped range in calculations for consistent results.

	const auto Speed{UE_REAL_TO_FLOAT(Velocity.Size2D())};

	if (Speed > RunSpeed)
	{
		GaitAmount = FMath::GetMappedRangeValueClamped(FVector2f{RunSpeed, GaitSettings.SprintSpeed}, {2.0f, 3.0f}, Speed);
	}
	else if (Speed > WalkSpeed)
	{
		GaitAmount = FMath::GetMappedRangeValueClamped(FVector2f{WalkSpeed, RunSpeed}, {1.0f, 2.0f}, Speed);
	}
	else
	{
		GaitAmount = FMath::GetMappedRangeValueClamped(FVector2f{0.0f, WalkSpeed}, {0.0f, 1.0f}, Speed);
	}

	if (MaxAllowedGait == AlsGaitTags::Walking)
	{
		MaxWalkSpeed = WalkSpeed;
	}
	else if (MaxAllowedGait == AlsGaitTags::Running)
	{
		MaxWalkSpeed = RunSpeed;
	}
	else if (MaxAllowedGait == AlsGaitTags::Sprinting)
	{
		MaxWalkSpeed = GaitSettings.SprintSpeed;
	}
	else
	{
		MaxWalkSpeed = GaitSettings.RunForwardSpeed;
	}

	MaxWalkSpeedCrouched = MaxWalkSpeed;

	// Get acceleration, deceleration and ground friction using a curve. This
	// allows us to precisely control the movement behavior at each speed.

	if (ALS_ENSURE(IsValid(GaitSettings.AccelerationAndDecelerationAndGroundFrictionCurve)))
	{
		const auto& AccelerationAndDecelerationAndGroundFrictionCurves{
			GaitSettings.AccelerationAndDecelerationAndGroundFrictionCurve->FloatCurves
		};

		MaxAccelerationWalking = AccelerationAndDecelerationAndGroundFrictionCurves[0].Eval(GaitAmount);
		BrakingDecelerationWalking = AccelerationAndDecelerationAndGroundFrictionCurves[1].Eval(GaitAmount);
		GroundFriction = AccelerationAndDecelerationAndGroundFrictionCurves[2].Eval(GaitAmount);
	}
}

void UAlsCharacterMovementComponent::SetMovementModeLocked(const bool bNewMovementModeLocked)
{
	bMovementModeLocked = bNewMovementModeLocked;
}

void UAlsCharacterMovementComponent::SetInputBlocked(const bool bNewInputBlocked)
{
	bInputBlocked = bNewInputBlocked;
}

bool UAlsCharacterMovementComponent::TryConsumePrePenetrationAdjustmentVelocity(FVector& OutVelocity)
{
	if (!bPrePenetrationAdjustmentVelocityValid)
	{
		OutVelocity = FVector::ZeroVector;
		return false;
	}

	OutVelocity = PrePenetrationAdjustmentVelocity;

	PrePenetrationAdjustmentVelocity = FVector::ZeroVector;
	bPrePenetrationAdjustmentVelocityValid = false;

	return true;
}

void UAlsCharacterMovementComponent::Prone(bool bClientSimulation /*= false*/)
{
	if (!HasValidData())
	{
		return;
	}

	if (!bClientSimulation && !CanProneInCurrentState())
	{
		return;
	}

	AAlsCharacter* Character = Cast<AAlsCharacter>(CharacterOwner);
	if (!IsValid(Character))
	{
		return;
	}

	// See if collision is already at desired size.
	if (Character->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == PronedHalfHeight)
	{
		if (!bClientSimulation)
		{
			Character->SetIsProned(true);
		}
		Character->OnStartProne(0.f, 0.f);
		return;
	}

	if (bClientSimulation && Character->GetLocalRole() == ROLE_SimulatedProxy)
	{
		// restore collision size before proning
		AAlsCharacter* DefaultCharacter = Character->GetClass()->GetDefaultObject<AAlsCharacter>();
		Character->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
		bShrinkProxyCapsule = true;
	}

	// Change collision size to proning dimensions
	const float ComponentScale = Character->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = Character->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = Character->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	const float ClampedPronedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, PronedHalfHeight);
	Character->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedPronedHalfHeight);
	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedPronedHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	if (!bClientSimulation)
	{
		// Proning to a larger height? (this is rare)
		if (ClampedPronedHalfHeight > OldUnscaledHalfHeight)
		{
			FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, Character);
			FCollisionResponseParams ResponseParam;
			InitCollisionParams(CapsuleParams, ResponseParam);
			const bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(UpdatedComponent->GetComponentLocation() + ScaledHalfHeightAdjust * GetGravityDirection(), GetWorldToGravityTransform(),
				UpdatedComponent->GetCollisionObjectType(), GetPawnCapsuleCollisionShape(SHRINK_None), CapsuleParams, ResponseParam);

			// If encroached, cancel
			if (bEncroached)
			{
				Character->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, OldUnscaledHalfHeight);
				return;
			}
		}

		if (bProneMaintainsBaseLocation)
		{
			// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
			UpdatedComponent->MoveComponent(ScaledHalfHeightAdjust * GetGravityDirection(), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
		}

		Character->SetIsProned(true);
	}

	bForceNextFloorCheck = true;

	// OnStartProne takes the change from the Default size, not the current one (though they are usually the same).
	const float MeshAdjust = ScaledHalfHeightAdjust;
	AAlsCharacter* DefaultCharacter = Character->GetClass()->GetDefaultObject<AAlsCharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - ClampedPronedHalfHeight);
	ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	AdjustProxyCapsuleSize();
	Character->OnStartProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// Don't smooth this change in mesh position
	if ((bClientSimulation && Character->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) && Character->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset -= MeshAdjust * -GetGravityDirection();
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

void UAlsCharacterMovementComponent::UnProne(bool bClientSimulation /*= false*/)
{
	if (!HasValidData())
	{
		return;
	}

	AAlsCharacter* Character = Cast<AAlsCharacter>(CharacterOwner);
	if (!IsValid(Character))
	{
		return;
	}

	AAlsCharacter* DefaultCharacter = Character->GetClass()->GetDefaultObject<AAlsCharacter>();

	// See if collision is already at desired size.
	if (Character->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight())
	{
		if (!bClientSimulation)
		{
			Character->SetIsProned(false);
		}
		Character->OnEndProne(0.f, 0.f);
		return;
	}

	const float CurrentPronedHalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float ComponentScale = Character->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = Character->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - OldUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;
	const FVector PawnLocation = UpdatedComponent->GetComponentLocation();

	// Grow to unproned size.
	check(Character->GetCapsuleComponent());

	if (!bClientSimulation)
	{
		// Try to stay in place and see if the larger capsule fits. We use a slightly taller capsule to avoid penetration.
		const UWorld* MyWorld = GetWorld();
		const float SweepInflation = UE_KINDA_SMALL_NUMBER * 10.f;
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(ProneTrace), false, Character);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);

		// Compensate for the difference between current capsule size and standing size
		const FCollisionShape StandingCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, -SweepInflation - ScaledHalfHeightAdjust); // Shrink by negative amount, so actually grow it.
		const ECollisionChannel CollisionChannel = UpdatedComponent->GetCollisionObjectType();
		bool bEncroached = true;

		if (!bProneMaintainsBaseLocation)
		{
			// Expand in place
			bEncroached = MyWorld->OverlapBlockingTestByChannel(PawnLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				// Try adjusting capsule position to see if we can avoid encroachment.
				if (ScaledHalfHeightAdjust > 0.f)
				{
					// Shrink to a short capsule, sweep down to base to find where that would hit something, and then try to stand up from there.
					float PawnRadius, PawnHalfHeight;
					Character->GetCapsuleComponent()->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
					const float ShrinkHalfHeight = PawnHalfHeight - PawnRadius;
					const float TraceDist = PawnHalfHeight - ShrinkHalfHeight;
					const FVector Down = TraceDist * GetGravityDirection();

					FHitResult Hit(1.f);
					const FCollisionShape ShortCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom, ShrinkHalfHeight);
					const bool bBlockingHit = MyWorld->SweepSingleByChannel(Hit, PawnLocation, PawnLocation + Down, GetWorldToGravityTransform(), CollisionChannel, ShortCapsuleShape, CapsuleParams);
					if (Hit.bStartPenetrating)
					{
						bEncroached = true;
					}
					else
					{
						// Compute where the base of the sweep ended up, and see if we can stand there
						const float DistanceToBase = (Hit.Time * TraceDist) + ShortCapsuleShape.Capsule.HalfHeight;
						const FVector Adjustment = (-DistanceToBase + StandingCapsuleShape.Capsule.HalfHeight + SweepInflation + MIN_FLOOR_DIST / 2.f) * -GetGravityDirection();
						const FVector NewLoc = PawnLocation + Adjustment;
						bEncroached = MyWorld->OverlapBlockingTestByChannel(NewLoc, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
						if (!bEncroached)
						{
							// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
							UpdatedComponent->MoveComponent(NewLoc - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
						}
					}
				}
			}
		}
		else
		{
			// Expand while keeping base location the same.
			FVector StandingLocation = PawnLocation + (StandingCapsuleShape.GetCapsuleHalfHeight() - CurrentPronedHalfHeight) * -GetGravityDirection();
			bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);

			if (bEncroached)
			{
				if (IsMovingOnGround())
				{
					// Something might be just barely overhead, try moving down closer to the floor to avoid it.
					const float MinFloorDist = UE_KINDA_SMALL_NUMBER * 10.f;
					if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
					{
						StandingLocation -= (CurrentFloor.FloorDist - MinFloorDist) * -GetGravityDirection();
						bEncroached = MyWorld->OverlapBlockingTestByChannel(StandingLocation, GetWorldToGravityTransform(), CollisionChannel, StandingCapsuleShape, CapsuleParams, ResponseParam);
					}
				}
			}

			if (!bEncroached)
			{
				// Commit the change in location.
				UpdatedComponent->MoveComponent(StandingLocation - PawnLocation, UpdatedComponent->GetComponentQuat(), false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
				bForceNextFloorCheck = true;
			}
		}

		// If still encroached then abort.
		if (bEncroached)
		{
			return;
		}

		Character->SetIsProned(false);
	}
	else
	{
		bShrinkProxyCapsule = true;
	}

	// Now call SetCapsuleSize() to cause touch/untouch events and actually grow the capsule
	Character->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), true);

	const float MeshAdjust = ScaledHalfHeightAdjust;
	AdjustProxyCapsuleSize();
	Character->OnEndProne(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// Don't smooth this change in mesh position
	if ((bClientSimulation && Character->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) && Character->GetRemoteRole() == ROLE_AutonomousProxy))
	{
		FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
		if (ClientData)
		{
			ClientData->MeshTranslationOffset += MeshAdjust * -GetGravityDirection();
			ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
		}
	}
}

bool UAlsCharacterMovementComponent::CanProneInCurrentState() const
{
#if 0
	if (!CanEverProne())
	{
		return false;
	}
#endif

	return (IsFalling() || IsMovingOnGround()) && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

void UAlsCharacterMovementComponent::SetPronedHalfHeight(const float NewValue)
{
	PronedHalfHeight = NewValue;

	auto* Character{ Cast<AAlsCharacter>(CharacterOwner) };
	if (IsValid(Character))
	{
		Character->RecalculatePronedEyeHeight();
	}
}

float UAlsCharacterMovementComponent::GetPronedHalfHeight() const
{
	return PronedHalfHeight;
}

bool UAlsCharacterMovementComponent::IsProning() const
{
	AAlsCharacter* Character = Cast<AAlsCharacter>(CharacterOwner);
	if (IsValid(Character))
	{
		return Character->IsProned();
	}

	return false;
}

bool UAlsCharacterMovementComponent::IsSlideTriggered() const
{
	if (SlideTriggerType == ESlideTriggerType::ESTT_DoubleTap)
	{
		return !bWantsToCrouch && Safe_bPrevWantsToCrouch;
	}

	if (SlideTriggerType == ESlideTriggerType::ESTT_SingleTap)
	{
		return bWantsToCrouch && !Safe_bPrevWantsToCrouch;
	}

	return false;
}

void UAlsCharacterMovementComponent::EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode)
{
	bWantsToCrouch = true;
	//bOrientRotationToMovement = false;
	
	Velocity += Velocity.GetSafeNormal2D() * SlideEnterImpulse;

	FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, true, NULL);
}

void UAlsCharacterMovementComponent::ExitSlide()
{
	bWantsToCrouch = false;
	// bOrientRotationToMovement = true;
}

bool UAlsCharacterMovementComponent::CanSlide(bool bCheckSpeed /*= true*/) const
{
	const FVector& Start = UpdatedComponent->GetComponentLocation();
	const FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.5f * FVector::DownVector;

	FCollisionQueryParams QueryParameters(__FUNCTION__, false, CharacterOwner);

	bool bValidSurface = GetWorld()->LineTraceTestByProfile(Start, End, TEXT("BlockAll"), QueryParameters);
	bool bEnoughSpeed = !bCheckSpeed || Velocity.SizeSquared() > pow(MinSlideSpeed, 2);

	return bValidSurface && bEnoughSpeed;
}

void UAlsCharacterMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}


	if (!CanSlide())
	{
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent* const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;

		FVector SlopeForce = CurrentFloor.HitResult.Normal;
		SlopeForce.Z = 0.f;
		Velocity += SlopeForce * SlideGravityForce * deltaTime;

		// Handle player input during slide with customizable strength
		if (!Acceleration.IsZero())
		{
			FVector InputAcceleration = FVector::ZeroVector;

			// Calculate lateral (left/right) input influence
			float LateralInput = FVector::DotProduct(Acceleration.GetSafeNormal2D(), UpdatedComponent->GetRightVector());
			InputAcceleration += UpdatedComponent->GetRightVector() * LateralInput * SlideSteeringStrength;

			// Optionally allow forward/backward input influence
			if (bAllowForwardInputDuringSlide)
			{
				float ForwardInput = FVector::DotProduct(Acceleration.GetSafeNormal2D(), UpdatedComponent->GetForwardVector());
				InputAcceleration += UpdatedComponent->GetForwardVector() * ForwardInput * SlideForwardInputStrength;
			}

			// Scale by the original acceleration magnitude and apply
			Acceleration = InputAcceleration * Acceleration.Size();
		}

		// Apply acceleration
		CalcVelocity(timeTick, GroundFriction * SlideFrictionFactor, false, GetMaxBrakingDeceleration());

#if 0
		// Clamp sliding speed to max
		if (Velocity.Size2D() > MaxSlideSpeed)
		{
			Velocity = Velocity.GetSafeNormal2D() * MaxSlideSpeed + FVector(0, 0, Velocity.Z);
		}
#endif

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;
		bool bFloorWalkable = CurrentFloor.IsWalkableFloor();

		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if (IsFalling())
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
			else if (IsSwimming()) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}


		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if (bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{
			// calculate possible alternate movement
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, OldFloor);
			if (!NewDelta.IsZero())
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if (IsSwimming())
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;
			}
		}

		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround() && bFloorWalkable)
		{
			// Make velocity reflect actual move
			if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}


	FHitResult Hit;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(Velocity.GetSafeNormal2D(), FVector::UpVector).ToQuat();
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
}
