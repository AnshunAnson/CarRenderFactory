// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class AOpenUSDActor;

/**
 * FOpenUSDActorDetails - OpenUSD Actor 的 Details 面板自定义
 *
 * 添加快捷操作按钮：
 * - Load USD
 * - Unload USD
 * - Reload USD
 */
class FOpenUSDActorDetails : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance()
    {
        return MakeShareable(new FOpenUSDActorDetails);
    }

    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    /** Load USD 按钮回调 */
    FReply OnLoadClicked();

    /** Unload USD 按钮回调 */
    FReply OnUnloadClicked();

    /** Reload USD 按钮回调 */
    FReply OnReloadClicked();

    /** 缓存选中的 Actor */
    TArray<TWeakObjectPtr<AOpenUSDActor>> SelectedActors;
};
