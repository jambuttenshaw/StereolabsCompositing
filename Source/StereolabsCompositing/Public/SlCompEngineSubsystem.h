// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"

#include "Subsystems/EngineSubsystem.h"

#include "Core/StereolabsCameraProxy.h"
#include "Core/StereolabsTextureBatch.h"

#include "SlCompEngineSubsystem.generated.h"


// A target can either be a view or a measure - it is convenient to wrap these together as they only differ in method of construction
class FSlCompImageTarget
{
public:
	FSlCompImageTarget(ESlView InView, bool bInInverseTonemapping = false);
	FSlCompImageTarget(ESlMeasure InMeasure);

	bool operator==(const FSlCompImageTarget&) const;

	// Returns a view if this target is targeting a view, nullopt otherwise
	TOptional<ESlView> GetView() const;
	// Returns a measure if this target is targeting a measure, nullopt otherwise
	TOptional<ESlMeasure> GetMeasure() const;

	bool IsInverseTonemappingEnabled() const { return bInverseTonemapping; }

private:
	TVariant<ESlView, ESlMeasure> ViewOrMeasure;
	bool bInverseTonemapping = false;
};


class ISlCompImageWrapper
{
public:
	// Pass key idiom is required (rather than simply using friend classes)
	// so objects can be created with MakeShared - enabled by passing PassKey's by const&
	class FPassKeyBase
	{
	protected:
		explicit FPassKeyBase() = default;
	};
public:
	virtual ~ISlCompImageWrapper() = default;

	virtual void CreateTexture(const FPassKeyBase&, TObjectPtr<USlTextureBatch> InBatch) = 0;
	virtual void DestroyTexture(const FPassKeyBase&) = 0;

	virtual void OnTextureUpdated() {}

	virtual UTexture* GetTexture() const = 0;

	virtual bool MatchesTarget(const FSlCompImageTarget& InTarget) const = 0;
};


/**
 * 
 */
UCLASS()
class STEREOLABSCOMPOSITING_API USlCompEngineSubsystem : public UEngineSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	

public:
	USlCompEngineSubsystem();

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
	template <typename... Args>
	TSharedPtr<ISlCompImageWrapper> GetOrCreateImageWrapper(Args... InArgs)
	{
		return GetOrCreateImageWrapperImpl(FSlCompImageTarget{ Forward<Args>(InArgs)... });
	}
private:
	TSharedPtr<ISlCompImageWrapper> GetOrCreateImageWrapperImpl(FSlCompImageTarget&& Target);

public:

	void DoInverseTonemapping(UTexture* Input, UTextureRenderTarget2D* Output);

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
	TArray<TWeakPtr<ISlCompImageWrapper>> Wrappers;

	bool bCanEverTick = false;

	FMatrix CameraProjectionMatrix;
	FMatrix CameraInvProjectionMatrix;
	float HorizontalFieldOfView = 90.0f;
	float VerticalFieldOfView = 90.0f;

	// For inverse tonemapping passes
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> InverseTonemappingMID;
	UPROPERTY(Transient)
	TObjectPtr<UCanvas> Canvas;
};


class FSlCompImageWrapper : public ISlCompImageWrapper
{
public:
	class FPassKey : public FPassKeyBase
	{
		friend class USlCompEngineSubsystem;
		friend class FSlCompImageWrapper;
		explicit FPassKey() = default;
	};

public:
	FSlCompImageWrapper(const FPassKey&, FSlCompImageTarget&& InTarget);
	virtual ~FSlCompImageWrapper();

	// On camera connect / disconnect, a new batch is created
	// Which means we must recreate our texture
	virtual void CreateTexture(const FPassKeyBase&, TObjectPtr<USlTextureBatch> InBatch) override;

	// Before creation of a new batch, all textures in the old batch must be destroyed
	// This will happen when camera is connected / disconnected
	virtual void DestroyTexture(const FPassKeyBase&) override;

	virtual UTexture* GetTexture() const override;

	virtual bool MatchesTarget(const FSlCompImageTarget& InTarget) const override;

private:
	FSlCompImageTarget Target;

	TStrongObjectPtr<USlTextureBatch> TextureBatch = nullptr;
	TStrongObjectPtr<USlTexture> Texture = nullptr;
};


class FSlCompTonemappedImageWrapper : public ISlCompImageWrapper
{
public:
	class FPassKey : public FPassKeyBase
	{
		friend class FSlCompTonemappedImageWrapper;
		friend class USlCompEngineSubsystem;
		explicit FPassKey() = default;
	};

public:
	FSlCompTonemappedImageWrapper(const FPassKey&, FSlCompImageTarget&& InTarget);
	virtual ~FSlCompTonemappedImageWrapper() = default;

	virtual void CreateTexture(const FPassKeyBase&, TObjectPtr<USlTextureBatch> InBatch) override;
	virtual void DestroyTexture(const FPassKeyBase&) override;

	virtual void OnTextureUpdated() override;

	virtual UTexture* GetTexture() const override;

	virtual bool MatchesTarget(const FSlCompImageTarget& InTarget) const override;

private:
	FSlCompImageTarget Target;

	// The image wrapper gives us the raw image (and allows sharing the underlying image if some clients do not want tonemapping)
	TSharedPtr<ISlCompImageWrapper> ImageWrapper;

	TStrongObjectPtr<UTextureRenderTarget2D> TonemappedTexture;
};
