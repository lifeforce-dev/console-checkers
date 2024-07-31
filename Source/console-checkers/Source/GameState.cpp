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
	if (m_moveHints.empty() || !m_isHintsEnabled)
	{
		return {};
	}

	const PieceMoveHint& hint = m_moveHints.front();
	std::vector<int32_t> hintIndices;
	hintIndices.reserve(hint.captureChain.size());

	// If a player isn't currently in the middle of a capture chain, we can just recommend
	// the best chain from the general list. Otherwise, we MUST pick from the touchedPiece's capture chain.
	auto getBestCaptureChain = [this]()
	{
		if (m_touchedCapturingPiece.has_value())
		{
			auto it = std::find_if(m_moveHints.begin(), m_moveHints.end(),
				[this](const PieceMoveHint& hint)
			{
				return m_gameBoard[hint.captureChain.front().sourceIndex] == m_touchedCapturingPiece;
			});

			if (it != m_moveHints.end())
			{
				return it->captureChain;
			}

#ifdef DEBUG
			// Something is terribly wrong.
			assert(false);
#endif
			return std::vector<PieceMoveDescription>{};
		}

		const PieceMoveHint& topHint = m_moveHints.front();
		return topHint.captureChain;
	};

	const auto& relevantCaptureChain = getBestCaptureChain();

	const int32_t captureChainOriginPieceIndex = relevantCaptureChain.front().sourceIndex;
	hintIndices.push_back(captureChainOriginPieceIndex);

	std::ranges::transform(relevantCaptureChain, std::back_inserter(hintIndices),
		[](const auto& capture)
	{
		return capture.destIndex;
	});

	return hintIndices;
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
		// Player has committed to moving this piece, and will be locked into its capture chain.
		m_touchedCapturingPiece = *sourcePiece;
		PerformCapture(moveDescription, movedPiece);

		// TODO: These should probably go in a function if not resetting them all at once is a bug.
		m_availableCaptures.clear();
		m_touchedPieceAvailableCaptures.clear();
		m_moveHints.clear();

		// If a capture happened we need to see if we have any more available.
		PopulateAvailableMoves();
	}

	// We just moved a piece, does it need to be kinged?
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
	if (occurrenceCount == GameplaySettings::s_gameStateRuleCount)
	{
		m_winConditionState = WinConditionReason::GameStateViolationDraw;
		m_game->GetUIEvents().GetWinConditionMetEvent().notify(m_winConditionState);
	}


	const auto& relevantCaptureList = m_touchedCapturingPiece.has_value() ?
		m_touchedPieceAvailableCaptures :
		m_availableCaptures;

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

std::string GameState::MoveEvalDescToString(const MoveEvalDescription& desc) const
{
	return fmt::format("[{} | {} | captured_pieces{}]"
		, Utility::ToGameBoardIndexFromCoord(desc.sourceCoord)
		, Utility::DirectionToString(desc.direction)
		, desc.capturedPieceIndices);
}

std::string GameState::MoveEvalDescStackToString(const std::deque<MoveEvalDescription>& desc) const
{
	auto strings = desc | std::ranges::views::transform([this](const MoveEvalDescription& d)
	{
		return MoveEvalDescToString(d);
	});

	return fmt::format("[{}]", fmt::join(strings, ", "));
	
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
		 //If we have already chose a piece, we must choose a capture from its capture chain.
		if (m_touchedCapturingPiece.has_value() && !m_touchedPieceAvailableCaptures.contains(moveDescription))
		{
			spdlog::warn("Player attempted to move when capture with multiple pieces. move={}", moveDescription);
			m_game->GetUIEvents().GetGameplayErrorPromptRequestedEvent().notify("You must continue capturing with the same piece.");
			return false;
		}

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
	const Piece* destPiece = GetPieceFromCoord(destCoord);

#ifdef DEBUG
	// We just checked that these are both valid locations on the board.
	assert(/*sourcePiece && */destPiece);

	// Middle piece couldn't possibly not be on the board if dest is.
	assert(middlePiece);
#endif

	return GetPieceAffinity(moveEval.sourceIdentity, middlePiece->identity) == Affinity::Enemy && destPiece->pieceType == PieceType::Empty;

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
	m_touchedCapturingPiece.reset();
	m_touchedPieceAvailableCaptures.clear();
	m_availableCaptures.clear();
	m_availableMoves.clear();
	m_moveHints.clear();

	PopulateAvailableMoves();

	if (m_availableCaptures.empty() && m_availableMoves.empty())
	{
		m_winConditionState = WinConditionReason::NoAvailableMovesLoss;
		m_game->GetUIEvents().GetWinConditionMetEvent().notify(m_winConditionState);
	}
}

