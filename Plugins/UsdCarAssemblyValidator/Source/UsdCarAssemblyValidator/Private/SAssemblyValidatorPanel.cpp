// Copyright Epic Games, Inc. All Rights Reserved.

#include "SAssemblyValidatorPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Styling/AppStyle.h"
#include "DesktopPlatformModule.h"

void SAssemblyValidatorPanel::Construct(const FArguments& InArgs)
{
	IdentifiedParts.Add(MakeShared<FString>(TEXT("Body")));
	IdentifiedParts.Add(MakeShared<FString>(TEXT("Wheel_FL")));
	IdentifiedParts.Add(MakeShared<FString>(TEXT("Wheel_FR")));
	IdentifiedParts.Add(MakeShared<FString>(TEXT("Wheel_RL")));
	IdentifiedParts.Add(MakeShared<FString>(TEXT("Wheel_RR")));
	IdentifiedParts.Add(MakeShared<FString>(TEXT("Caliper_FL")));
	IdentifiedParts.Add(MakeShared<FString>(TEXT("SteeringWheel")));

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Select USD:")))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(10.0f, 0.0f, 0.0f, 0.0f)
					[
						SAssignNew(UsdFileButton, SComboButton)
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Choose File...")))
						]
						.MenuContent()
						[
							SNew(SButton)
							.Text(FText::FromString(TEXT("Browse")))
							.OnClicked(this, &SAssemblyValidatorPanel::OnBrowseUsdFile)
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f)
					[
						SAssignNew(ReloadStageButton, SButton)
						.Text(FText::FromString(TEXT("Reload")))
						.OnClicked(this, &SAssemblyValidatorPanel::OnReloadStage)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f)
					[
						SAssignNew(ScanVariantsButton, SButton)
						.Text(FText::FromString(TEXT("Scan")))
						.OnClicked(this, &SAssemblyValidatorPanel::OnScanVariants)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f)
					[
						SAssignNew(VerifyAssemblyButton, SButton)
						.Text(FText::FromString(TEXT("Verify")))
						.OnClicked(this, &SAssemblyValidatorPanel::OnVerifyAssembly)
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(0.75f)
		.Padding(5.0f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(5.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Variants")))
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SAssignNew(VariantSetsTreeView, STreeView<TSharedPtr<FString>>)
						.TreeItemsSource(&VariantSetItems)
						.OnGenerateRow(this, &SAssemblyValidatorPanel::GenerateVariantSetRow)
						.OnGetChildren_Lambda([](TSharedPtr<FString> Item, TArray<TSharedPtr<FString>>& OutChildren) {})
					]
				]
			]
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(5.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Current")))
						.Justification(ETextJustify::Center)
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(5.0f)
					[
						SAssignNew(CurrentVariantTextBlock, STextBlock)
						.Text(FText::FromString(TEXT("None")))
						.ColorAndOpacity(FLinearColor::Yellow)
					]
				]
			]
			+ SSplitter::Slot()
			.Value(0.5f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(5.0f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Parts:")))
						.Justification(ETextJustify::Left)
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SAssignNew(IdentifiedPartsListView, SListView<TSharedPtr<FString>>)
						.ListItemsSource(&IdentifiedParts)
						.OnGenerateRow(this, &SAssemblyValidatorPanel::GeneratePartRow)
						.HeaderRow(SNew(SHeaderRow) + SHeaderRow::Column("PartName")[SNew(STextBlock).Text(FText::FromString(TEXT("Part")))])
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5.0f)
				[
					SAssignNew(InjectToCCRButton, SButton)
					.Text(FText::FromString(TEXT("Inject to CCR")))
					.OnClicked(this, &SAssemblyValidatorPanel::OnInjectToCCR)
					.HAlign(HAlign_Center)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5.0f, 0.0f)
					[
						SAssignNew(SuccessStatTextBlock, STextBlock)
						.Text(FText::FromString(TEXT("Success: 0")))
						.ColorAndOpacity(FLinearColor::Green)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5.0f, 0.0f)
					[
						SAssignNew(WarningStatTextBlock, STextBlock)
						.Text(FText::FromString(TEXT("Warning: 0")))
						.ColorAndOpacity(FLinearColor::Yellow)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5.0f, 0.0f)
					[
						SAssignNew(FailedStatTextBlock, STextBlock)
						.Text(FText::FromString(TEXT("Failed: 0")))
						.ColorAndOpacity(FLinearColor::Red)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.MaxHeight(150.0f)
				.Padding(5.0f)
				[
					SNew(SBox)
					.MaxDesiredHeight(150.0f)
					[
						SAssignNew(LogOutputTextBox, SMultiLineEditableTextBox)
						.IsReadOnly(true)
						.Text(FText::FromString(TEXT("Log Output...\n")))
						.Font(FAppStyle::GetFontStyle(FName("Mono.9")))
					]
				]
			]
		]
	];
	
	AddLogMessage(TEXT("[System] Panel initialized."));
}

