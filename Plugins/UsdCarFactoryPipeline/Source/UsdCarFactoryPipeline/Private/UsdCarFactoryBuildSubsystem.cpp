#include "UsdCarFactoryBuildSubsystem.h"

#include "Actors/UsdHierarchicalBuildActor.h"
#include "Async/Async.h"
#include "Editor.h"
#include "HAL/PlatformTime.h"

using namespace UsdCarFactoryPipelineBuild;

DEFINE_LOG_CATEGORY_STATIC(LogUsdCarFactoryBuildSubsystem, Log, All);

UUsdCarFactoryBuildSubsystem* UUsdCarFactoryBuildSubsystem::Get()
{
	static TWeakObjectPtr<UUsdCarFactoryBuildSubsystem> FallbackSubsystem;

	auto EnsureFallbackSubsystem = []() -> UUsdCarFactoryBuildSubsystem*
	{
		if (!FallbackSubsystem.IsValid())
		{
			UUsdCarFactoryBuildSubsystem* NewFallbackSubsystem =
				NewObject<UUsdCarFactoryBuildSubsystem>(GetTransientPackage(), NAME_None, RF_Transient);
			if (!NewFallbackSubsystem)
			{
				return nullptr;
			}

			NewFallbackSubsystem->AddToRoot();
			FallbackSubsystem = NewFallbackSubsystem;

			UE_LOG(
				LogUsdCarFactoryBuildSubsystem,
				Warning,
				TEXT("Editor subsystem instance is unavailable. Falling back to transient UsdCarFactoryBuildSubsystem.")
			);
		}

		return FallbackSubsystem.Get();
	};

	if (UUsdCarFactoryBuildSubsystem* ExistingFallbackSubsystem = FallbackSubsystem.Get())
	{
		if (ExistingFallbackSubsystem->HasGlobalParseWork() || ExistingFallbackSubsystem->JobStates.Num() > 0)
		{
			return ExistingFallbackSubsystem;
		}
	}

	if (GEditor)
	{
		if (UUsdCarFactoryBuildSubsystem* EditorSubsystem = GEditor->GetEditorSubsystem<UUsdCarFactoryBuildSubsystem>())
		{
			return EditorSubsystem;
		}
	}

	return EnsureFallbackSubsystem();
}

void UUsdCarFactoryBuildSubsystem::Deinitialize()
{
	JobStates.Reset();
	GlobalParseSchedulerState.CompletedResultCount.Reset();

	FSourceParseResult IgnoredResult;
	while (GlobalParseSchedulerState.CompletedResults.Dequeue(IgnoredResult))
	{
	}

	Super::Deinitialize();
}

void UUsdCarFactoryBuildSubsystem::EnqueueGlobalParseTask(FSourceParseTask&& Task)
{
	// UnrealUSD stage open/traversal has proven unstable on background worker threads.
	// Execute immediately on the caller thread so legacy async call sites stay functional
	// without sending USD resolver work to the thread pool.
	ExecuteGlobalParseTask(TWeakObjectPtr<UUsdCarFactoryBuildSubsystem>(this), MoveTemp(Task));
}

bool UUsdCarFactoryBuildSubsystem::DequeueGlobalParseResult(FSourceParseResult& OutResult)
{
	if (!GlobalParseSchedulerState.CompletedResults.Dequeue(OutResult))
	{
		return false;
	}

	GlobalParseSchedulerState.CompletedResultCount.Decrement();
	return true;
}

bool UUsdCarFactoryBuildSubsystem::HasGlobalCompletedParseResults() const
{
	return GlobalParseSchedulerState.CompletedResultCount.GetValue() > 0;
}

bool UUsdCarFactoryBuildSubsystem::HasGlobalParseWork() const
{
	return GlobalParseSchedulerState.CompletedResultCount.GetValue() > 0;
}

FUsdProxyBuildJobState& UUsdCarFactoryBuildSubsystem::GetJobState(AUsdHierarchicalBuildActor& Actor)
{
	return JobStates.FindOrAdd(TObjectKey<AUsdHierarchicalBuildActor>(&Actor));
}

void UUsdCarFactoryBuildSubsystem::ResetJobState(AUsdHierarchicalBuildActor& Actor)
{
	GetJobState(Actor).ResetForNextBuild();
}

void UUsdCarFactoryBuildSubsystem::RemoveJobState(const AUsdHierarchicalBuildActor& Actor)
{
	JobStates.Remove(TObjectKey<AUsdHierarchicalBuildActor>(const_cast<AUsdHierarchicalBuildActor*>(&Actor)));
}

void UUsdCarFactoryBuildSubsystem::ExecuteGlobalParseTask(
	TWeakObjectPtr<UUsdCarFactoryBuildSubsystem> WeakSubsystem,
	FSourceParseTask&& Task
)
{
	FSourceDeltaScanResult DeltaResult;
	const double DeltaScanStartSeconds = FPlatformTime::Seconds();
	const bool bDeltaScanSucceeded = AUsdHierarchicalBuildActor::RunSourceDeltaScanSync(
		Task.SourceUsdPath,
		Task.CacheSnapshots,
		DeltaResult
	);

	FSourceParseResult ParseResult = AUsdHierarchicalBuildActor::MakeParseResult(
		Task.TargetActor,
		Task.RequestId,
		Task.SourceUsdPath,
		Task.SourceUsdTimestampUtc,
		Task.SourceUsdFileSizeBytes,
		MoveTemp(DeltaResult),
		bDeltaScanSucceeded,
		FPlatformTime::Seconds() - DeltaScanStartSeconds,
		TEXT("Asynchronous source delta scan failed.")
	);

	if (UUsdCarFactoryBuildSubsystem* StrongSubsystem = WeakSubsystem.Get())
	{
		StrongSubsystem->GlobalParseSchedulerState.CompletedResults.Enqueue(MoveTemp(ParseResult));
		StrongSubsystem->GlobalParseSchedulerState.CompletedResultCount.Increment();

		AsyncTask(
			ENamedThreads::GameThread,
			[WeakSubsystem, TargetActor = Task.TargetActor]()
			{
				if (UUsdCarFactoryBuildSubsystem* GameThreadSubsystem = WeakSubsystem.Get())
				{
					if (AUsdHierarchicalBuildActor* StrongActor = TargetActor.Get())
					{
						StrongActor->ScheduleParseResultPump();
					}
				}
			}
		);
	}
}
