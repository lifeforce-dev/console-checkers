//---------------------------------------------------------------
//
// GameState.h
//

#pragma once

#include "IGameStateDisplayInfo.h"

#include "GameTypes.h"
#include "Utility.h"

#include <optional>
#include <span>
#include <string>
#include <unordered_set>
#include <vector>

namespace Checkers {

//===============================================================

class Game;
class PlayerState;

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

private:

	// Describes our test for whether we can move a piece.
	struct MoveEvalDescription
	{
		// From where are we evaluating a move?
		glm::ivec2 sourceCoord;

		// Which direction are we going to try and move?
		glm::ivec2 direction;

		// If this move causes a capture, we need to cache that capture here to account for the board state change.
		std::vector<int32_t> capturedPieceIndices;

		// Record of how we got here.
		std::vector<PieceMoveDescription> currentCaptureChain;

		// Which player does the source piece belong to.
		Identity sourceIdentity;

		// TODO: there is probably a bug where if a piece is kinged in the middle of a
		// capture chain, we don't account for that in our search, and so the additional directions
		// won't be checked.
	};

	// Returns a string representation for MoveEvalDescription
	std::string MoveEvalDescToString(const MoveEvalDescription& desc) const;

	// Returns a string representation for list of MoveEvalDescription
	std::string MoveEvalDescStackToString(const std::deque<MoveEvalDescription>& desc) const;

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

	// Returns true if the MoveEvalDescription describes simply moving a piece to an empty location.
	bool DoesDescribeMove(const MoveEvalDescription& moveEval) const;

	// Returns true if the MoveEvalDescription describes capturing a piece.
	bool DoesDescribeCapture(const MoveEvalDescription& moveEval) const;

	// Returns the Piece at the given board coord.
	const Piece* GetPieceFromCoord(const glm::ivec2& coord) const;

	// Given a piece, returns a view of all directions it is allowed to move.
	std::span<const glm::ivec2> GetDirectionsForPiece(const Piece& piece) const;

	// When the turn starts, perform some bookkeeping.
	void HandleTurnStart();

	// When a turn changes, we need to evaluate the board to prepare for move requests, among other things.
	void PopulateAvailableMoves();

	// Gets the player state object for the current player.
	PlayerState* GetTurnPlayerState() const;

	// Gets the player state object for the enemy player.
	PlayerState* GetOpponentPlayerState() const;

	// Returns the player state for a given identity.
	PlayerState* GetPlayerStateForId(Identity id) const;

	// TODO: This function needs a refactor due to its complexity.
	// Given a source, discovers all possilble moves, captures, and builds the maximal capture chain.
	void AddAvailableMovesForPieceIndex(int32_t source);

	// Given two pieces, checks and returns their affinity.
	Affinity GetPieceAffinity(const Identity sourcePieceIdentity, const Identity destPieceIdentity) const;

	// Generates a hash of this game state at the time this function was called.
	std::size_t GenerateGameBoardHash() const;

private:

	// Each player has a state object with info specific to that player. This list holds each player.
	std::vector<std::unique_ptr<PlayerState>> m_playerStates;

	// Indicates whose turn it currently is.
	Identity m_turnPlayerIdentity = Identity::Neutral;

	// The checkers board data.
	std::vector<Piece> m_gameBoard;

	// Used to quickly check if a particular move given to us is a valid one.
	std::unordered_set<PieceMoveDescription, PieceMoveDescriptionHash> m_availableMoves;

	// Cache of available captures associated with the piece a player has chosen to capture with.
	std::unordered_set<PieceMoveDescription, PieceMoveDescriptionHash> m_touchedPieceAvailableCaptures;

	// Used to quickly check if a particular capture given to us is a valid one.
	std::unordered_set<PieceMoveDescription, PieceMoveDescriptionHash> m_availableCaptures;

	// List of Moves and their capture chains so that we can offer hints to the player.
	std::vector<PieceMoveHint> m_moveHints;

	// Current win state of the game
	WinConditionReason m_winConditionState = WinConditionReason::None;

	// Pointer to our parent;
	Game* m_game = nullptr;

	// Keeps track of how many times a specific game state has occurred.
	std::unordered_map<GameStateRecord, int32_t, GameStateRecordHash> m_gameStateOccurrences;

	// When true, hints will be displayed to the player.
	bool m_isHintsEnabled = false;

	// For capture chains, once a player chooses a piece, they may only move that piece.
	// This is to handle the case of multiple pieces that can capture.
	std::optional<Piece> m_touchedCapturingPiece;
};


//===============================================================
}
