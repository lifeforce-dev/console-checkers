//---------------------------------------------------------------
//
// ConsoleGameDisplay.cpp
//

#include "ConsoleGameDisplay.h"

#include "IGameBoardViewStrategy.h"
#include "IGameStateDisplayInfo.h"
#include "UIEvents.h"
#include "UITextStrings.h"

#include <iostream>
#include <ranges>

#include <spdlog/fmt/bundled/core.h>
#include <spdlog/spdlog.h>

namespace Checkers {

//===============================================================

void ConsoleGameDisplay::Initialize(IGameStateDisplayInfo* displayData, UIEvents& events)
{
	m_gameStateInfo = displayData;

	events.GetWelcomePromptRequestedEvent().subscribe(
		[this]()
	{
		std::string uiScreenText = fmt::format("{}\n\n\n", UIText::s_welcomeMessage);

		uiScreenText.append(GetGameBoardDisplay());

		std::cout << uiScreenText;
	});

	events.GetGameBoardViewStrategyChangedEvent().subscribe(
		[this](IGameBoardViewStrategy* view)
	{
		IGameBoardViewStrategy* old = m_gameBoardView;
		m_gameBoardView = view;

		// We assume that if we didn't have a view before, this call was initialization
		// and not a change.
		if (old)
		{
			// We only want to reprint the board if our view actually changed
			std::cout << GetGameBoardDisplay();
		}
	});

	events.GetHelpPromptRequestedEvent().subscribe([this]()
	{
		std::cout << fmt::format(UIText::s_helpCommandPromptMessage, m_gameBoardView->GetMoveCommandSyntax());
	});

	m_isInitialized = true;
}

std::string ConsoleGameDisplay::GetGameBoardDisplay() const
{
#ifdef DEBUG
	// We should not be calling any functions on ConsoleGameDisplay unless we're initialized.
	assert(m_isInitialized);

	// Hi! If you're here, you forgot to set a view, you should go do that.
	assert(m_gameBoardView);
#endif

	return m_gameBoardView->GetGameBoardDisplayText(m_gameStateInfo->GetGameBoardData());
}

//===============================================================

}
