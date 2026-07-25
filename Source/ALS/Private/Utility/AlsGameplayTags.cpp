#include "Utility/AlsGameplayTags.h"

namespace AlsViewModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(FirstPerson, FName{ANSITEXTVIEW("Als.ViewMode.FirstPerson")})
	UE_DEFINE_GAMEPLAY_TAG(ThirdPerson, FName{ANSITEXTVIEW("Als.ViewMode.ThirdPerson")})
}

namespace AlsLocomotionModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Grounded, FName{ANSITEXTVIEW("Als.LocomotionMode.Grounded")})
	UE_DEFINE_GAMEPLAY_TAG(InAir, FName{ANSITEXTVIEW("Als.LocomotionMode.InAir")})
}

namespace AlsRotationModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(VelocityDirection, FName{ANSITEXTVIEW("Als.RotationMode.VelocityDirection")})
	UE_DEFINE_GAMEPLAY_TAG(ViewDirection, FName{ANSITEXTVIEW("Als.RotationMode.ViewDirection")})
	UE_DEFINE_GAMEPLAY_TAG(Aiming, FName{ANSITEXTVIEW("Als.RotationMode.Aiming")})
}

namespace AlsStanceTags
{
	UE_DEFINE_GAMEPLAY_TAG(Standing, FName{ANSITEXTVIEW("Als.Stance.Standing")})
	UE_DEFINE_GAMEPLAY_TAG(Crouching, FName{ANSITEXTVIEW("Als.Stance.Crouching")})
	UE_DEFINE_GAMEPLAY_TAG(Proning, FName{ANSITEXTVIEW("Als.Stance.Proning")})
}

namespace AlsGaitTags
{
	UE_DEFINE_GAMEPLAY_TAG(Walking, FName{ANSITEXTVIEW("Als.Gait.Walking")})
	UE_DEFINE_GAMEPLAY_TAG(Running, FName{ANSITEXTVIEW("Als.Gait.Running")})
	UE_DEFINE_GAMEPLAY_TAG(Sprinting, FName{ANSITEXTVIEW("Als.Gait.Sprinting")})
}

namespace AlsOverlayModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Default, FName{ANSITEXTVIEW("Als.OverlayMode.Default")})
	UE_DEFINE_GAMEPLAY_TAG(Masculine, FName{ANSITEXTVIEW("Als.OverlayMode.Masculine")})
	UE_DEFINE_GAMEPLAY_TAG(Feminine, FName{ANSITEXTVIEW("Als.OverlayMode.Feminine")})
	UE_DEFINE_GAMEPLAY_TAG(Injured, FName{ANSITEXTVIEW("Als.OverlayMode.Injured")})
	UE_DEFINE_GAMEPLAY_TAG(HandsTied, FName{ANSITEXTVIEW("Als.OverlayMode.HandsTied")})
	UE_DEFINE_GAMEPLAY_TAG(Rifle, FName{ANSITEXTVIEW("Als.OverlayMode.Rifle")})
	UE_DEFINE_GAMEPLAY_TAG(PistolOneHanded, FName{ANSITEXTVIEW("Als.OverlayMode.PistolOneHanded")})
	UE_DEFINE_GAMEPLAY_TAG(PistolTwoHanded, FName{ANSITEXTVIEW("Als.OverlayMode.PistolTwoHanded")})
	UE_DEFINE_GAMEPLAY_TAG(Bow, FName{ANSITEXTVIEW("Als.OverlayMode.Bow")})
	UE_DEFINE_GAMEPLAY_TAG(Torch, FName{ANSITEXTVIEW("Als.OverlayMode.Torch")})
	UE_DEFINE_GAMEPLAY_TAG(Binoculars, FName{ANSITEXTVIEW("Als.OverlayMode.Binoculars")})
	UE_DEFINE_GAMEPLAY_TAG(Box, FName{ANSITEXTVIEW("Als.OverlayMode.Box")})
	UE_DEFINE_GAMEPLAY_TAG(Barrel, FName{ANSITEXTVIEW("Als.OverlayMode.Barrel")})
}

namespace AlsLocomotionActionTags
{
	UE_DEFINE_GAMEPLAY_TAG(Mantling, FName{ANSITEXTVIEW("Als.LocomotionAction.Mantling")})
	UE_DEFINE_GAMEPLAY_TAG(Ragdolling, FName{ANSITEXTVIEW("Als.LocomotionAction.Ragdolling")})
	UE_DEFINE_GAMEPLAY_TAG(GettingUp, FName{ANSITEXTVIEW("Als.LocomotionAction.GettingUp")})
	UE_DEFINE_GAMEPLAY_TAG(Rolling, FName{ANSITEXTVIEW("Als.LocomotionAction.Rolling")})
	UE_DEFINE_GAMEPLAY_TAG(Sliding, FName{ANSITEXTVIEW("Als.LocomotionAction.Sliding")})
	UE_DEFINE_GAMEPLAY_TAG(ProningTransition, FName{ANSITEXTVIEW("Als.LocomotionAction.ProningTransition")})
}

namespace AlsGroundedEntryModeTags
{
	UE_DEFINE_GAMEPLAY_TAG(FromRoll, FName{ANSITEXTVIEW("Als.GroundedEntryMode.FromRoll")})
}
