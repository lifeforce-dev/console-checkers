//---------------------------------------------------------------
//
// ConsoleGameDisplay.cpp
//

#include "ConsoleGameDisplay.h"

#include "IGameBoardViewStrategy.h"
#include "IGameStateDisplayInfo.h"
#include "UIEvents.h"
#include "UITextStrings.h"

#include <iostream>
#include <ranges>

#include <spdlog/spdlog.h>
// Magic include needed for spdlog to log a custom type.
// Must be included after spdlog.h. Thanks spdlog.
#include <spdlog/fmt/ostr.h>
// Magic include needed for spdlog to log a vector inline.
#include <spdlog/fmt/ranges.h>

namespace Checkers {

//===============================================================

void ConsoleGameDisplay::Initialize(IGameStateDisplayInfo* displayData, UIEvents& events)
{
	m_gameStateInfo = displayData;

	events.GetWelcomePromptRequestedEvent().subscribe(
		[this]()
	{
		std::string uiScreenText = fmt::format("{}\n", UIText::s_welcomeMessage);
		std::cout << uiScreenText;
	});

	events.GetGameBoardViewStrategyChangedEvent().subscribe(
		[this](IGameBoardViewStrategy* view)
	{
		if (!view)
		{
			// We'll silently (to the player) fail here.
			spdlog::error("UI notified of null view style change.");
			return;
		}

		IGameBoardViewStrategy* old = m_gameBoardView;
		m_gameBoardView = view;

		// We assume that if we didn't have a view before, this call was initialization
		// and not a change.
		if (old)
		{
			// We only want to reprint the board if our view actually changed
			std::cout << GetFullGameDisplayText();
		}
	});

	events.GetHelpPromptRequestedEvent().subscribe(
		[this]()
	{
		std::cout << fmt::format(UIText::s_helpCommandPromptMessage, m_gameBoardView->GetMoveCommandSyntax());
	});

	events.GetCommandErrorPromptRequestedEvent().subscribe(
		[this](const std::string& promptMessage)
	{
		std::cout << promptMessage << "\n";
	});

	events.GetTurnChangedEvent().subscribe(
		[this]()
	{
		const std::string gameUI = fmt::format("\n{}\n{}",
		"-------------------------------------",
		GetFullGameDisplayText());
		std::cout << gameUI;
	});

	events.GetPieceCapturedEvent().subscribe(
		[this]()
	{
		std::cout << fmt::format("{} captures a piece!\n", m_gameStateInfo->GetTurnPlayerId());
	});
	events.GetPieceMovedEvent().subscribe(
		[this]()
	{
		// TODO: Pass the board indices, and then ask the view strategy to translate to player-facing coords.
		// This way we can say the player moved from A to B.
		std::cout << fmt::format("{} moves a piece. \n", m_gameStateInfo->GetTurnPlayerId());
	});

	events.GetAdditionalPieceCaptureRequiredEvent().subscribe(
		[this]()
	{
		const std::string gameUI = fmt::format("\n{}\n{}\nAn additional capture is required!\n",
			"-------------------------------------",
			GetFullGameDisplayText());
		std::cout << gameUI;
	});

	events.GetGameplayErrorPromptRequestedEvent().subscribe(
		[this](const std::string& promptMessage)
	{
		std::cout << fmt::format("{}\n", promptMessage);
	});

	events.GetWinConditionMetEvent().subscribe(
		[this](WinConditionReason reason)
	{
		switch (reason)
		{
		case WinConditionReason::AllEnemyPiecesCapturedWin:
			std::cout << fmt::format("{} has captured all of the opponent's remaining pieces and wins!",
				m_gameStateInfo->GetTurnPlayerId());
			break;
		case WinConditionReason::NoAvailableMovesLoss:
			{
			Identity opponentId = m_gameStateInfo->GetTurnPlayerId() == Identity::Red ? Identity::Black : Identity::Red;
				std::cout << fmt::format("{} is unable to move, {} wins!",
					m_gameStateInfo->GetTurnPlayerId(),
					opponentId);
				break;
			}
		case WinConditionReason::GameStateViolationDraw:
			std::cout << "Game is a draw! Same game state occurred 3 times.";
			break;
		case WinConditionReason::None:
			spdlog::warn("Win condition triggered but was empty.");
			break;
		default:
			spdlog::error("Unhandled win condition");
			break;
		}
	});

	m_isInitialized = true;
}

std::string ConsoleGameDisplay::GetGameBoardDisplay() const
{
#ifdef DEBUG
	// We should not be calling any functions on ConsoleGameDisplay unless we're initialized.
	assert(m_isInitialized);

	// Hi! If you're here, you forgot to set a view, you should go do that.
	assert(m_gameBoardView);
#endif

	return m_gameBoardView->GetGameBoardDisplayText(m_gameStateInfo->GetGameBoardData());
}

std::string ConsoleGameDisplay::GetFullGameDisplayText() const
{
	std::string turnPlayerString = fmt::format("Turn Player: {}", m_gameStateInfo->GetTurnPlayerId());

	std::string blackPlayerCapturedPieces = fmt::format(
		"Black Player Captured Pieces:{}", m_gameStateInfo->GetBlackPlayerCapturedPieces().size());

	std::string redCapturedPieces = fmt::format(
		"Red Player Captured Pieces:{}", m_gameStateInfo->GetRedPlayerCapturedPieces().size());

	// {Turn player}{black captured pieces}{game state}{red captured pieces}
	return fmt::format("{}\n\n{}\n{}\n{}\n",
		blackPlayerCapturedPieces,
		GetGameBoardDisplay(),
		redCapturedPieces,
		turnPlayerString);
}

//===============================================================

}
