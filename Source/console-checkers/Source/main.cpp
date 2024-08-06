#include "ConsoleGameDisplay.h"
#include "Game.h"
#include "GameState.h"

#include <spdlog/spdlog-inl.h>
#include <spdlog/sinks/msvc_sink.h>

void SetupLogger() {

	// The console should be used for our game UI, so send logs to the VS Console Window.
	// We could write to file but we just want to use logs for debugging, the game won't ship with logging.
	auto vs_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
	auto logger = std::make_shared<spdlog::logger>("checkers_logger", vs_sink);
	spdlog::set_default_logger(logger);
	spdlog::set_level(spdlog::level::debug);
}

void EnableANSI() {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE)
		return;
	// https://learn.microsoft.com/en-us/windows/console/setconsolemode
	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode))
		return;

	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
}

int main()
{
	EnableANSI();
	SetupLogger();

	// TODO: Perhaps this stuff goes into an Application class or something.
	Checkers::Game game;
	Checkers::ConsoleGameDisplay display;

	// Init the display.
	display.Initialize(game.GetGameState(), game.GetUIEvents());

	// Run until we're done.
	game.Run();

	return 0;
}
