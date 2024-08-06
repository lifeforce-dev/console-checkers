//---------------------------------------------------------------
//
// GameState.h
//

#pragma once

#include "IGameStateDisplayInfo.h"

#include "GameTypes.h"
#include "Utility.h"

#include <optional>
#include <vector>

namespace Checkers {

//===============================================================

class Game;
class PlayerState;
class MoveDiscoveryEngine;
class GameState : public IGameStateDisplayInfo
{
public:

	GameState(const GameState& other) = delete;
	GameState& operator=(const GameState& other) = delete;
	GameState(GameState&& other) noexcept = default;
	GameState& operator=(GameState&& other) noexcept = default;

	GameState(Game* game);
	virtual ~GameState() override;

	// IGameStateDisplayInfo impl
	const std::vector<Piece>& GetGameBoardData() const override { return m_gameBoard; }
	const std::vector<Piece>& GetRedPlayerCapturedPieces() const override;
	const std::vector<Piece>& GetBlackPlayerCapturedPieces() const override;
	const Identity GetTurnPlayerId() const override { return m_turnPlayerIdentity; }
	std::vector<int32_t> GetBestHintIndices() const override;

	// Anything other than None indicates a win condition has been met, and describes how.
	WinConditionReason GetWinState() const { return m_winConditionState; }

	// Changes the turn player back and forth, or sets the initial player when called for the first time.
	void ToggleTurnPlayer();

	// Takes the given move description and attempts to move there given the rules of Checkers.
	void MovePiece(const PieceMoveDescription& moveDescription);

	// Turns on hints, notifies, then turns off hints.
	void ScopedActivateHintsAndNotify();

	// Returns whether the piece at the source index is our touched piece.
	bool IsTouchedPiece(int32_t sourceIndex) const;

	// Returns whether the player has touched a piece yet for this turn.
	// In checkers, when facing multiple pieces that can capture, the player may only choose one piece to execute the
	// capture chain of.
	bool HasPlayerTouchedPiece() const { return m_touchedCapturingPiece.has_value(); }

	// Returns the Piece at the given board coord.
	const Piece* GetPieceFromCoord(const glm::ivec2& coord) const;

	// Given two pieces, checks and returns their affinity.
	Affinity GetPieceAffinity(const Identity sourcePieceIdentity, const Identity destPieceIdentity) const;

private:
	// When a turn changes, we need to evaluate the board to prepare for move requests, among other things.
	void PopulateAvailableMoves();

	// Moves a piece and updates the game state.
	void PerformMove(const PieceMoveDescription& moveDescription, const Piece& movedPiece);

	// Captures a piece and updates the game state.
	void PerformCapture(const PieceMoveDescription& moveDescription, const Piece& movedPiece);

	// Promotes the piece at the given position to a king.
	void PromotePiece(int32_t destIndex, const Piece& movedPiece);

	// Checks the various conditions that could reject a move request.
	bool IsMoveDescriptionValid(const PieceMoveDescription& moveDescription);

	// A piece should be promoted if they're in the correct spot on the board.
	bool ShouldKingPiece(int32_t locationIndex, Identity pieceIdentity) const;

	// Given an ID returns the list of captured pieces recorded in that player state.
	const std::vector<Piece>& GetCapturedPiecesForPlayerId(Identity playerId) const;

	// Returns true if the piece at the given location is controlled by either player.
	bool IsPieceControlledAtCoord(const glm::ivec2& boardCoord);

	// When the turn starts, perform some bookkeeping.
	void HandleTurnStart();

	// Gets the player state object for the current player.
	PlayerState* GetTurnPlayerState() const;

	// Gets the player state object for the enemy player.
	PlayerState* GetOpponentPlayerState() const;

	// Returns the player state for a given identity.
	PlayerState* GetPlayerStateForId(Identity id) const;

	// Generates a hash of this game state at the time this function was called.
	std::size_t GenerateGameBoardHash() const;

	// Checks to see if our state satisfies the given win condition reasons, notifies and returns true if so.
	bool CheckAndNotifyWinCondition(WinConditionReason reason);

private:

	// Each player has a state object with info specific to that player. This list holds each player.
	std::vector<std::unique_ptr<PlayerState>> m_playerStates;

	// Builds and manages our understanding of available moves and captures for each piece and player.
	std::unique_ptr<MoveDiscoveryEngine> m_moveDiscoveryEngine = nullptr;

	// Indicates whose turn it currently is.
	Identity m_turnPlayerIdentity = Identity::Neutral;

	// The checkers board data.
	std::vector<Piece> m_gameBoard;

	// Current win state of the game
	WinConditionReason m_winConditionState = WinConditionReason::None;

	// Pointer to our parent;
	Game* m_game = nullptr;

	// Keeps track of how many times a specific game state has occurred.
	std::unordered_map<GameStateRecord, int32_t, GameStateRecordHash> m_gameStateOccurrences;

	// For capture chains, once a player chooses a piece, they may only move that piece.
	// This is to handle the case of multiple pieces that can capture.
	std::optional<Piece> m_touchedCapturingPiece;

	// When true, hints will be displayed to the player.
	bool m_isHintsEnabled = false;
};

//===============================================================
}
