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

private:
	Identity m_identity = Identity::Neutral;

};

//===============================================================

}
