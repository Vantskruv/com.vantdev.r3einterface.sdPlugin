#include "pch.h"
#include "FKeyCommand.h"

std::tuple<unsigned short, bool>* FKeyCommand::keyBeingPressed = nullptr;
/*
const FKeyCommand::KeyCode FKeyCommand::extendedKeys[] = { FKeyCommand::KeyCode::UP, FKeyCommand::KeyCode::DOWN, FKeyCommand::KeyCode::LEFT, FKeyCommand::KeyCode::RIGHT,
                                                           FKeyCommand::KeyCode::INSERT, FKeyCommand::KeyCode::HOME, FKeyCommand::KeyCode::PAGE_UP, FKeyCommand::KeyCode::PAGEDOWN,
                                                           FKeyCommand::KeyCode::DEL, FKeyCommand::KeyCode::END
};

*/