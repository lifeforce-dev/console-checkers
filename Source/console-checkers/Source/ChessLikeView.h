//---------------------------------------------------------------
//
// ChessLikeView.h
//

#pragma once


#include "IGameBoardViewStrategy.h"

namespace Checkers {

//===============================================================

class ChessLikeView final : public IGameBoardViewStrategy {
public:
	ChessLikeView(const ChessLikeView& other) = delete;
	ChessLikeView& operator=(const ChessLikeView& other) = delete;
	ChessLikeView(ChessLikeView&& other) noexcept = default;
	ChessLikeView& operator=(ChessLikeView&& other) noexcept = default;

	ChessLikeView();
	virtual ~ChessLikeView() override;

	// IGameViewViewStrategy impl

	// Shows pieces as glyphs. ex. blackPawn= b, blackKing= B, etc.
	std::string GetGameBoardDisplayText(const std::vector<Piece>& gameBoard,
		const std::vector<int32_t>& hintIndices) const override;

	// Translates from chess coordinates that the player sees, to an index our game can use.
	int32_t GetGameBoardIndexFromInput(const std::string& position) const override;

	// This view deals in chess like coordinates for piece positions.
	std::string_view GetMoveCommandSyntax() const override;

	// Returns our Id.
	GameBoardViewStrategyId GetId() const override { return m_id; }

private:
	// No one should ever set this besides us right here.
	GameBoardViewStrategyId m_id = GameBoardViewStrategyId::ChessLikeView;
};

//===============================================================

}