//---------------------------------------------------------------
//
// ChessLikeView.h
//

#pragma once

#include "ChessLikeView.h"

#include "IGameBoardViewStrategy.h"

namespace Checkers {

//===============================================================

class ChessLikeView final : public IGameBoardViewStrategy {
public:
	ChessLikeView(const ChessLikeView& other) = delete;
	ChessLikeView& operator=(const ChessLikeView& other) = delete;

	ChessLikeView() = default;
	ChessLikeView(ChessLikeView&& other) noexcept = default;
	ChessLikeView& operator=(ChessLikeView&& other) noexcept = default;

	virtual ~ChessLikeView() override;

	// Shows pieces as glyphs. ex. blackPawn= b, blackKing= B, etc.
	[[nodiscard]]
	std::string GetGameBoardDisplayText(const std::vector<Piece>& gameBoard) const override;

	// Translates from chess coordinates that the player sees, to an index our game can use.
	[[nodiscard]]
	int32_t GameBoardPositionToIndex(const std::string& position) const override;
};


//===============================================================

}