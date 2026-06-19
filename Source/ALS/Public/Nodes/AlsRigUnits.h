#pragma once

#include "RigVMFunctions/RigVMFunction_ControlFlow.h"
#include "RigVMFunctions/Math/RigVMFunction_MathFloat.h"
#include "RigVMFunctions/Simulation/RigVMFunction_SimBase.h"
#include "Units/RigUnit.h"
#include "AlsRigUnits.generated.h"

/**
* FAlsRigVMFunction_Clamp01Float
*/
USTRUCT(DisplayName = "Clamp 01", Meta = (Category = "ALS"))
struct ALS_API FAlsRigVMFunction_Clamp01Float : public FRigVMFunction_MathFloatBase
{
	GENERATED_BODY()

public:
	/** Clamp target value */
	UPROPERTY(Meta = (Input))
	float Value{0.0f};

	/** Clamp result */
	UPROPERTY(Meta = (Output))
	float Result{0.0f};

public:
	RIGVM_METHOD()
	virtual void Execute() override;
};

/**
* FAlsRigVMFunction_DamperExactVector
*/
USTRUCT(DisplayName = "Damper Exact (Vector)", Meta = (Category = "ALS"))
struct ALS_API FAlsRigVMFunction_DamperExactVector : public FRigVMFunction_SimBase
{
	GENERATED_BODY()

public:
	/** Damper Exact Vector target value */
	UPROPERTY(Meta = (Input))
	FVector Target{ForceInit};

	// HalfLife is the time it takes for the distance to the target to be reduced by half.
	UPROPERTY(Meta = (Input, ClampMin = 0, ForceUnits = "s"))
	float HalfLife{1.0f};

	/** Current Damper Exact Vector value */
	UPROPERTY(Transient, Meta = (Output))
	FVector Current{ForceInit};

	/** Is Damper Exact Vector value */
	UPROPERTY(Transient)
	bool bInitialized{false};

public:
	virtual void Initialize() override;

	RIGVM_METHOD()
	// ReSharper disable once CppFunctionIsNotImplemented
	virtual void Execute() override;
};

/**
* FAlsRigVMFunction_DamperExactQuaternion
*/
USTRUCT(DisplayName = "Damper Exact (Quaternion)", Meta = (Category = "ALS"))
struct ALS_API FAlsRigVMFunction_DamperExactQuaternion : public FRigVMFunction_SimBase
{
	GENERATED_BODY()

public:
	/** Damper Exact Quat target value */
	UPROPERTY(Meta = (Input))
	FQuat Target{ForceInit};

	// HalfLife is the time it takes for the distance to the target to be reduced by half.
	UPROPERTY(Meta = (Input, ClampMin = 0, ForceUnits = "s"))
	float HalfLife{1.0f};

	/** Current Damper Exact Quat value */
	UPROPERTY(Transient, Meta = (Output))
	FQuat Current{ForceInit};

	/** Is Damper Exact Quat value */
	UPROPERTY(Transient)
	bool bInitialized{false};

public:
	virtual void Initialize() override;

	RIGVM_METHOD()
	// ReSharper disable once CppFunctionIsNotImplemented
	virtual void Execute() override;
};

// Calculates the projection location and direction of the perpendicular to AC through B.
USTRUCT(DisplayName = "Calculate Pole Vector", Meta = (Category = "ALS", NodeColor = "0.05 0.25 0.05"))
struct ALS_API FAlsRigUnit_CalculatePoleVector : public FRigUnit
{
	GENERATED_BODY()

public:
	UPROPERTY(Meta = (Input, ExpandByDefault))
	FRigElementKey ItemA;

	UPROPERTY(Meta = (Input, ExpandByDefault))
	FRigElementKey ItemB;

	UPROPERTY(Meta = (Input, ExpandByDefault))
	FRigElementKey ItemC;

	UPROPERTY(Meta = (Input))
	bool bInitial{false};

	UPROPERTY(Transient, Meta = (Output))
	bool bSuccess{false};

	UPROPERTY(Transient, DisplayName = "Item B Location", Meta = (Output))
	FVector ItemBLocation{ForceInit};

	UPROPERTY(Transient, DisplayName = "Item B Projection Location", Meta = (Output))
	FVector ItemBProjectionLocation{ForceInit};

	UPROPERTY(Transient, Meta = (Output))
	FVector PoleDirection{FVector::XAxisVector};

	UPROPERTY(Transient)
	FCachedRigElement CachedItemA;

	UPROPERTY(Transient)
	FCachedRigElement CachedItemB;

	UPROPERTY(Transient)
	FCachedRigElement CachedItemC;

public:
	RIGVM_METHOD()
	// ReSharper disable once CppFunctionIsNotImplemented
	virtual void Execute() override;
};

/**
* FAlsRigVMFunction_IsGameWorld
*/
USTRUCT(DisplayName = "Is Game World", Meta = (Category = "ALS"))
struct ALS_API FAlsRigVMFunction_IsGameWorld : public FRigVMFunction_ControlFlowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient, DisplayName = "Execute", Meta = (Input))
	FRigVMExecuteContext ExecuteContext;

	/** What to execute if True */
	UPROPERTY(Transient, Meta = (Output))
	FRigVMExecuteContext True;

	/** What to execute if False */
	UPROPERTY(Transient, Meta = (Output))
	FRigVMExecuteContext False;

	/** What to execute when completed */
	UPROPERTY(meta=(Output))
	FRigVMExecuteContext Completed;

	UPROPERTY(Transient, Meta = (Singleton))
	FName BlockToRun;

public:
	RIGVM_METHOD()
	// ReSharper disable once CppFunctionIsNotImplemented
	virtual void Execute() override;

	virtual const TArray<FName>& GetControlFlowBlocks_Impl() const override;
};