void GameState::PopulateAvailableMoves()
{
	// for every player piece
	for (int32_t i = 0; i < m_gameBoard.size(); ++i)
	{
		if (m_gameBoard[i].identity == m_turnPlayerIdentity)
		{
			AddAvailableMovesForPieceIndex(i);
		}
	}

	std::ranges::sort(m_moveHints, std::ranges::greater{}, &PieceMoveHint::score);
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
	// Just need some sort of value to reserve some space in our vectors.
	static constexpr int32_t theoreticalMaxCaptureCount = 12;
	std::vector<PieceMoveDescription> maximalCaptureChain;
	maximalCaptureChain.reserve(theoreticalMaxCaptureCount);

	glm::ivec2 sourceCoord = Utility::ToGameBoardCoordFromIndex(source);
	const Piece* sourcePiece = GetPieceFromCoord(sourceCoord);

	std::deque<MoveEvalDescription> evalStateStack;
	std::span<const glm::ivec2> directions = GetDirectionsForPiece(*sourcePiece);
	for (auto direction : directions)
	{
		evalStateStack.emplace_back(sourceCoord, direction, std::vector<int32_t>{}, std::vector<PieceMoveDescription>{}, sourcePiece->identity);
	}

	spdlog::info("Starting search for {} piece at {}", m_turnPlayerIdentity, source);
	// If we found a capture and are looking for more, moves are irrelevant.
	bool isSearchingForAdditionalCaptures = false;
	while (!evalStateStack.empty())
	{
		spdlog::info(MoveEvalDescStackToString(evalStateStack));
		MoveEvalDescription moveEval = evalStateStack.back();
		spdlog::info("Popping: {}", MoveEvalDescToString(moveEval));
		evalStateStack.pop_back();
		if (DoesDescribeMove(moveEval) && !isSearchingForAdditionalCaptures)
		{
			glm::ivec2 destCoord = Utility::GetMoveDestCoord(moveEval.sourceCoord, moveEval.direction);
			m_availableMoves.insert({ Utility::ToGameBoardIndexFromCoord(sourceCoord),
				Utility::ToGameBoardIndexFromCoord(destCoord) });
			 
			spdlog::info("Found Move: {}",Utility::ToGameBoardIndexFromCoord(destCoord));

			PieceMoveHint hint;
			// Basic moves are assigned a score of 0.
			hint.score = 0;
			hint.captureChain.emplace_back(Utility::ToGameBoardIndexFromCoord(sourceCoord), Utility::ToGameBoardIndexFromCoord(destCoord));
			m_moveHints.push_back(std::move(hint));
		}
		else if (DoesDescribeCapture(moveEval))
		{
			// This is where our current move eval landed.
			glm::ivec2 destCoord = Utility::GetCaptureDestCoord(moveEval.sourceCoord, moveEval.direction);

			PieceMoveDescription captureDesc = { Utility::ToGameBoardIndexFromCoord(moveEval.sourceCoord),
				Utility::ToGameBoardIndexFromCoord(destCoord) };
			spdlog::info("landed at {}", Utility::ToGameBoardIndexFromCoord(destCoord));

			// currentCaptureChain records capture. This is necessary to prevent infinite loops.
			moveEval.currentCaptureChain.emplace_back(Utility::ToGameBoardIndexFromCoord(moveEval.sourceCoord),
				Utility::ToGameBoardIndexFromCoord(destCoord));

			spdlog::info("capture_chain {}", moveEval.currentCaptureChain);

			glm::ivec2 middlePieceCoord = Utility::GetMoveDestCoord(moveEval.sourceCoord, moveEval.direction);
			spdlog::info("captured at {}", Utility::ToGameBoardIndexFromCoord(middlePieceCoord));
			// Now we need to check if we can capture anything else to continue the capture chain.
			MoveEvalDescription lookahead;
			lookahead.sourceCoord = destCoord;
			lookahead.sourceIdentity = moveEval.sourceIdentity;
			lookahead.capturedPieceIndices = moveEval.capturedPieceIndices;
			lookahead.capturedPieceIndices.push_back(Utility::ToGameBoardIndexFromCoord(middlePieceCoord));
			lookahead.currentCaptureChain = moveEval.currentCaptureChain;

			bool isLookaheadCaptureAvailable = false;
			for (auto direction : directions)
			{
				lookahead.direction = direction;

				spdlog::info("lookahead checking {} at {}", Utility::DirectionToString(lookahead.direction),
					Utility::ToGameBoardIndexFromCoord(Utility::GetCaptureDestCoord(lookahead.sourceCoord, lookahead.direction)));
				if (DoesDescribeCapture(lookahead))
				{
					middlePieceCoord = Utility::GetMoveDestCoord(lookahead.sourceCoord, lookahead.direction);
					auto it = std::ranges::find(lookahead.capturedPieceIndices,
						Utility::ToGameBoardIndexFromCoord(middlePieceCoord));

					bool hasCapturedPieceAlready = it != lookahead.capturedPieceIndices.end();
					spdlog::info("would capture {}| {}| captured_pieces {}", Utility::ToGameBoardIndexFromCoord(middlePieceCoord),
						!hasCapturedPieceAlready ? "Piece not yet captured." : "Piece already captured.", lookahead.capturedPieceIndices);
					if (!hasCapturedPieceAlready)
					{
						MoveEvalDescription newCaptureEval = { lookahead.sourceCoord, lookahead.direction, lookahead.capturedPieceIndices,
							lookahead.currentCaptureChain, sourcePiece->identity };
						spdlog::info("Adding capture {}", MoveEvalDescToString(newCaptureEval));
						evalStateStack.emplace_back(newCaptureEval);
						isLookaheadCaptureAvailable = true;
					}
				}
			}

			if (!isLookaheadCaptureAvailable)
			{
				if (moveEval.currentCaptureChain.size() > maximalCaptureChain.size())
				{
					maximalCaptureChain = moveEval.currentCaptureChain;
				}
				const PieceMoveDescription& originMove = moveEval.currentCaptureChain[0];
				if (m_touchedCapturingPiece.has_value() && m_gameBoard[originMove.sourceIndex] == m_touchedCapturingPiece.value())
				{
					m_touchedPieceAvailableCaptures.insert({ originMove });
				}
				m_availableCaptures.insert({ originMove });

			}
		}
	}
	// We need to add the source move for our capture chain to the list of available captures.
	if (!maximalCaptureChain.empty())
	{
		m_moveHints.emplace_back(maximalCaptureChain.size(), maximalCaptureChain);
		spdlog::info("maximal capture chain={}", maximalCaptureChain);
	}
}

Affinity GameState::GetPieceAffinity(const Identity sourcePieceIdentity, const Identity destPieceIdentity) const
{
	if (sourcePieceIdentity == Identity::Neutral || destPieceIdentity == Identity::Neutral)
	{
		return Affinity::Neutral;
	}

	return sourcePieceIdentity == destPieceIdentity ? Affinity::Friendly : Affinity::Enemy;
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
