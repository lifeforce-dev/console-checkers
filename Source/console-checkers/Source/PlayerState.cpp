//---------------------------------------------------------------
//
// PlayerState.cpp
//

#include "PlayerState.h"

#include <ranges>

namespace Checkers {

//===============================================================

PlayerState::PlayerState(Identity identity)
	: m_identity(identity)
{
}

PlayerState::~PlayerState() = default;

const std::vector<Piece>& PlayerState::GetCapturedPieces() const
{
	return m_capturedPieces;
}

void PlayerState::CapturePiece(const Piece& piece)
{
	m_capturedPieces.push_back(piece);
}

size_t PlayerState::GetPiecesCount(const std::vector<Piece>& gameBoard) const
{
	auto isMyPiece = [this](const Piece& piece)
	{
		return piece.identity == m_identity;
	};

	auto piecesView = gameBoard | std::ranges::views::filter(isMyPiece);
	return std::ranges::distance(piecesView);
}

//===============================================================

}
