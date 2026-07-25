#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "AlsConstants.generated.h"

UCLASS(Meta = (BlueprintThreadSafe))
class ALS_API UAlsConstants : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

protected:
	// Bones

	inline static FName RootBone{ANSITEXTVIEW("root")};
	inline static FName PelvisBone{ANSITEXTVIEW("pelvis")};
	inline static FName HeadBone{ANSITEXTVIEW("head")};
	inline static FName Spine03Bone{ANSITEXTVIEW("spine_03")};
	inline static FName FootLeftBone{ANSITEXTVIEW("foot_l")};
	inline static FName FootRightBone{ANSITEXTVIEW("foot_r")};
	inline static FName HandLeftGunVirtualBone{ANSITEXTVIEW("VB hand_l_to_ik_hand_gun")};
	inline static FName HandRightGunVirtualBone{ANSITEXTVIEW("VB hand_r_to_ik_hand_gun")};
	inline static FName FootLeftIkBone{ANSITEXTVIEW("ik_foot_l")};
	inline static FName FootRightIkBone{ANSITEXTVIEW("ik_foot_r")};
	inline static FName FootLeftVirtualBone{ANSITEXTVIEW("VB foot_l")};
	inline static FName FootRightVirtualBone{ANSITEXTVIEW("VB foot_r")};

	// Animation Slots

	inline static FName TransitionSlot{ANSITEXTVIEW("Transition")};
	inline static FName TurnInPlaceStandingSlot{ANSITEXTVIEW("TurnInPlaceStanding")};
	inline static FName TurnInPlaceCrouchingSlot{ANSITEXTVIEW("TurnInPlaceCrouching")};
	inline static FName TurnInPlaceProningSlot{ANSITEXTVIEW("TurnInPlaceProning")}; // Fork: turn in place agachado/deitado.

	// Layering Animation Curves

	inline static FName LayerHeadCurve{ANSITEXTVIEW("LayerHead")};
	inline static FName LayerHeadAdditiveCurve{ANSITEXTVIEW("LayerHeadAdditive")};
	inline static FName LayerHeadSlotCurve{ANSITEXTVIEW("LayerHeadSlot")};
	inline static FName LayerArmLeftCurve{ANSITEXTVIEW("LayerArmLeft")};
	inline static FName LayerArmLeftAdditiveCurve{ANSITEXTVIEW("LayerArmLeftAdditive")};
	inline static FName LayerArmLeftLocalSpaceCurve{ANSITEXTVIEW("LayerArmLeftLocalSpace")};
	inline static FName LayerArmLeftSlotCurve{ANSITEXTVIEW("LayerArmLeftSlot")};
	inline static FName LayerArmRightCurve{ANSITEXTVIEW("LayerArmRight")};
	inline static FName LayerArmRightAdditiveCurve{ANSITEXTVIEW("LayerArmRightAdditive")};
	inline static FName LayerArmRightLocalSpaceCurve{ANSITEXTVIEW("LayerArmRightLocalSpace")};
	inline static FName LayerArmRightSlotCurve{ANSITEXTVIEW("LayerArmRightSlot")};
	inline static FName LayerHandLeftCurve{ANSITEXTVIEW("LayerHandLeft")};
	inline static FName LayerHandRightCurve{ANSITEXTVIEW("LayerHandRight")};
	inline static FName LayerSpineCurve{ANSITEXTVIEW("LayerSpine")};
	inline static FName LayerSpineAdditiveCurve{ANSITEXTVIEW("LayerSpineAdditive")};
	inline static FName LayerSpineSlotCurve{ANSITEXTVIEW("LayerSpineSlot")};
	inline static FName LayerPelvisCurve{ANSITEXTVIEW("LayerPelvis")};
	inline static FName LayerPelvisSlotCurve{ANSITEXTVIEW("LayerPelvisSlot")};
	inline static FName LayerLegsCurve{ANSITEXTVIEW("LayerLegs")};
	inline static FName LayerLegsSlotCurve{ANSITEXTVIEW("LayerLegsSlot")};
	inline static FName HandLeftIkCurve{ANSITEXTVIEW("HandLeftIk")};
	inline static FName HandRightIkCurve{ANSITEXTVIEW("HandRightIk")};
	inline static FName ViewBlockCurve{ANSITEXTVIEW("ViewBlock")};
	inline static FName AllowPitchAimingCurve{ANSITEXTVIEW("AllowPitchAiming")}; // Fork: gate do pitch da spine no aiming.
	inline static FName HipsDirectionLockCurve{ANSITEXTVIEW("HipsDirectionLock")};

	// Pose Animation Curves

	inline static FName PoseGaitCurve{ANSITEXTVIEW("PoseGait")};
	inline static FName PoseMovingCurve{ANSITEXTVIEW("PoseMoving")};
	inline static FName PoseStandingCurve{ANSITEXTVIEW("PoseStanding")};
	inline static FName PoseCrouchingCurve{ANSITEXTVIEW("PoseCrouching")};
	inline static FName PoseProningCurve{ANSITEXTVIEW("PoseProning")}; // Fork: peso da pose de prone.
	inline static FName PoseGroundedCurve{ANSITEXTVIEW("PoseGrounded")};
	inline static FName PoseInAirCurve{ANSITEXTVIEW("PoseInAir")};
	inline static FName PoseAimingCurve{ANSITEXTVIEW("PoseAiming")};

	// Feet Animation Curves

	inline static FName FootLeftIkCurve{ANSITEXTVIEW("FootLeftIk")};
	inline static FName FootLeftLockCurve{ANSITEXTVIEW("FootLeftLock")};
	inline static FName FootRightIkCurve{ANSITEXTVIEW("FootRightIk")};
	inline static FName FootRightLockCurve{ANSITEXTVIEW("FootRightLock")};
	inline static FName FootPlantedCurve{ANSITEXTVIEW("FootPlanted")};
	inline static FName FeetCrossingCurve{ANSITEXTVIEW("FeetCrossing")};

	// Other Animation Curves

	inline static FName RotationYawSpeedCurve{ANSITEXTVIEW("RotationYawSpeed")};
	inline static FName RotationYawOffsetCurve{ANSITEXTVIEW("RotationYawOffset")};
	inline static FName AllowTransitionsCurve{ANSITEXTVIEW("AllowTransitions")};
	inline static FName SprintBlockCurve{ANSITEXTVIEW("SprintBlock")};
	inline static FName GroundPredictionBlockCurve{ANSITEXTVIEW("GroundPredictionBlock")};
	inline static FName FootstepSoundBlockCurve{ANSITEXTVIEW("FootstepSoundBlock")};

	// Debug

	inline static FName CurvesDebugDisplay{ANSITEXTVIEW("ALS.Curves")};
	inline static FName StateDebugDisplay{ANSITEXTVIEW("ALS.State")};
	inline static FName ShapesDebugDisplay{ANSITEXTVIEW("ALS.Shapes")};
	inline static FName TracesDebugDisplay{ANSITEXTVIEW("ALS.Traces")};
	inline static FName MantlingDebugDisplay{ANSITEXTVIEW("ALS.Mantling")};

