// Copyright Epic Games, Inc. All Rights Reserved.

#include "StereolabsCompositingStyle.h"
#include "StereolabsCompositing.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FStereolabsCompositingStyle::StyleInstance = nullptr;

void FStereolabsCompositingStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FStereolabsCompositingStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FStereolabsCompositingStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("StereolabsCompositingStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FStereolabsCompositingStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("StereolabsCompositingStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("StereolabsCompositing")->GetBaseDir() / TEXT("Resources"));

	Style->Set("StereolabsCompositing.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FStereolabsCompositingStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FStereolabsCompositingStyle::Get()
{
	return *StyleInstance;
}
