// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"

AMainMenuGameMode::AMainMenuGameMode()
{
	// 메인 메뉴에서는 플레이어 캐릭터(APlayerPawn)가 스폰될 필요가 없으므로 기본 Pawn을 nullptr 또는 기본 SpectatorPawn으로 설정합니다.
	DefaultPawnClass = nullptr;
}