public:
	// Bones

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName RootBoneName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName PelvisBoneName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName HeadBoneName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName Spine03BoneName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName FootLeftBoneName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName FootRightBoneName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName HandLeftGunVirtualBoneName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName HandRightGunVirtualBoneName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName FootLeftIkBoneName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName FootRightIkBoneName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName FootLeftVirtualBoneName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Bones", Meta = (ReturnDisplayName = "Bone Name"))
	static FName FootRightVirtualBoneName();

	// Animation Slots

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Slots", Meta = (ReturnDisplayName = "Slot Name"))
	static FName TransitionSlotName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Slots", Meta = (ReturnDisplayName = "Slot Name"))
	static FName TurnInPlaceStandingSlotName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Slots", Meta = (ReturnDisplayName = "Slot Name"))
	static FName TurnInPlaceCrouchingSlotName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Slots", Meta = (ReturnDisplayName = "Slot Name"))
	static FName TurnInPlaceProningSlotName();

	// Layering Animation Curves

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerHeadCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerHeadAdditiveCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerHeadSlotCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerArmLeftCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerArmLeftAdditiveCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerArmLeftLocalSpaceCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerArmLeftSlotCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerArmRightCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerArmRightAdditiveCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerArmRightLocalSpaceCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerArmRightSlotCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerHandLeftCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerHandRightCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerSpineCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerSpineAdditiveCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerSpineSlotCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerPelvisCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerPelvisSlotCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerLegsCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName LayerLegsSlotCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName HandLeftIkCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName HandRightIkCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName ViewBlockCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName AllowPitchAimingCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName HipsDirectionLockCurveName();

	// Pose Animation Curves

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName PoseGaitCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName PoseMovingCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName PoseStandingCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName PoseCrouchingCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName PoseProningCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName PoseGroundedCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName PoseInAirCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName PoseAimingCurveName();

	// Feet Animation Curves

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName FootLeftIkCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName FootLeftLockCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName FootRightIkCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName FootRightLockCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName FootPlantedCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName FeetCrossingCurveName();

	// Other Animation Curves

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName RotationYawSpeedCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName RotationYawOffsetCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName AllowTransitionsCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName SprintBlockCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName GroundPredictionBlockCurveName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Animation Curves", Meta = (ReturnDisplayName = "Curve Name"))
	static FName FootstepSoundBlockCurveName();

	// Debug

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static FName CurvesDebugDisplayName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static FName StateDebugDisplayName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static FName ShapesDebugDisplayName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static FName TracesDebugDisplayName();

	UFUNCTION(BlueprintPure, Category = "ALS|Constants|Debug", Meta = (ReturnDisplayName = "Display Name"))
	static FName MantlingDebugDisplayName();
};