FReply SAssemblyValidatorPanel::OnBrowseUsdFile()
{
	IDesktopPlatform* DP = FDesktopPlatformModule::Get();
	if (DP)
	{
		TArray<FString> Files;
		DP->OpenFileDialog(nullptr, TEXT("Select USD File"), FPaths::ProjectContentDir(), TEXT(""), TEXT("USD Files (*.usd)|*.usd"), EFileDialogFlags::None, Files);
		if (Files.Num() > 0)
		{
			CurrentUsdFile = Files[0];
			AddLogMessage(FString::Printf(TEXT("[Load] File: %s"), *FPaths::GetCleanFilename(CurrentUsdFile)));
		}
	}
	return FReply::Handled();
}

FReply SAssemblyValidatorPanel::OnReloadStage()
{
	AddLogMessage(TEXT("[Action] Reload stage."));
	return FReply::Handled();
}

FReply SAssemblyValidatorPanel::OnScanVariants()
{
	AddLogMessage(TEXT("[Action] Scan variants."));
	VariantSetItems.Empty();
	VariantSetItems.Add(MakeShared<FString>(TEXT("Variant 1")));
	VariantSetItems.Add(MakeShared<FString>(TEXT("Variant 2")));
	if (VariantSetsTreeView.IsValid())
	{
		VariantSetsTreeView->RequestTreeRefresh();
	}
	return FReply::Handled();
}

FReply SAssemblyValidatorPanel::OnVerifyAssembly()
{
	AddLogMessage(TEXT("[Action] Verify assembly."));
	SuccessCount++;
	UpdateStatistics();
	return FReply::Handled();
}

FReply SAssemblyValidatorPanel::OnInjectToCCR()
{
	AddLogMessage(TEXT("[Action] Inject to CCR."));
	SuccessCount++;
	UpdateStatistics();
	return FReply::Handled();
}

TSharedRef<ITableRow> SAssemblyValidatorPanel::GenerateVariantSetRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem.IsValid() ? *InItem : TEXT("")))
		];
}

TSharedRef<ITableRow> SAssemblyValidatorPanel::GeneratePartRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem.IsValid() ? *InItem : TEXT("")))
		];
}

void SAssemblyValidatorPanel::AddLogMessage(const FString& Message, bool bIsError, bool bIsWarning)
{
	if (LogOutputTextBox.IsValid())
	{
		FString CurrentText = LogOutputTextBox->GetText().ToString();
		
		// Simple line limiting - keep track of line count
		int32 LineCount = 0;
		for (int32 i = 0; i < CurrentText.Len(); ++i)
		{
			if (CurrentText[i] == '\n') LineCount++;
		}
		
		// If too many lines, remove old content
		if (LineCount > 50)
		{
			int32 RemoveUntil = 0;
			int32 Count = 0;
			for (int32 i = 0; i < CurrentText.Len(); ++i)
			{
				if (CurrentText[i] == '\n')
				{
					Count++;
					if (Count >= LineCount - 50)
					{
						RemoveUntil = i + 1;
						break;
					}
				}
			}
			CurrentText = CurrentText.Mid(RemoveUntil);
		}
		
		FString Timestamp = FDateTime::Now().ToString(TEXT("%H:%M:%S"));
		FString LogEntry = FString::Printf(TEXT("[%s] %s\n"), *Timestamp, *Message);
		
		CurrentText.Append(LogEntry);
		LogOutputTextBox->SetText(FText::FromString(CurrentText));
	}
	
	if (bIsError)
	{
		FailureCount++;
	}
	else if (bIsWarning)
	{
		WarningCount++;
	}
}

void SAssemblyValidatorPanel::UpdateStatistics()
{
	if (SuccessStatTextBlock.IsValid())
	{
		SuccessStatTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Success: %d"), SuccessCount)));
	}
	if (WarningStatTextBlock.IsValid())
	{
		WarningStatTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Warning: %d"), WarningCount)));
	}
	if (FailedStatTextBlock.IsValid())
	{
		FailedStatTextBlock->SetText(FText::FromString(FString::Printf(TEXT("Failed: %d"), FailureCount)));
	}
}
