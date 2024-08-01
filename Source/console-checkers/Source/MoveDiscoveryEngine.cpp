//---------------------------------------------------------------
//
// MoveDiscoveryEngine.cpp
//

#include "MoveDiscoveryEngine.h"

#include "GameState.h"
#include "Utility.h"

#include <spdlog/spdlog.h>
// Magic include needed for spdlog to log a custom type.
// Must be included after spdlog.h. Thanks spdlog.
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/ranges.h>

#include <ranges>

namespace Checkers {

//===============================================================

MoveDiscoveryEngine::~MoveDiscoveryEngine() = default;

MoveDiscoveryEngine::MoveDiscoveryEngine(GameState* state)
	: m_gameState(state)
{
	m_maximalCaptureChain.reserve(GameplaySettings::s_theoreticalMaxCaptureCount);
}

std::vector<int32_t> MoveDiscoveryEngine::GetBestHintIndices() const
{
	// Scoring:
	// Basic Moves: 0
	// Captures: score equals size of the capture list.

	if (m_moveHints.empty())
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
		if (m_gameState->HasPlayerTouchedPiece())
		{
			auto it = std::find_if(m_moveHints.begin(), m_moveHints.end(),
				[this](const PieceMoveHint& hint)
				{
					return m_gameState->IsTouchedPiece(hint.captureChain.front().sourceIndex);
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

void MoveDiscoveryEngine::ResetDiscoveredMoves()
{
	m_touchedPieceAvailableCaptures.clear();
	m_availableCaptures.clear();
	m_availableMoves.clear();
	m_moveHints.clear();
	m_maximalCaptureChain.clear();
}

void MoveDiscoveryEngine::DiscoverMovesForSourceIndex(int32_t sourceIndex)
{
	if (!Utility::IsValidGameBoardIndex(sourceIndex))
	{
		return;
	}
	// | We're given a sourceIndex index. We need its coord for move evaluation
	//   and we need to know what piece exists there.

	// | Initialize our stack with some initial directions to check.
	InitMoveStateStack(Utility::ToGameBoardCoordFromIndex(sourceIndex));

	spdlog::info("Starting search for {} piece at {}", m_gameState->GetTurnPlayerId(), sourceIndex);

	// | Begin discovering moves
	while (!m_moveDiscoveryStack.empty())
	{
		spdlog::info(MoveEvalDescStackToString(m_moveDiscoveryStack));

		// | Load the move context that we'll be evaluating.
		MoveEvaluationContext moveEval = m_moveDiscoveryStack.back();
		spdlog::info("Popping: {}", MoveEvalDescToString(moveEval));
		m_moveDiscoveryStack.pop_back();

		// | Determine whether this evaluation context results in a move, or a capture, or a noop.
		if (DoesDescribeMove(moveEval))
		{
			HandleMoveDiscovered(moveEval);
		}
		// | We've determined this move describes a capture. We now need to discover all the captures this context could lead to.
		else if (DoesDescribeCapture(moveEval))
		{
			// | Make note of the destCoord described in the context. This is where our piece would land after the capture.
			const glm::ivec2 destCoord = Utility::GetCaptureDestCoord(moveEval.sourceCoord, moveEval.direction);
			spdlog::info("landed at {}", Utility::ToGameBoardIndexFromCoord(destCoord));

			// | Since we made a capture, this is now part of the current capture chain we are building.
			moveEval.currentCaptureChain.emplace_back(Utility::ToGameBoardIndexFromCoord(moveEval.sourceCoord),
				Utility::ToGameBoardIndexFromCoord(destCoord));

			spdlog::info("capture_chain {}", moveEval.currentCaptureChain);
			HandleCaptureDiscovered(moveEval);
		}
	}
	// We need to add the sourceIndex move for our capture chain to the list of available captures.
	if (!m_maximalCaptureChain.empty())
	{
		m_moveHints.emplace_back(m_maximalCaptureChain.size(), m_maximalCaptureChain);
		spdlog::info("maximal capture chain={}", m_maximalCaptureChain);
	}

	std::ranges::sort(m_moveHints, std::ranges::greater{}, & PieceMoveHint::score);
}

const std::unordered_set<PieceMoveDescription,
	PieceMoveDescriptionHash>& MoveDiscoveryEngine::GetAvailableCapturesForTouchedPiece() const
{
	return m_touchedPieceAvailableCaptures;
}

const std::unordered_set<PieceMoveDescription,
	PieceMoveDescriptionHash>& MoveDiscoveryEngine::GetAvailableCaptures() const
{
	return m_availableCaptures;
}

const std::unordered_set<PieceMoveDescription,
	PieceMoveDescriptionHash>& MoveDiscoveryEngine::GetAvailableMoves() const
{
	return m_availableMoves;
}

bool MoveDiscoveryEngine::HasMove(const PieceMoveDescription& moveDescription) const
{
	return m_availableMoves.contains(moveDescription);
}

bool MoveDiscoveryEngine::HasCapture(const PieceMoveDescription& moveDescription) const
{
	return m_availableCaptures.contains(moveDescription);
}

bool MoveDiscoveryEngine::HasCaptureForTouchedPiece(const PieceMoveDescription& moveDescription) const
{
	return m_touchedPieceAvailableCaptures.contains(moveDescription);
}

bool MoveDiscoveryEngine::DoesDescribeMove(const MoveEvaluationContext& moveEval) const
{
	const glm::ivec2 destCoord = Utility::GetMoveDestCoord(moveEval.sourceCoord, moveEval.direction);
	if (!Utility::IsValidGameBoardCoord(moveEval.sourceCoord) || !Utility::IsValidGameBoardCoord(destCoord))
	{
		return false;
	}

	const Piece* destPiece = m_gameState->GetPieceFromCoord(destCoord);

#ifdef DEBUG
	// We just checked that the dest is a valid location on the board.
	assert(destPiece);
#endif

	return destPiece->pieceType == PieceType::Empty;
}


bool MoveDiscoveryEngine::DoesDescribeCapture(const MoveEvaluationContext& moveEval) const
{
	const glm::ivec2 destCoord = Utility::GetCaptureDestCoord(moveEval.sourceCoord, moveEval.direction);
	if (!Utility::IsValidGameBoardCoord(moveEval.sourceCoord) || !Utility::IsValidGameBoardCoord(destCoord))
	{
		return false;
	}

	// In order for this to be a valid capture, dest needs to be valid, and the piece in between dest and source
	// must be an enemy.
	const glm::ivec2 middlePieceCoord = Utility::GetMoveDestCoord(moveEval.sourceCoord, moveEval.direction);

	const Piece* middlePiece = m_gameState->GetPieceFromCoord(middlePieceCoord);
	const Piece* destPiece = m_gameState->GetPieceFromCoord(destCoord);

#ifdef DEBUG
	// We just checked that these are both valid locations on the board.
	assert(/*sourcePiece && */destPiece);

	// Middle piece couldn't possibly not be on the board if dest is.
	assert(middlePiece);
#endif

	return Utility::GetPieceAffinity(moveEval.sourceIdentity,
		middlePiece->identity) == Affinity::Enemy &&
		destPiece->pieceType == PieceType::Empty;

}

std::string MoveDiscoveryEngine::MoveEvalDescToString(const MoveEvaluationContext& desc) const
{
	return fmt::format("[{} | {} | captured_pieces{}]"
		, Utility::ToGameBoardIndexFromCoord(desc.sourceCoord)
		, Utility::DirectionToString(desc.direction)
		, desc.capturedPieceIndices);
}

std::string MoveDiscoveryEngine::MoveEvalDescStackToString(const std::deque<MoveEvaluationContext>& desc) const
{
	auto strings = desc | std::ranges::views::transform([this](const MoveEvaluationContext& d)
	{
		return MoveEvalDescToString(d);
	});

	return fmt::format("[{}]", fmt::join(strings, ", "));

}

void MoveDiscoveryEngine::InitMoveStateStack(const glm::ivec2& sourceCoord)
{
	const Piece* sourcePiece = m_gameState->GetPieceFromCoord(sourceCoord);
	const std::span<const glm::ivec2> directions = Utility::GetDirectionsForPiece(*sourcePiece);
	for (auto direction : directions)
	{
		m_moveDiscoveryStack.emplace_back(sourceCoord,
			direction,
			std::vector<int32_t>{},
			std::vector<PieceMoveDescription>{},
			sourcePiece->identity);
	}
}

void MoveDiscoveryEngine::HandleMoveDiscovered(const MoveEvaluationContext& moveEval)
{
	// | Record the move as an option the player has. We already have the source index, we just need the dest index.
	//   which we can calculate by passing the dest coord.
	glm::ivec2 destCoord = Utility::GetMoveDestCoord(moveEval.sourceCoord, moveEval.direction);
	m_availableMoves.insert({ Utility::ToGameBoardIndexFromCoord(moveEval.sourceCoord),
		Utility::ToGameBoardIndexFromCoord(destCoord) });

	spdlog::info("Found Move: {}", Utility::ToGameBoardIndexFromCoord(destCoord));

	// | Make the move visible as a hint.
	PieceMoveHint hint;
	// Basic moves are assigned a score of 0.
	hint.score = 0;
	hint.captureChain.emplace_back(Utility::ToGameBoardIndexFromCoord(moveEval.sourceCoord),
		Utility::ToGameBoardIndexFromCoord(destCoord));

	m_moveHints.push_back(std::move(hint));
}

void MoveDiscoveryEngine::HandleCaptureDiscovered(const MoveEvaluationContext& moveEval)
{
	const glm::ivec2 middlePieceCoord = Utility::GetMoveDestCoord(moveEval.sourceCoord, moveEval.direction);
	spdlog::info("captured at {}", Utility::ToGameBoardIndexFromCoord(middlePieceCoord));

	// | Check all available directions from the lookahead context. If one results in a capture,
	//   add it to the context stack so we can evaluate further.
	const bool isLookaheadCaptureAvailable = CheckLookaheadCapture(moveEval);

	// | If the lookahead turned up nothing, we're at the end of the capture chain.
	if (!isLookaheadCaptureAvailable)
	{
		// | Save off the maximal capture chain
		if (moveEval.currentCaptureChain.size() > m_maximalCaptureChain.size())
		{
			m_maximalCaptureChain = moveEval.currentCaptureChain;
		}

		// | Only the origin move of the capture chain is available for the player to choose.
		// | If a piece has been touched, we need to add this move to the available captures specific to that piece.
		const PieceMoveDescription& originMove = moveEval.currentCaptureChain[0];
		if (m_gameState->IsTouchedPiece(originMove.sourceIndex))
		{
			m_touchedPieceAvailableCaptures.insert({ originMove });
		}

		// | This is the global list of available captures, it needs to be added here too.
		m_availableCaptures.insert({ originMove });
	}
}

bool MoveDiscoveryEngine::CheckLookaheadCapture(const MoveEvaluationContext& moveEval)
{
	// | Now we need to set up an evaluation context that takes place from the current destCoord,
	//   to a capture destination at all of the piece's available directions. This lookahead is
	//   necessary to know if we should continue adding to this capture chain.
	MoveEvaluationContext lookahead = CreateLookaheadCaptureContext(moveEval);

	const Piece* sourcePiece = m_gameState->GetPieceFromCoord(moveEval.sourceCoord);
	const std::span<const glm::ivec2> directions = Utility::GetDirectionsForPiece(*sourcePiece);
	for (auto direction : directions)
	{
		lookahead.direction = direction;

		spdlog::info("lookahead checking {} at {}", Utility::DirectionToString(lookahead.direction),
			Utility::ToGameBoardIndexFromCoord(Utility::GetCaptureDestCoord(lookahead.sourceCoord,
				lookahead.direction)));

		if (DoesDescribeCapture(lookahead))
		{
			glm::ivec2 middlePieceCoord = Utility::GetMoveDestCoord(moveEval.sourceCoord, moveEval.direction);
			middlePieceCoord = Utility::GetMoveDestCoord(lookahead.sourceCoord, lookahead.direction);
			auto it = std::ranges::find(lookahead.capturedPieceIndices,
				Utility::ToGameBoardIndexFromCoord(middlePieceCoord));

			// | If we find a middle piece that we've already captured, then we should ignore the capture.
			bool hasCapturedPieceAlready = it != lookahead.capturedPieceIndices.end();
			spdlog::info("would capture {}| {}| captured_pieces {}", Utility::ToGameBoardIndexFromCoord(middlePieceCoord),
				!hasCapturedPieceAlready ? "Piece not yet captured." : "Piece already captured.", lookahead.capturedPieceIndices);
			if (!hasCapturedPieceAlready)
			{
				// | This is a new capture, add it to the stack for further evaluation
				MoveEvaluationContext newCaptureEval = { lookahead.sourceCoord, lookahead.direction, lookahead.capturedPieceIndices,
					lookahead.currentCaptureChain, sourcePiece->identity };
				spdlog::info("Adding capture {}", MoveEvalDescToString(newCaptureEval));
				m_moveDiscoveryStack.emplace_back(newCaptureEval);

				// | Record that we have not finished searching the capture chain.
				return true;
			}
		}
	}

	return false;
}

MoveDiscoveryEngine::MoveEvaluationContext MoveDiscoveryEngine::CreateLookaheadCaptureContext(
	const MoveEvaluationContext& moveEval)
{
	// This context will be tossed onto the move discovery stack should it yield captures.
	MoveEvaluationContext lookahead;
	lookahead.direction = moveEval.direction;
	lookahead.sourceCoord = Utility::GetCaptureDestCoord(moveEval.sourceCoord, moveEval.direction);
	lookahead.sourceIdentity = moveEval.sourceIdentity;

	// The context object tracks enough board state to make up-to-date decisions on capturing.
	lookahead.capturedPieceIndices = moveEval.capturedPieceIndices;

	// | We need to record the piece between the source and destination that we captured.
	//   - We must store that we captured it in the context so that when we look ahead to see if there
	//      are more moves we can make from the destCoord, we don't try and re-capture the piece we just took.
	const glm::ivec2 middlePieceCoord = Utility::GetMoveDestCoord(moveEval.sourceCoord, moveEval.direction);
	lookahead.capturedPieceIndices.push_back(Utility::ToGameBoardIndexFromCoord(middlePieceCoord));
	lookahead.currentCaptureChain = moveEval.currentCaptureChain;

	return lookahead;
}

//===============================================================

}