inline FName UAlsConstants::RootBoneName()
{
	return RootBone;
}

inline FName UAlsConstants::PelvisBoneName()
{
	return PelvisBone;
}

inline FName UAlsConstants::HeadBoneName()
{
	return HeadBone;
}

inline FName UAlsConstants::Spine03BoneName()
{
	return Spine03Bone;
}

inline FName UAlsConstants::FootLeftBoneName()
{
	return FootLeftBone;
}

inline FName UAlsConstants::FootRightBoneName()
{
	return FootRightBone;
}

inline FName UAlsConstants::HandLeftGunVirtualBoneName()
{
	return HandLeftGunVirtualBone;
}

inline FName UAlsConstants::HandRightGunVirtualBoneName()
{
	return HandRightGunVirtualBone;
}

inline FName UAlsConstants::FootLeftIkBoneName()
{
	return FootLeftIkBone;
}

inline FName UAlsConstants::FootRightIkBoneName()
{
	return FootRightIkBone;
}

inline FName UAlsConstants::FootLeftVirtualBoneName()
{
	return FootLeftVirtualBone;
}

inline FName UAlsConstants::FootRightVirtualBoneName()
{
	return FootRightVirtualBone;
}

inline FName UAlsConstants::TransitionSlotName()
{
	return TransitionSlot;
}

inline FName UAlsConstants::TurnInPlaceStandingSlotName()
{
	return TurnInPlaceStandingSlot;
}

inline FName UAlsConstants::TurnInPlaceCrouchingSlotName()
{
	return TurnInPlaceCrouchingSlot;
}

inline FName UAlsConstants::TurnInPlaceProningSlotName()
{
	return TurnInPlaceProningSlot;
}

inline FName UAlsConstants::LayerHeadCurveName()
{
	return LayerHeadCurve;
}

inline FName UAlsConstants::LayerHeadAdditiveCurveName()
{
	return LayerHeadAdditiveCurve;
}

inline FName UAlsConstants::LayerHeadSlotCurveName()
{
	return LayerHeadSlotCurve;
}

inline FName UAlsConstants::LayerArmLeftCurveName()
{
	return LayerArmLeftCurve;
}

inline FName UAlsConstants::LayerArmLeftAdditiveCurveName()
{
	return LayerArmLeftAdditiveCurve;
}

inline FName UAlsConstants::LayerArmLeftLocalSpaceCurveName()
{
	return LayerArmLeftLocalSpaceCurve;
}

inline FName UAlsConstants::LayerArmLeftSlotCurveName()
{
	return LayerArmLeftSlotCurve;
}

inline FName UAlsConstants::LayerArmRightCurveName()
{
	return LayerArmRightCurve;
}

inline FName UAlsConstants::LayerArmRightAdditiveCurveName()
{
	return LayerArmRightAdditiveCurve;
}

inline FName UAlsConstants::LayerArmRightLocalSpaceCurveName()
{
	return LayerArmRightLocalSpaceCurve;
}

inline FName UAlsConstants::LayerArmRightSlotCurveName()
{
	return LayerArmRightSlotCurve;
}

