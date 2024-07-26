//---------------------------------------------------------------
//
// ICommand.h
//

#pragma once

#include "GameTypes.h"

#include <string>

namespace Checkers {

//===============================================================

class Game;

class ICommand {
public:
	ICommand(const ICommand& other) = delete;
	ICommand& operator=(const ICommand& other) = delete;
	ICommand(ICommand&& other) noexcept = default;
	ICommand& operator=(ICommand&& other) noexcept = default;

	ICommand() = default;
	virtual ~ICommand();

	// Each command knows how to execute itself.
	virtual void Execute(Game* game) = 0;

	// Canceled commands should not execute.
	virtual void Cancel() = 0;

	// Check this before executing. Executing a canceled command can cause bad things to happen.
	virtual bool IsCanceled() = 0;
};

// TODO: Maybe move these impls somewhere else.
//---------------------------------------------------------------

class HelpCommand : public ICommand {
public:

	HelpCommand(const HelpCommand& other) = delete;
	HelpCommand& operator=(const HelpCommand& other) = delete;
	HelpCommand(HelpCommand&& other) noexcept = default;
	HelpCommand& operator=(HelpCommand&& other) noexcept = default;

	HelpCommand() = default;
	virtual ~HelpCommand() override;

	// ICommandImpl
	void Execute(Game* game) override;

	// HelpCommand shouldn't have any reason to cancel.
	void Cancel() override {/*NYI*/};
	bool IsCanceled() override { return false; }
};

//---------------------------------------------------------------

class ChangeViewStyleCommand : public ICommand {
public:

	struct Settings
	{
		static constexpr int32_t s_viewStyleArgPosition = 0;
	};

	ChangeViewStyleCommand(const ChangeViewStyleCommand& other) = delete;
	ChangeViewStyleCommand& operator=(const ChangeViewStyleCommand& other) = delete;
	ChangeViewStyleCommand(ChangeViewStyleCommand&& other) noexcept = default;
	ChangeViewStyleCommand& operator=(ChangeViewStyleCommand&& other) noexcept = default;

	ChangeViewStyleCommand(const std::string& styleOption);
	virtual ~ChangeViewStyleCommand() override;

	// ICommandImpl
	void Execute(Game* game) override;
	void Cancel() override;
	bool IsCanceled() override;

private:
	// Will be set upon execution.
	GameBoardViewStrategyId m_newViewStrategy = GameBoardViewStrategyId::End;

	// Various errors can cause us to cancel this command.
	bool m_isCanceled;
};

//---------------------------------------------------------------


class MoveCommand : public ICommand {
public:

	struct Settings
	{
		static constexpr int32_t s_sourceArgPosition = 0;
		static constexpr int32_t s_destinationArgPosition = 1;
	};

	MoveCommand(const MoveCommand& other) = delete;
	MoveCommand& operator=(const MoveCommand& other) = delete;
	MoveCommand(MoveCommand&& other) noexcept = default;
	MoveCommand& operator=(MoveCommand&& other) noexcept = default;

	MoveCommand(const std::string& sourceInput, const std::string& destinationInput);
	virtual ~MoveCommand() override;

	// ICommandImpl
	void Execute(Game* game) override;
	void Cancel() override;
	bool IsCanceled() override { return m_isCanceled; }

private:
	// Will be translated to the index of the piece the player wants to move.
	std::string m_sourceInput;

	// Will be translated to the index of the empty place the player wants to move the source piece.
	std::string m_destinationInput;

	// Various errors can cause us to cancel this command.
	bool m_isCanceled = false;
};

//===============================================================

}
