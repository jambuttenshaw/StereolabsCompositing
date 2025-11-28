// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"

#include "Subsystems/EngineSubsystem.h"

#include "Core/StereolabsCameraProxy.h"
#include "Core/StereolabsTextureBatch.h"

#include "SlCompEngineSubsystem.generated.h"

class FSlCompImageWrapper;

// A target can either be a view or a measure - it is convenient to wrap these together as they only differ in method of construction
class FSlCompImageWrapperTarget
{
public:
	FSlCompImageWrapperTarget(ESlView View);
	FSlCompImageWrapperTarget(ESlMeasure Measure);

	bool operator==(const FSlCompImageWrapperTarget&) const;

	// Returns a view if this target is targeting a view, nullopt otherwise
	TOptional<ESlView> GetView() const;
	// Returns a measure if this target is targeting a measure, nullopt otherwise
	TOptional<ESlMeasure> GetMeasure() const;

private:
	TVariant<ESlView, ESlMeasure> ViewOrMeasure;
};

/**
 * 
 */
UCLASS()
class STEREOLABSCOMPOSITING_API USlCompEngineSubsystem : public UEngineSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//~ Begin UEngineSubsystem interface	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End	UEngineSubsystem interface

	//~ Begin FTickableGameObject interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableInEditor() const override { return bCanEverTick; }
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
	//~ End FTickableGameObject interface

protected:
	// Callbacks from camera proxy
	UFUNCTION()
	void OnCameraOpened();
	UFUNCTION()
	void OnCameraClosed();

public:

	// Images from camera are provided by an ImageWrapper
	// This object manages the lifetime of textures and enables
	// sharing of resources between clients
	template <typename T>
	TSharedPtr<FSlCompImageWrapper> GetOrCreateImageWrapper(T InTarget)
	{
		return GetOrCreateImageWrapperImpl(FSlCompImageWrapperTarget{ InTarget });
	}
private:
	TSharedPtr<FSlCompImageWrapper> GetOrCreateImageWrapperImpl(FSlCompImageWrapperTarget&& Target);

public:
	// Camera properties

	UFUNCTION(BlueprintCallable)
	const FMatrix& GetProjectionMatrix() { return CameraProjectionMatrix; }
	UFUNCTION(BlueprintCallable)
	const FMatrix& GetInvProjectionMatrix() { return CameraInvProjectionMatrix; }

	UFUNCTION(BlueprintCallable)
	inline float GetHorizontalFieldOfView() { return HorizontalFieldOfView;}
	
	UFUNCTION(BlueprintCallable)
	inline float GetVerticalFieldOfView() { return VerticalFieldOfView;}
	

private:
	/** Current batch */
	TStrongObjectPtr<USlTextureBatch> Batch = nullptr;

	// Wrappers manage the lifetime of images within the batch
	// The engine subsystem manages distributing wrappers to clients
	// We don't want the engine subsystem itself to participate in the lifetime management of the wrappers
	TArray<TWeakPtr<FSlCompImageWrapper>> Wrappers;

	bool bCanEverTick = false;

	FMatrix CameraProjectionMatrix;
	FMatrix CameraInvProjectionMatrix;
	float HorizontalFieldOfView = 90.0f;
	float VerticalFieldOfView = 90.0f;
};


class FSlCompImageWrapper
{
public:
	// Pass key idiom is required (rather than simply using friend classes)
	// so objects can be created with MakeShared - enabled by passing PassKey's by const&
	class PassKey
	{
		friend class FSlCompImageWrapper;
		friend class USlCompEngineSubsystem;
		explicit PassKey() = default;
	};

public:
	FSlCompImageWrapper(const PassKey&, FSlCompImageWrapperTarget&& InTarget, TObjectPtr<USlTextureBatch> InBatch);
	~FSlCompImageWrapper();

	// On camera connect / disconnect, a new batch is created
	// Which means we must recreate our texture
	void CreateTexture(const PassKey&, TObjectPtr<USlTextureBatch> InBatch);

	// Before creation of a new batch, all textures in the old batch must be destroyed
	// This will happen when camera is connected / disconnected
	void DestroyTexture(const PassKey&);

	UTexture2D* GetTexture() const;

	bool Matches(const FSlCompImageWrapperTarget& InTarget) const;

private:
	// Variant of view or measure
	FSlCompImageWrapperTarget Target;

	TStrongObjectPtr<USlTextureBatch> TextureBatch = nullptr;
	TStrongObjectPtr<USlTexture> Texture = nullptr;
};
