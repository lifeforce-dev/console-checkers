//---------------------------------------------------------------
//
// GameState.cpp
//

#include "Game.h"

#include "GameSettings.h"
#include "GameState.h"
#include "GameTypes.h"
#include "MoveDiscoveryEngine.h"
#include "PlayerState.h"
#include "Utility.h"

#include <spdlog/spdlog.h>
// Magic include needed for spdlog to log a custom type.
// Must be included after spdlog.h. Thanks spdlog.
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/ranges.h>

#include <deque>
#include <ranges>

namespace Checkers {

//===============================================================


GameState::GameState(Game* game)
	: m_gameBoard(GameplaySettings::s_defaultGameBoard.size())
	, m_game(game)
	, m_moveDiscoveryEngine(std::make_unique<MoveDiscoveryEngine>(this))
{
	// Creates both players.
	m_playerStates.push_back(std::make_unique<PlayerState>(Identity::Red));
	m_playerStates.push_back(std::make_unique<PlayerState>(Identity::Black));

	// Init our board with what we have in settings. This helps with debugging as well.
	std::ranges::copy(GameplaySettings::s_defaultGameBoard,
		std::begin(m_gameBoard));
	static int32_t s_uid = 0;
	for (auto& piece : m_gameBoard)
	{
		piece.uid = s_uid ++;
	}
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

std::vector<int32_t> GameState::GetBestHintIndices() const
{
	return m_isHintsEnabled ? m_moveDiscoveryEngine->GetBestHintIndices() : std::vector<int32_t>{};
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
	// We should 100% have perfectly valid indices and pieces from here out. If not, I wanna know.
	assert(sourcePiece);
#endif

	// Cache our source piece to retain context.
	const Piece movedPiece = *sourcePiece;

	const int32_t distance = Utility::GetDistanceFromSourceIndex(moveDescription.sourceIndex, moveDescription.destIndex);
	// We can tell by the distance between source/dest if this is a simple move or a capture.
	const bool isMove = distance == GameplaySettings::s_moveDistance;
	const bool isCapture = distance == GameplaySettings::s_captureDistance;

	if (isMove)
	{
		PerformMove(moveDescription, movedPiece);
	}
	else if (isCapture)
	{
		// Player has committed to moving this piece, and will be locked into its capture chain.
		m_touchedCapturingPiece = *sourcePiece;
		PerformCapture(moveDescription, movedPiece);

		// If a capture happened we need to see if we have any more available.
		m_moveDiscoveryEngine->ResetDiscoveredMoves();
		PopulateAvailableMoves();
	}

	// We just moved a piece, does it need to be kinged?
	if (ShouldKingPiece(moveDescription.destIndex, movedPiece.identity))
	{
		PromotePiece(moveDescription.destIndex, movedPiece);
	}

	if (CheckAndNotifyWinCondition(WinConditionReason::AllEnemyPiecesCapturedWin) ||
		CheckAndNotifyWinCondition(WinConditionReason::GameStateViolationDraw))
	{
		return;
	}

	const auto& relevantCaptureList = m_touchedCapturingPiece.has_value() ?
		m_moveDiscoveryEngine->GetAvailableCapturesForTouchedPiece() :
		m_moveDiscoveryEngine->GetAvailableCaptures();

	if (!relevantCaptureList.empty())
	{
		m_game->GetUIEvents().GetAdditionalPieceCaptureRequiredEvent().notify();
		return;
	}

	ToggleTurnPlayer();
}

void GameState::ScopedActivateHintsAndNotify()
{
	m_isHintsEnabled = true;
	m_game->GetUIEvents().GetDisplayHintRequestedEvent().notify();
	m_isHintsEnabled = false;
}

bool GameState::IsTouchedPiece(int32_t sourceIndex) const
{
	if (!Utility::IsValidGameBoardIndex(sourceIndex))
	{
		return false;
	}

	return m_touchedCapturingPiece.has_value() && m_gameBoard[sourceIndex] == m_touchedCapturingPiece.value();
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
	if (m_moveDiscoveryEngine->IsAnyCaptureAvailable())
	{
		//If we have already chose a piece, we must choose a capture from its capture chain.
		if (HasPlayerTouchedPiece() && !m_moveDiscoveryEngine->HasCaptureForTouchedPiece(moveDescription))
		{
			spdlog::warn("Player attempted to move when capture with multiple pieces. move={}", moveDescription);
			m_game->GetUIEvents().GetGameplayErrorPromptRequestedEvent().notify("You must continue capturing with the same piece.");
			return false;
		}

		if (!m_moveDiscoveryEngine->HasCapture(moveDescription))
		{
			spdlog::warn("Player attempted to move when capture required. move={}", moveDescription);
			m_game->GetUIEvents().GetGameplayErrorPromptRequestedEvent().notify("A capture is available, you must take it.");
			return false;
		}
	}
	else if (!m_moveDiscoveryEngine->HasMove(moveDescription))
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

	const bool shouldKingRedPiece = pieceIdentity == Identity::Red && currentLocationCoord.x == GameplaySettings::s_redKingRankIndex;
	const bool shouldKingBlackPiece = pieceIdentity == Identity::Black && currentLocationCoord.x == GameplaySettings::s_blackKingRankIndex;
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

const Piece* GameState::GetPieceFromCoord(const glm::ivec2& coord) const
{
	if (!Utility::IsValidGameBoardCoord(coord))
	{
		return nullptr;
	}

	return &m_gameBoard[Utility::ToGameBoardIndexFromCoord(coord)];
}

void GameState::HandleTurnStart()
{
	m_touchedCapturingPiece.reset();
	m_moveDiscoveryEngine->ResetDiscoveredMoves();

	PopulateAvailableMoves();

	CheckAndNotifyWinCondition(WinConditionReason::NoAvailableMovesLoss);
}

void GameState::PopulateAvailableMoves()
{
	// for every player piece
	for (int32_t i = 0; i < m_gameBoard.size(); ++i)
	{
		if (m_gameBoard[i].identity == m_turnPlayerIdentity)
		{
			m_moveDiscoveryEngine->DiscoverMovesForSourceIndex(i);
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

bool GameState::CheckAndNotifyWinCondition(WinConditionReason reason)
{
	switch (reason)
	{
	case WinConditionReason::AllEnemyPiecesCapturedWin:
		{
			const std::size_t opponentPiecesCount = GetOpponentPlayerState()->GetPiecesCount(m_gameBoard);
			if (opponentPiecesCount == 0)
			{
				m_winConditionState = WinConditionReason::AllEnemyPiecesCapturedWin;
				m_game->GetUIEvents().GetWinConditionMetEvent().notify(m_winConditionState);
			}
		}
		return true;
	case WinConditionReason::NoAvailableMovesLoss:
		{
			if (!m_moveDiscoveryEngine->IsAnyMoveOrCaptureAvailable())
			{
				m_winConditionState = WinConditionReason::NoAvailableMovesLoss;
				m_game->GetUIEvents().GetWinConditionMetEvent().notify(m_winConditionState);
			}
		}
		return true;
	case WinConditionReason::GameStateViolationDraw:
		{
			// Our game state just changed. We'll hash it and keep a count of any duplicates.
			std::size_t gameBoardHash = GenerateGameBoardHash();
			spdlog::info("hash:{}", gameBoardHash);
			int& occurrenceCount = m_gameStateOccurrences[{m_turnPlayerIdentity, gameBoardHash}];
			occurrenceCount++;
			if (occurrenceCount == GameplaySettings::s_gameStateRuleCount)
			{
				m_winConditionState = WinConditionReason::GameStateViolationDraw;
				m_game->GetUIEvents().GetWinConditionMetEvent().notify(m_winConditionState);
			}
		}
		return true;
	// fallthrough
	case WinConditionReason::None:
	default:
		return false;
	}
}



//===============================================================

}