inline FName UAlsConstants::LayerHandLeftCurveName()
{
	return LayerHandLeftCurve;
}

inline FName UAlsConstants::LayerHandRightCurveName()
{
	return LayerHandRightCurve;
}

inline FName UAlsConstants::LayerSpineCurveName()
{
	return LayerSpineCurve;
}

inline FName UAlsConstants::LayerSpineAdditiveCurveName()
{
	return LayerSpineAdditiveCurve;
}

inline FName UAlsConstants::LayerSpineSlotCurveName()
{
	return LayerSpineSlotCurve;
}

inline FName UAlsConstants::LayerPelvisCurveName()
{
	return LayerPelvisCurve;
}

inline FName UAlsConstants::LayerPelvisSlotCurveName()
{
	return LayerPelvisSlotCurve;
}

inline FName UAlsConstants::LayerLegsCurveName()
{
	return LayerLegsCurve;
}

inline FName UAlsConstants::LayerLegsSlotCurveName()
{
	return LayerLegsSlotCurve;
}

inline FName UAlsConstants::HandLeftIkCurveName()
{
	return HandLeftIkCurve;
}

inline FName UAlsConstants::HandRightIkCurveName()
{
	return HandRightIkCurve;
}

inline FName UAlsConstants::ViewBlockCurveName()
{
	return ViewBlockCurve;
}

inline FName UAlsConstants::HipsDirectionLockCurveName()
{
	return HipsDirectionLockCurve;
}

inline FName UAlsConstants::AllowPitchAimingCurveName()
{
	return AllowPitchAimingCurve;
}

inline FName UAlsConstants::PoseGaitCurveName()
{
	return PoseGaitCurve;
}

inline FName UAlsConstants::PoseMovingCurveName()
{
	return PoseMovingCurve;
}

inline FName UAlsConstants::PoseStandingCurveName()
{
	return PoseStandingCurve;
}

inline FName UAlsConstants::PoseCrouchingCurveName()
{
	return PoseCrouchingCurve;
}

inline FName UAlsConstants::PoseProningCurveName()
{
	return PoseProningCurve;
}

inline FName UAlsConstants::PoseGroundedCurveName()
{
	return PoseGroundedCurve;
}

inline FName UAlsConstants::PoseInAirCurveName()
{
	return PoseInAirCurve;
}

inline FName UAlsConstants::PoseAimingCurveName()
{
	return PoseAimingCurve;
}

inline FName UAlsConstants::FootLeftIkCurveName()
{
	return FootLeftIkCurve;
}

inline FName UAlsConstants::FootLeftLockCurveName()
{
	return FootLeftLockCurve;
}

inline FName UAlsConstants::FootRightIkCurveName()
{
	return FootRightIkCurve;
}

inline FName UAlsConstants::FootRightLockCurveName()
{
	return FootRightLockCurve;
}

inline FName UAlsConstants::FootPlantedCurveName()
{
	return FootPlantedCurve;
}

inline FName UAlsConstants::FeetCrossingCurveName()
{
	return FeetCrossingCurve;
}

inline FName UAlsConstants::RotationYawSpeedCurveName()
{
	return RotationYawSpeedCurve;
}

inline FName UAlsConstants::RotationYawOffsetCurveName()
{
	return RotationYawOffsetCurve;
}

inline FName UAlsConstants::AllowTransitionsCurveName()
{
	return AllowTransitionsCurve;
}

inline FName UAlsConstants::SprintBlockCurveName()
{
	return SprintBlockCurve;
}

inline FName UAlsConstants::GroundPredictionBlockCurveName()
{
	return GroundPredictionBlockCurve;
}

inline FName UAlsConstants::FootstepSoundBlockCurveName()
{
	return FootstepSoundBlockCurve;
}

inline FName UAlsConstants::CurvesDebugDisplayName()
{
	return CurvesDebugDisplay;
}

inline FName UAlsConstants::StateDebugDisplayName()
{
	return StateDebugDisplay;
}

inline FName UAlsConstants::ShapesDebugDisplayName()
{
	return ShapesDebugDisplay;
}

inline FName UAlsConstants::TracesDebugDisplayName()
{
	return TracesDebugDisplay;
}

inline FName UAlsConstants::MantlingDebugDisplayName()
{
	return MantlingDebugDisplay;
}
