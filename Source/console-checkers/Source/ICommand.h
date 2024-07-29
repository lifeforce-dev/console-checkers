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

struct CommandErrorInfo {
	// Player-facing reason as to why this command failed to process.
	std::string errorReason;
};

class ICommand {
public:
	ICommand(const ICommand& other) = delete;
	ICommand& operator=(const ICommand& other) = delete;
	ICommand(ICommand&& other) noexcept = default;
	ICommand& operator=(ICommand&& other) noexcept = default;

	ICommand() = default;
	virtual ~ICommand();

	// Each command knows how to execute itself.
	virtual bool Execute(Game* game) = 0;

	// Canceled commands should not execute.
	virtual void Cancel() = 0;

	// Check this before executing. Executing a canceled command can cause bad things to happen.
	virtual bool IsCanceled() const = 0;

	// If an error happens inside the command, we need to inform the player.
	// We don't have access to events and such, so just fill this object out.
	virtual const CommandErrorInfo& GetErrorInfo() = 0;
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
	bool Execute(Game* game) override;

	// HelpCommand shouldn't have any reason to cancel.
	void Cancel() override {/*NYI*/};
	bool IsCanceled() const override { return false; }

	const CommandErrorInfo& GetErrorInfo() override;
};

//---------------------------------------------------------------

class ChangeViewStyleCommand : public ICommand {
public:

	struct Settings
	{
		static constexpr int32_t s_viewStyleArgPosition = 0;
		static constexpr int32_t s_requiredArgumentCount = 1;
	};

	ChangeViewStyleCommand(const ChangeViewStyleCommand& other) = delete;
	ChangeViewStyleCommand& operator=(const ChangeViewStyleCommand& other) = delete;
	ChangeViewStyleCommand(ChangeViewStyleCommand&& other) noexcept = default;
	ChangeViewStyleCommand& operator=(ChangeViewStyleCommand&& other) noexcept = default;

	ChangeViewStyleCommand(const std::string& styleOption);
	virtual ~ChangeViewStyleCommand() override;

	// ICommandImpl
	bool Execute(Game* game) override;
	void Cancel() override;
	bool IsCanceled() const override;
	const CommandErrorInfo& GetErrorInfo() override;

private:

	// Option the player chose.
	int32_t m_optionValue = 0;

	// Various errors can cause us to cancel this command.
	bool m_isCanceled = false;

	// As much context as we can have around any errors that happen.
	CommandErrorInfo m_errorInfo;
};

//---------------------------------------------------------------


class MoveCommand : public ICommand {
public:

	struct Settings
	{
		static constexpr int32_t s_sourceArgPosition = 0;
		static constexpr int32_t s_destinationArgPosition = 1;
		static constexpr int32_t s_requiredArgumentCount = 2;
	};

	MoveCommand(const MoveCommand& other) = delete;
	MoveCommand& operator=(const MoveCommand& other) = delete;
	MoveCommand(MoveCommand&& other) noexcept = default;
	MoveCommand& operator=(MoveCommand&& other) noexcept = default;

	MoveCommand(const std::string& sourceInput, const std::string& destinationInput);
	virtual ~MoveCommand() override;

	// ICommandImpl
	bool Execute(Game* game) override;
	void Cancel() override;
	bool IsCanceled() const override { return m_isCanceled; }
	const CommandErrorInfo& GetErrorInfo() override;

private:
	// Will be translated to the index of the piece the player wants to move.
	std::string m_sourceInput;

	// Will be translated to the index of the empty place the player wants to move the source piece.
	std::string m_destinationInput;

	// Various errors can cause us to cancel this command.
	bool m_isCanceled = false;

	// As much context as we can have around any errors that happen.
	CommandErrorInfo m_errorInfo;
};

//===============================================================

}
