//---------------------------------------------------------------
//
// PlayerState.h
//

#pragma once

#include "GameTypes.h"

namespace Checkers {

//===============================================================

class PlayerState
{
public:
	PlayerState(const PlayerState& other) = delete;
	PlayerState& operator=(const PlayerState& other) = delete;

	PlayerState(PlayerState&& other) noexcept = default;
	PlayerState& operator=(PlayerState&& other) noexcept = default;
	PlayerState(Identity identity);
	~PlayerState();

	// Returns a view of piece that this player specifically owns. we use auto because...
	size_t GetPiecesCount(const std::vector<Piece>& gameBoard) const;

	// Returns our list of captured pieces.
	const std::vector<Piece>& GetCapturedPieces() const;

	// Returns our identity.
	Identity GetIdentity() const { return m_identity; }

	// Adds the piece we captured to our collection.
	void CapturePiece(const Piece& piece);

private:
	// List of pieces we've captured.
	std::vector<Piece> m_capturedPieces;

	// Our identity aligns with the pieces we're allowed to interact with and own.
	Identity m_identity = Identity::Neutral;

};

//===============================================================

}
