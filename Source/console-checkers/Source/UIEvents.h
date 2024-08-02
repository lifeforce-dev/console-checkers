//---------------------------------------------------------------
//
// UIEvents.h
//

#pragma once

#include <observable/observable.hpp>

namespace Checkers {

//===============================================================

class IGameBoardViewStrategy;

// These events are intended to be one direction notifications from the game to the display.
// The intention is that we could completely rip out the display and rewrite it with whatever we want
// (SFML GUI, OpenGL, Direct X, imgui, etc), and so long as the display hooks up these events, everything should work.
class UIEvents
{
public:

	// The game is requesting that we show a welcome prompt to the player.
	using WelcomePromptRequestedEvent = observable::subject<void()>;
	WelcomePromptRequestedEvent& GetWelcomePromptRequestedEvent() { return m_welcomePromptRequestedEvent; }

	// The player has requested the help message.
	using HelpPromptRequestedEvent = observable::subject<void()>;
	HelpPromptRequestedEvent& GetHelpPromptRequestedEvent() { return m_helpPromptRequestedEvent; }

	// A player has requested hints. We will show the best available move.
	using DisplayHintsRequestedEvent = observable::subject<void()>;
	DisplayHintsRequestedEvent& GetDisplayHintRequestedEvent() { return m_displayHintsRequestedEvent; }

	// An error occurred with a command and the player should be informed.
	using CommandErrorPromptRequestedEvent = observable::subject<void(const std::string&)>;
	CommandErrorPromptRequestedEvent& GetCommandErrorPromptRequestedEvent() { return m_commandErrorPromptRequestedEvent; }

	// This is a UX-level error that happens at the game level (beyond command processing), the player needs to be notified.
	using GameplayErrorPromptRequestedEvent = observable::subject<void(const std::string&)>;
	GameplayErrorPromptRequestedEvent& GetGameplayErrorPromptRequestedEvent() { return m_gameplayErrorPromptRequestedEvent; }

	// The game has requested that we change the visual representation of the checkers board.
	using GameBoardViewStrategyChangedEvent = observable::subject<void(IGameBoardViewStrategy*)>;
	GameBoardViewStrategyChangedEvent& GetGameBoardViewStrategyChangedEvent() { return m_gameBoardViewStrategyChangedEvent; }

	// Player turn has toggled from one to the other.
	using TurnChangedEvent = observable::subject<void()>;
	TurnChangedEvent& GetTurnChangedEvent() { return m_turnChangedEvent; }

	// Happens when a piece has been captured.
	using PieceCapturedEvent = observable::subject<void()>;
	PieceCapturedEvent& GetPieceCapturedEvent() { return m_pieceCapturedEvent; }

	// Happens when an additional piece capture is required. UI should redraw the game in this case.
	using AdditionalPieceCaptureRequiredEvent = observable::subject<void()>;
	AdditionalPieceCaptureRequiredEvent& GetAdditionalPieceCaptureRequiredEvent() { return m_additionalPieceCaptureRequiredEvent; }

	// Happens when a piece simply moves.
	using PieceMovedEvent = observable::subject<void()>;
	PieceMovedEvent& GetPieceMovedEvent() { return m_pieceMovedEvent; }

	// If a piece was kinged, you'll receive this message instead of a PieceMoved event.
	using PiecePromotedEvent = observable::subject<void()>;
	PiecePromotedEvent& GetPiecePromotedEvent() { return m_piecePromotedEvent; }

	// Game has concluded, either a win or a draw.
	using WinConditionMetEvent = observable::subject<void(WinConditionReason reason)>;
	WinConditionMetEvent& GetWinConditionMetEvent() { return m_winConditionMetEvent; }

private:
	WelcomePromptRequestedEvent m_welcomePromptRequestedEvent;
	HelpPromptRequestedEvent m_helpPromptRequestedEvent;
	CommandErrorPromptRequestedEvent m_commandErrorPromptRequestedEvent;
	GameplayErrorPromptRequestedEvent m_gameplayErrorPromptRequestedEvent;
	GameBoardViewStrategyChangedEvent m_gameBoardViewStrategyChangedEvent;
	TurnChangedEvent m_turnChangedEvent;
	PieceCapturedEvent m_pieceCapturedEvent;
	PieceMovedEvent m_pieceMovedEvent;
	PiecePromotedEvent m_piecePromotedEvent;
	WinConditionMetEvent m_winConditionMetEvent;
	AdditionalPieceCaptureRequiredEvent m_additionalPieceCaptureRequiredEvent;
	DisplayHintsRequestedEvent m_displayHintsRequestedEvent;
};

//===============================================================

}
