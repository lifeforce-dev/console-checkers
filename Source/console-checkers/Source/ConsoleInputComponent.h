//---------------------------------------------------------------
//
// ConsoleInputComponent.h
//

#pragma once

namespace Checkers {

//===============================================================

class ConsoleInputComponent
{
public:
	ConsoleInputComponent(const ConsoleInputComponent& other) = delete;
	ConsoleInputComponent& operator=(const ConsoleInputComponent& other) = delete;

	ConsoleInputComponent(ConsoleInputComponent&& other) {}
	ConsoleInputComponent& operator=(ConsoleInputComponent&& other)
	{
		if (this == &other)
			return *this;
		return *this;
	}

	ConsoleInputComponent() = default;
	~ConsoleInputComponent() = default;
};

//===============================================================

}
