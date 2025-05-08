// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "Subsystems/EngineSubsystem.h"
#include "Stereolabs/Public/Core/StereolabsCameraProxy.h"

#include "StereolabsCompositingEngineSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class STEREOLABSCOMPOSITING_API UStereolabsCompositingEngineSubsystem : public UEngineSubsystem, public FTickableGameObject
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
	// Textures with camera data:

	UPROPERTY(BlueprintReadOnly, Category = "Stereolabs|Textures")
	USlTexture* ColorTexture;

	/** Left eye depth texture  */
	UPROPERTY(BlueprintReadOnly, Category = "Stereolabs|Textures")
	USlTexture* DepthTexture;

	/** Left eye depth texture  */
	UPROPERTY(BlueprintReadOnly, Category = "Stereolabs|Textures")
	USlTexture* NormalTexture;

	UFUNCTION(BlueprintCallable)
	UTexture2D* GetColorTexture();

	UFUNCTION(BlueprintCallable)
	UTexture2D* GetDepthTexture();

	UFUNCTION(BlueprintCallable)
	UTexture2D* GetNormalTexture();

private:
	/** Current batch */
	UPROPERTY()
	class USlGPUTextureBatch* Batch;

	bool bCanEverTick = false;
};
