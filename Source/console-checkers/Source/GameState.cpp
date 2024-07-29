//---------------------------------------------------------------
//
// GameState.cpp
//

#include "Game.h"

#include "GameSettings.h"
#include "GameState.h"
#include "GameTypes.h"
#include "PlayerState.h"
#include "Utility.h"

#include <deque>

#include <spdlog/spdlog.h>
// Magic include needed for spdlog to log a custom type.
// Must be included after spdlog.h. Thanks spdlog.
#include <spdlog/fmt/ostr.h>

namespace Checkers {

//===============================================================

GameState::GameState(Game* game)
	: m_gameBoard(GameplaySettings::s_defaultGameBoard.size())
	, m_game(game)
{
	// Creates both players.
	m_playerStates.push_back(std::make_unique<PlayerState>(Identity::Red));
	m_playerStates.push_back(std::make_unique<PlayerState>(Identity::Black));

	// Init our board with what we have in settings. This helps with debugging as well.
	std::ranges::copy(GameplaySettings::s_defaultGameBoard,
		std::begin(m_gameBoard));
}

GameState::~GameState() = default;

const std::vector<Piece>& GameState::GetRedPlayerCapturedPieces() const
{
	return GetCapturedPiecesForPlayerId(Identity::Red);
}

const std::vector<Piece>& GameState::GetBlackPlayerCapturedPieces() const
{
	return GetCapturedPiecesForPlayerId(Identity::Black);
}

void GameState::ToggleTurnPlayer()
{
	Identity oldTurnPlayer = m_turnPlayerIdentity;

	if (m_turnPlayerIdentity == Identity::Neutral)
	{
		// First turn has started, red always goes first.
		m_turnPlayerIdentity = Identity::Red;
	}
	else
	{
		m_turnPlayerIdentity = (m_turnPlayerIdentity == Identity::Red) ? Identity::Black : Identity::Red;
	}
	m_game->GetUIEvents().GetTurnChangedEvent().notify();
	spdlog::info("Player turn change. old {} player new {} player", oldTurnPlayer, m_turnPlayerIdentity);

	HandleTurnStart();
}

void GameState::MovePiece(const PieceMoveDescription& moveDescription)
{
	if (!IsMoveDescriptionValid(moveDescription))
	{
		return;
	}

	const Piece* sourcePiece = GetPieceFromCoord(Utility::ToGameBoardCoordFromIndex(moveDescription.sourceIndex));
#ifdef DEBUG
	const Piece* destPiece = GetPieceFromCoord(Utility::ToGameBoardCoordFromIndex(moveDescription.destIndex));

	// We should 100% have perfectly valid indices and pieces from here out. If not, I wanna know.
	assert(sourcePiece && destPiece);
#endif

	int32_t distance = Utility::GetDistanceFromSourceIndex(moveDescription.sourceIndex, moveDescription.destIndex);

	// Cache our source piece to retain context.
	const Piece movedPiece = *sourcePiece;

	// We can tell by the distance between source/dest if this is a simple move or a capture.
	const bool isMove = distance == GameplaySettings::s_moveDistance;
	const bool isCapture = distance == GameplaySettings::s_captureDistance;

	if (isMove)
	{
		PerformMove(moveDescription, movedPiece);
	}
	else if (isCapture)
	{
		PerformCapture(moveDescription, movedPiece);

		// If a capture happened we need to see if we have any more available.
		PopulateAvailableMoves();
	}

	// Do this after both capture and move
	if (ShouldKingPiece(moveDescription.destIndex, movedPiece.identity))
	{
		PromotePiece(moveDescription.destIndex, movedPiece);
	}

	const std::size_t opponentPiecesCount = GetOpponentPlayerState()->GetPiecesCount(m_gameBoard);
	if (opponentPiecesCount == 0)
	{
		m_winConditionState = WinConditionReason::AllEnemyPiecesCapturedWin;
		m_game->GetUIEvents().GetWinConditionMetEvent().notify(m_winConditionState);
		return;
	}

	// Our game state just changed. We'll hash it and keep a count of any duplicates.
	std::size_t gameBoardHash = GenerateGameBoardHash();
	spdlog::info("hash:{}", gameBoardHash);
	int& occurrenceCount = m_gameStateOccurrences[{m_turnPlayerIdentity, gameBoardHash}];
	occurrenceCount++;
	if (occurrenceCount == 3)
	{
		m_winConditionState = WinConditionReason::GameStateViolationDraw;
		m_game->GetUIEvents().GetWinConditionMetEvent().notify(m_winConditionState);
	}

	if (!m_availableCaptures.empty())
	{
		m_game->GetUIEvents().GetAdditionalPieceCaptureRequiredEvent().notify();
		return;
	}

	ToggleTurnPlayer();
}

void GameState::PerformMove(const PieceMoveDescription& moveDescription, const Piece& movedPiece)
{
	m_gameBoard[moveDescription.sourceIndex] = GameBoardStatics::s_emptyPiece;
	m_gameBoard[moveDescription.destIndex] = movedPiece;
	m_game->GetUIEvents().GetPieceMovedEvent();
}

void GameState::PerformCapture(const PieceMoveDescription& moveDescription, const Piece& movedPiece)
{
	// Capture needs to clean up a middle piece.
	m_gameBoard[moveDescription.sourceIndex] = GameBoardStatics::s_emptyPiece;
	m_gameBoard[moveDescription.destIndex] = movedPiece;

	const glm::ivec2 sourceToDestDirection = Utility::GetDirectionBetweenTwoIndices(
		moveDescription.sourceIndex,
		moveDescription.destIndex);

	const glm::ivec2 middlePieceCoord = Utility::GetMoveDestCoord(
		Utility::ToGameBoardCoordFromIndex(moveDescription.sourceIndex),
		sourceToDestDirection);

	GetPlayerStateForId(m_turnPlayerIdentity)->CapturePiece(*GetPieceFromCoord(middlePieceCoord));
	m_gameBoard[Utility::ToGameBoardIndexFromCoord(middlePieceCoord)] = GameBoardStatics::s_emptyPiece;

	m_game->GetUIEvents().GetPieceCapturedEvent().notify();
}

void GameState::PromotePiece(int32_t destIndex, const Piece& movedPiece)
{
	m_gameBoard[destIndex] = movedPiece.identity == Identity::Red ?
		GameBoardStatics::s_redKing :
		GameBoardStatics::s_blackKing;

	m_game->GetUIEvents().GetPiecePromotedEvent();
}

bool GameState::IsMoveDescriptionValid(const PieceMoveDescription& moveDescription)
{
	// If we have available captures, we MUST take them and should not even consider available moves.
	if (!m_availableCaptures.empty())
	{
		if (!m_availableCaptures.contains(moveDescription))
		{
			spdlog::warn("Player attempted to move when capture required. move={}", moveDescription);
			m_game->GetUIEvents().GetGameplayErrorPromptRequestedEvent().notify("A capture is available, you must take it.");
			return false;
		}
	}
	else if (!m_availableMoves.contains(moveDescription))
	{
		spdlog::warn("Player attempted unlisted move. move={}", moveDescription);
		m_game->GetUIEvents().GetGameplayErrorPromptRequestedEvent().notify("This is not a valid move. Choose a different one.");
		return false;
	}

	return true;
}

bool GameState::ShouldKingPiece(int32_t locationIndex, Identity pieceIdentity) const
{
	glm::ivec2 currentLocationCoord = Utility::ToGameBoardCoordFromIndex(locationIndex);

	const bool shouldKingRedPiece = pieceIdentity == Identity::Red && currentLocationCoord.x == GameplaySettings::redKingRankIndex;
	const bool shouldKingBlackPiece = pieceIdentity == Identity::Black && currentLocationCoord.x == GameplaySettings::blackKingRankIndex;
	return shouldKingRedPiece || shouldKingBlackPiece;
}

const std::vector<Piece>& GameState::GetCapturedPiecesForPlayerId(Identity playerId) const
{
	const PlayerState* player = GetPlayerStateForId(playerId);

#ifdef DEBUG
	// If we're here we didn't set the game up properly.
	assert(player);
#endif

	return player->GetCapturedPieces();
}

bool GameState::IsPieceControlledAtCoord(const glm::ivec2& boardCoord)
{
	if (!Utility::IsValidGameBoardCoord(boardCoord))
	{
		return false;
	}

	const Piece* piece = GetPieceFromCoord(boardCoord);
	return piece->identity == Identity::Red || piece->identity == Identity::Black;
}

bool GameState::DoesDescribeMove(const MoveEvalDescription& moveEval) const
{
	const glm::ivec2 destCoord = Utility::GetMoveDestCoord(moveEval.sourceCoord, moveEval.direction);
	if (!Utility::IsValidGameBoardCoord(moveEval.sourceCoord) || !Utility::IsValidGameBoardCoord(destCoord))
	{
		return false;
	}

	const Piece* destPiece = GetPieceFromCoord(destCoord);

#ifdef DEBUG
	// We just checked that the dest is a valid location on the board.
	assert(destPiece);
#endif

	return destPiece->pieceType == PieceType::Empty;
}

bool GameState::DoesDescribeCapture(const MoveEvalDescription& moveEval) const
{
	const glm::ivec2 destCoord = Utility::GetCaptureDestCoord(moveEval.sourceCoord, moveEval.direction);
	if (!Utility::IsValidGameBoardCoord(moveEval.sourceCoord) || !Utility::IsValidGameBoardCoord(destCoord))
	{
		return false;
	}

	// In order for this to be a valid capture, dest needs to be valid, and the piece in between dest and source
	// must be an enemy.
	const glm::ivec2 middlePieceCoord = Utility::GetMoveDestCoord(moveEval.sourceCoord, moveEval.direction);

	const Piece* middlePiece = GetPieceFromCoord(middlePieceCoord);
	const Piece* sourcePiece = GetPieceFromCoord(moveEval.sourceCoord);
	const Piece* destPiece = GetPieceFromCoord(destCoord);

#ifdef DEBUG
	// We just checked that these are both valid locations on the board.
	assert(sourcePiece && destPiece);

	// Middle piece couldn't possibly not be on the board if dest is.
	assert(middlePiece);
#endif

	return GetPieceAffinity(*sourcePiece, *middlePiece) == Affinity::Enemy && destPiece->pieceType == PieceType::Empty;

}

const Piece* GameState::GetPieceFromCoord(const glm::ivec2& coord) const
{
	if (!Utility::IsValidGameBoardCoord(coord))
	{
		return nullptr;
	}

	return &m_gameBoard[Utility::ToGameBoardIndexFromCoord(coord)];
}

std::span<const glm::ivec2> GameState::GetDirectionsForPiece(const Piece& piece) const
{
	if (piece == GameBoardStatics::s_redPawn)
	{
		return GameplaySettings::s_redPawnDirections;
	}
	if (piece == GameBoardStatics::s_blackPawn)
	{
		return GameplaySettings::s_blackPawnDirections;
	}
	if (piece == GameBoardStatics::s_blackKing || piece == GameBoardStatics::s_redKing)
	{
		return GameplaySettings::s_kingDirections;
	}
#ifdef DEBUG
	spdlog::error("Attempted to get directions for an unknown piece. piece={}", piece);
	// I want to know if we get here, it means something is wrong with our equality check.
	assert(false);
#endif

	return {};
}

void GameState::HandleTurnStart()
{
	PopulateAvailableMoves();

	if (m_availableCaptures.empty() && m_availableMoves.empty())
	{
		m_winConditionState = WinConditionReason::NoAvailableMovesLoss;
		m_game->GetUIEvents().GetWinConditionMetEvent().notify(m_winConditionState);
	}

}

void GameState::PopulateAvailableMoves()
{
	m_availableCaptures.clear();
	m_availableMoves.clear();

	// for every player piece
	for (int32_t i = 0; i < m_gameBoard.size(); ++i)
	{
		if (m_gameBoard[i].identity == m_turnPlayerIdentity)
		{
			AddAvailableMovesForPieceIndex(i);
		}
	}
}

PlayerState* GameState::GetTurnPlayerState() const
{
	return GetPlayerStateForId(m_turnPlayerIdentity);
}

PlayerState* GameState::GetOpponentPlayerState() const
{
	const Identity opponentId = m_turnPlayerIdentity == Identity::Red ? Identity::Black : Identity::Red;
	return GetPlayerStateForId(opponentId);
}

PlayerState* GameState::GetPlayerStateForId(Identity id) const
{
	auto it = std::find_if(std::begin(m_playerStates), std::end(m_playerStates),
	[this, id](const std::unique_ptr<PlayerState>& playerState)
	{
		return playerState->GetIdentity() == id;
	});
	if (it == std::end(m_playerStates))
	{
#ifdef DEBUG
		// If this happens it means we're calling this function before the game has started.
		assert(false);
#endif
		return nullptr;
	}

	return it->get();
}

void GameState::AddAvailableMovesForPieceIndex(int32_t source)
{
	if (!Utility::IsValidGameBoardIndex(source))
	{
		return;
	}

	glm::ivec2 sourceCoord = Utility::ToGameBoardCoordFromIndex(source);
	const Piece* sourcePiece = GetPieceFromCoord(sourceCoord);

	// DFS to find all available moves.
	std::deque<MoveEvalDescription> moveTestDeque;
	std::span<const glm::ivec2> directions = GetDirectionsForPiece(*sourcePiece);

	// Initialize our deque
	for (auto direction : directions)
	{
		moveTestDeque.emplace_back(sourceCoord, direction);
	}

	// TODO: DFS to find all possible capture chains and rank them for hints.
	while (!moveTestDeque.empty())
	{
		MoveEvalDescription moveEval = moveTestDeque.front();
		moveTestDeque.pop_front();

		glm::ivec2 sourceCord = moveEval.sourceCoord;
		if (DoesDescribeMove(moveEval))
		{
			glm::ivec2 destCoord = Utility::GetMoveDestCoord(moveEval.sourceCoord, moveEval.direction);
			m_availableMoves.insert({ Utility::ToGameBoardIndexFromCoord(sourceCoord),
				Utility::ToGameBoardIndexFromCoord(destCoord) });
		}
		else if(DoesDescribeCapture(moveEval))
		{
			glm::ivec2 destCoord = Utility::GetCaptureDestCoord(moveEval.sourceCoord, moveEval.direction);
			m_availableCaptures.insert({ Utility::ToGameBoardIndexFromCoord(sourceCoord),
				Utility::ToGameBoardIndexFromCoord(destCoord) });
		}
	}
}

Affinity GameState::GetPieceAffinity(const Piece& sourcePiece, const Piece& destPiece) const
{
	if (sourcePiece.identity == Identity::Neutral || destPiece.identity == Identity::Neutral)
	{
		return Affinity::Neutral;
	}

	return sourcePiece.identity == destPiece.identity ? Affinity::Friendly : Affinity::Enemy;
}

std::size_t GameState::GenerateGameBoardHash() const
{
	std::size_t hash = 0;

	hash_combine(hash, static_cast<int32_t>(GetTurnPlayerId()));
	for (std::size_t i = 0; i < m_gameBoard.size(); ++i)
	{
		hash_combine(hash, m_gameBoard[i]);
		hash_combine(hash, i);
	}
	return hash;
}

//===============================================================

}
