// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#include "OpenUSDActorDetails.h"
#include "OpenUSDActor.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "OpenUSDActorDetails"

void FOpenUSDActorDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    // 缓存选中的 Actor
    SelectedActors.Empty();
    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
    DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

    for (TWeakObjectPtr<UObject> Obj : ObjectsBeingCustomized)
    {
        if (AOpenUSDActor* Actor = Cast<AOpenUSDActor>(Obj.Get()))
        {
            SelectedActors.Add(Actor);
        }
    }

    // 添加 USD Actions 分类
    IDetailCategoryBuilder& ActionCategory = DetailBuilder.EditCategory("USD Actions", LOCTEXT("USDActionsCategory", "USD Actions"));

    // Load/Unload/Reload 按钮
    ActionCategory.AddCustomRow(LOCTEXT("LoadUSDRow", "Load USD"))
    .WholeRowContent()
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(5.0f, 2.0f)
        [
            SNew(SButton)
            .Text(LOCTEXT("LoadUSD", "Load USD"))
            .ToolTipText(LOCTEXT("LoadUSDTooltip", "Load USD file and spawn mesh actors"))
            .OnClicked(this, &FOpenUSDActorDetails::OnLoadClicked)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(5.0f, 2.0f)
        [
            SNew(SButton)
            .Text(LOCTEXT("UnloadUSD", "Unload USD"))
            .ToolTipText(LOCTEXT("UnloadUSDTooltip", "Unload USD and destroy spawned actors"))
            .OnClicked(this, &FOpenUSDActorDetails::OnUnloadClicked)
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .Padding(5.0f, 2.0f)
        [
            SNew(SButton)
            .Text(LOCTEXT("ReloadUSD", "Reload USD"))
            .ToolTipText(LOCTEXT("ReloadUSDTooltip", "Reload USD file"))
            .OnClicked(this, &FOpenUSDActorDetails::OnReloadClicked)
        ]
    ];
}

FReply FOpenUSDActorDetails::OnLoadClicked()
{
    for (TWeakObjectPtr<AOpenUSDActor> ActorPtr : SelectedActors)
    {
        if (AOpenUSDActor* Actor = ActorPtr.Get())
        {
            Actor->Load();
        }
    }
    return FReply::Handled();
}

FReply FOpenUSDActorDetails::OnUnloadClicked()
{
    for (TWeakObjectPtr<AOpenUSDActor> ActorPtr : SelectedActors)
    {
        if (AOpenUSDActor* Actor = ActorPtr.Get())
        {
            Actor->Unload();
        }
    }
    return FReply::Handled();
}

FReply FOpenUSDActorDetails::OnReloadClicked()
{
    for (TWeakObjectPtr<AOpenUSDActor> ActorPtr : SelectedActors)
    {
        if (AOpenUSDActor* Actor = ActorPtr.Get())
        {
            Actor->Reload();
        }
    }
    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
