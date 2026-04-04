// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Horror/HorrorGameMode.h"

AHorrorGameMode::AHorrorGameMode()
{
	// Horror variant relies on Blueprint-authored classes for concrete pawn/controller/UI composition.
	DefaultPawnClass = nullptr;
	bStartPlayersAsSpectators = false;
}
