// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_FirstPersonGameMode.h"

ATP_FirstPersonGameMode::ATP_FirstPersonGameMode()
{
	// Template GameMode keeps C++ side minimal and expects Blueprint to provide concrete pawn/controller classes.
	DefaultPawnClass = nullptr;
	bStartPlayersAsSpectators = false;
}
