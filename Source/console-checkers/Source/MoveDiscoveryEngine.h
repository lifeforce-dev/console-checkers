//---------------------------------------------------------------
//
// MoveDiscoveryEngine.h
//

#pragma once

#include "GameTypes.h"

#include <deque>
#include <unordered_set>

namespace Checkers {

//===============================================================

class GameState;
class MoveDiscoveryEngine {

public:
	MoveDiscoveryEngine(const MoveDiscoveryEngine& other) = delete;
	MoveDiscoveryEngine(MoveDiscoveryEngine&& other) noexcept = default;
	MoveDiscoveryEngine& operator=(const MoveDiscoveryEngine& other) = delete;
	MoveDiscoveryEngine& operator=(MoveDiscoveryEngine&& other) noexcept = default;

	~MoveDiscoveryEngine();
	MoveDiscoveryEngine(GameState* state);

	// Returns the best known set of moves for the current player to take.
	std::vector<int32_t> GetBestHintIndices() const;

	// Clears every list of discovered moves.
	void ResetDiscoveredMoves();

	// TODO: This function needs a refactor due to its complexity.
	// Given a sourceIndex, discovers all possilble moves, captures, and builds the maximal capture chain.
	void DiscoverMovesForSourceIndex(int32_t sourceIndex);

	// Returns all known capture hashes for the touched piece.
	const std::unordered_set<PieceMoveDescription, PieceMoveDescriptionHash>& GetAvailableCapturesForTouchedPiece() const;

	// Returns all known capture hashes.
	const std::unordered_set<PieceMoveDescription, PieceMoveDescriptionHash>& GetAvailableCaptures() const;

	// Returns all known move hashes for the touched piece.
	const std::unordered_set<PieceMoveDescription, PieceMoveDescriptionHash>& GetAvailableMoves() const;

	// Convenience functions
	bool HasMove(const PieceMoveDescription& moveDescription) const;
	bool HasCapture(const PieceMoveDescription& moveDescription) const;
	bool HasCaptureForTouchedPiece(const PieceMoveDescription& moveDescription) const;
	bool IsAnyMoveOrCaptureAvailable() const { return IsAnyMoveAvailable() || IsAnyCaptureAvailable(); }
	bool IsAnyMoveAvailable() const { return !m_availableMoves.empty(); }
	bool IsAnyCaptureAvailable() const { return !m_availableCaptures.empty(); }

private:
	// Holds relevant state at the time of creation so it can properly evaluate a move.
	struct MoveEvaluationContext
	{
		// The current source coord that we're looking from.
		glm::ivec2 sourceCoord;

		// The direction we are looking towards, relative to the source.
		glm::ivec2 direction;

		// If this move causes a capture, we need to cache that capture here to account for the board state change.
		std::vector<int32_t> capturedPieceIndices;

		// The list of moves leading up to the end of a capture chain at the point this object was created.
		std::vector<PieceMoveDescription> currentCaptureChain;

		// Which player does the source piece belong to.
		Identity sourceIdentity;

		// TODO: there is probably a bug where if a piece is kinged in the middle of a
		// capture chain, we don't account for that in our search, and so the additional directions
		// won't be checked.
	};

	// Returns true if the MoveEvaluationContext describes simply moving a piece to an empty location.
	bool DoesDescribeMove(const MoveEvaluationContext& moveEval) const;

	// Returns true if the MoveEvaluationContext describes capturing a piece.
	bool DoesDescribeCapture(const MoveEvaluationContext& moveEval) const;

	// Returns a string representation for MoveEvaluationContext
	std::string MoveEvalDescToString(const MoveEvaluationContext& desc) const;

	// Returns a string representation for list of MoveEvaluationContext
	std::string MoveEvalDescStackToString(const std::deque<MoveEvaluationContext>& desc) const;

	// Used in a DFS to find all available moves, captures, and the maximal capture chain.
	std::deque<MoveEvaluationContext> m_moveStateStack;

	// Contains the list of moves a player available that results in the highest number of captures.
	std::vector<PieceMoveDescription> m_maximalCaptureChain;

	// Used to quickly check if a particular move given to us is a valid one.
	std::unordered_set<PieceMoveDescription, PieceMoveDescriptionHash> m_availableMoves;

	// Cache of available captures associated with the piece a player has chosen to capture with.
	std::unordered_set<PieceMoveDescription, PieceMoveDescriptionHash> m_touchedPieceAvailableCaptures;

	// Used to quickly check if a particular capture given to us is a valid one.
	std::unordered_set<PieceMoveDescription, PieceMoveDescriptionHash> m_availableCaptures;

	// List of Moves and their capture chains so that we can offer hints to the player.
	std::vector<PieceMoveHint> m_moveHints;

	// ptr to our parent. In order to discover moves, we need to be able to ask questions on the game state.
	GameState* m_gameState = nullptr;
};

//===============================================================

}
