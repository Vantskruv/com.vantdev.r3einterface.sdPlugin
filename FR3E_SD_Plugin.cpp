#include "FR3E_SD_Plugin.h"

#include "Common/ESDConnectionManager.h"
#include "Common/EPLJSONUtils.h"

#include "r3esd_src/FKeyCommand.h"
#include "r3esd_src/FR3EData.h"

#include <atomic>
#include <thread>
#include <vector>

//static const std::string blackImage = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=";
//static const std::string redImage = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8DwHwAFBQIAX8jx0gAAAABJRU5ErkJggg==";

FR3E_SD_Plugin::FR3E_SD_Plugin()
{
	try
	{
		r3e_data = new FR3EData();
	}
	catch (...)
	{
		r3e_data = nullptr;
	}
}

FR3E_SD_Plugin::~FR3E_SD_Plugin()
{
	bRunBlink = false;
	if(tBlinkContext.joinable()) tBlinkContext.join();
	
	if(r3e_data) delete r3e_data;
}

void FR3E_SD_Plugin::thread_blinkContext()
{
	bool bBlink = true;

	if(r3e_data) while (r3e_data->isPitOptionsRunning() && bRunBlink)
	{
		mCurrentInContext.lock();
		mConnectionManager->SetState(bBlink, currentInContext);
		mCurrentInContext.unlock();
		bBlink = !bBlink;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	mCurrentInContext.lock();
	mConnectionManager->SetState(0, currentInContext);
	currentInContext = std::string("");
	mCurrentInContext.unlock();
}

void FR3E_SD_Plugin::KeyDownForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	// Nothing to do
}

void FR3E_SD_Plugin::KeyUpForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	if (inAction.compare("com.vantdev.r3sd.toggleboxoptionbutton") == 0) // If a toggleboxoptionbutton has been released.
	{
		// Retrieve what box-option the button has (actually, this should be the same value that is bound for this context in the std::map mVisibileContexts)
		json settings;
		EPLJSONUtils::GetObjectByName(inPayload, "settings", settings);
		int boxOption = EPLJSONUtils::GetIntByName(settings, "box_option", 0);
		mConnectionManager->LogMessage("KeyUpForAction: " + std::to_string(boxOption));
		
		mVisibleToggleButtonsMutex.lock();
		// Toggle the option
		toggleOption(boxOption);
		// Set the state of all toggle-buttons which has this box-option.
		// Though, the refuel toggle-boxes that does not has the same refueling option should be set to state 0 (refueling toggle buttons working as a group of radiobuttons).
		for (auto iContext : mVisibleToggleButtons)
		{
			if (boxOption == REFUEL_SAFE || boxOption == REFUEL_NORMAL || boxOption == REFUEL_RISKY || boxOption == REFUEL_OPTIMUM)
			{
				if (iContext.second == REFUEL_SAFE || iContext.second == REFUEL_NORMAL || iContext.second == REFUEL_RISKY || iContext.second == REFUEL_OPTIMUM)
				{
					mConnectionManager->SetState(refuel_option == iContext.second, iContext.first);
				}
			}
			else if (iContext.second == boxOption)
			{
				mConnectionManager->SetState(isBoxOptionActive(boxOption), iContext.first);
			}
		}
		mVisibleToggleButtonsMutex.unlock();
	}
	else if (inAction.compare("com.vantdev.r3sd.requestboxbutton") == 0) // If a requestboxbutton has been released.
	{
		if (!r3e_data) return;
		// Retrieve the box-options set in requestboxbutton
		json settings;
		EPLJSONUtils::GetObjectByName(inPayload, "settings", settings);

		bool bUseToggleButtons = EPLJSONUtils::GetBoolByName(settings, "use_toggle_buttons_only", false);
		bool bRequestBoxThisLap = EPLJSONUtils::GetBoolByName(settings, "request_box", false);
		bool bClosePitMenu = EPLJSONUtils::GetBoolByName(settings, "close_pit_menu", false);

		std::vector<std::pair<int, bool>> pitOptions;
		int iRefuelOption;

		// If setting "use_toggle_buttons" is true, we ignore all the box-settings in this button, and use the box-options set from the toggle-buttons instead.
		if (bUseToggleButtons)
		{
			//In r3etoggle.js, the REFUEL_SAFE option is 7, the REFUEL_NORMAL option is 8 and so on.
			//setPitOptions wants to have 0, 1, 2 and 3 for the refueling options, so we reduce the refuel_option, which is set from r3etoggle.js, with 7.
			//This can be ugly if the indexes of refuel options in r3etoggle.js changes ...
			//Note that if we would change these indexes in r3etoggle.js, we need to change them aswell in the header file of this class.
			//The same goes for r3esd_reqbox.js!!!

			pitOptions = {
				{R3E_PIT_MENU_PENALTY, serve_penalty},
				{R3E_PIT_MENU_DRIVERCHANGE, driver_change},
				{R3E_PIT_MENU_BODYWORK, bodywork},
				{R3E_PIT_MENU_FRONTAERO, front_wing},
				{R3E_PIT_MENU_REARAERO, rear_wing},
				{R3E_PIT_MENU_SUSPENSION, suspension},
				{R3E_PIT_MENU_FRONTTIRES, front_tires},
				{R3E_PIT_MENU_REARTIRES, rear_tires},
				{R3E_PIT_MENU_FUEL, refuel_option != NO_REFUEL} };

			iRefuelOption = refuel_option - REFUEL_SAFE;
		}
		else
		{
			mCurrentInContext.lock();
			if (currentInContext.compare(inContext) == 0)
			{
				mConnectionManager->SetState(0, inContext);
				mCurrentInContext.unlock();
				r3e_data->setPitOptions(pitOptions, 0, false, true, true);
				return;
			}
			mCurrentInContext.unlock();
			iRefuelOption = EPLJSONUtils::GetIntByName(settings, "refuel_option", REFUEL_SAFE) - REFUEL_SAFE;
			if (iRefuelOption < 0) iRefuelOption = NO_REFUEL;

			pitOptions = {
				{R3E_PIT_MENU_PENALTY, EPLJSONUtils::GetBoolByName(settings, "serve_penalty", false)},
				{R3E_PIT_MENU_DRIVERCHANGE, EPLJSONUtils::GetBoolByName(settings, "driver_change", false)},
				{R3E_PIT_MENU_BODYWORK, EPLJSONUtils::GetBoolByName(settings, "fix_bodywork", false)},
				{R3E_PIT_MENU_FRONTAERO, EPLJSONUtils::GetBoolByName(settings, "fix_front_aero", false)},
				{R3E_PIT_MENU_REARAERO, EPLJSONUtils::GetBoolByName(settings, "fix_rear_aero", false)},
				{R3E_PIT_MENU_SUSPENSION, EPLJSONUtils::GetBoolByName(settings, "fix_suspension", false)},
				{R3E_PIT_MENU_FRONTTIRES, EPLJSONUtils::GetBoolByName(settings, "front_tires", false)},
				{R3E_PIT_MENU_REARTIRES, EPLJSONUtils::GetBoolByName(settings, "rear_tires", false)},
				{R3E_PIT_MENU_FUEL, iRefuelOption} };
		}

		mConnectionManager->SetState(0, inContext);

		if (r3e_data->setPitOptions(pitOptions, iRefuelOption, bRequestBoxThisLap, bClosePitMenu, true))
		{
			bRunBlink = false;
			if (tBlinkContext.joinable()) tBlinkContext.join();
			bRunBlink = true;
			mCurrentInContext.lock();
			currentInContext = std::string(inContext);
			tBlinkContext = std::thread(&FR3E_SD_Plugin::thread_blinkContext, this);
			mCurrentInContext.unlock();
		}
	}
}

bool FR3E_SD_Plugin::isBoxOptionActive(int _boxOption)
{
	if (_boxOption == SERVE_PENALTY) return serve_penalty;
	if (_boxOption == DRIVER_CHANGE) return driver_change;
	if (_boxOption == FRONT_TIRES) return front_tires;
	if (_boxOption == REAR_TIRES) return rear_tires;
	if (_boxOption == BODYWORK) return bodywork;
	if (_boxOption == FRONT_WING) return front_wing;
	if (_boxOption == REAR_WING) return rear_wing;
	if (_boxOption == SUSPENSION) return suspension;

	return _boxOption == refuel_option;
}

void FR3E_SD_Plugin::toggleOption(int _boxOption)
{
	if (_boxOption == SERVE_PENALTY) serve_penalty = !serve_penalty;
	else if (_boxOption == DRIVER_CHANGE) driver_change = !driver_change;
	else if (_boxOption == FRONT_TIRES) front_tires = !front_tires;
	else if (_boxOption == REAR_TIRES) rear_tires = !rear_tires;
	else if (_boxOption == BODYWORK) bodywork = !bodywork;
	else if (_boxOption == FRONT_WING) front_wing = !front_wing;
	else if (_boxOption == REAR_WING) rear_wing = !rear_wing;
	else if (_boxOption == SUSPENSION) suspension = !suspension;
	else if (_boxOption == refuel_option) refuel_option = NO_REFUEL;	//If button refuel_option is the same as the option to toggle, with set refuel_option to NO_REFUEL, otherwise we set one of the refuel options as active.
	else if (_boxOption == REFUEL_SAFE) refuel_option = REFUEL_SAFE;
	else if (_boxOption == REFUEL_NORMAL) refuel_option = REFUEL_NORMAL;
	else if (_boxOption == REFUEL_RISKY) refuel_option = REFUEL_RISKY;
	else if (_boxOption == REFUEL_OPTIMUM) refuel_option = REFUEL_OPTIMUM;
}

void FR3E_SD_Plugin::WillAppearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	mConnectionManager->LogMessage("WillAppearForAction: " + inAction);

	// It is only necessary to add toggle box option buttons
	if (inAction.compare("com.vantdev.r3sd.toggleboxoptionbutton") == 0)
	{
		json settings;
		EPLJSONUtils::GetObjectByName(inPayload, "settings", settings);

		// We set the state of the button
		int boxOption = EPLJSONUtils::GetIntByName(settings, "box_option", 0);
		mVisibleToggleButtonsMutex.lock();
		if (isBoxOptionActive(boxOption)) mConnectionManager->SetState(1, inContext);
		else mConnectionManager->SetState(0, inContext);

		// Remember the context
		mVisibleToggleButtons.insert(std::make_pair(inContext, boxOption));
		mVisibleToggleButtonsMutex.unlock();
	}
	else if (inAction.compare("com.vantdev.r3sd.requestboxbutton") == 0)
	{
	}
}

void FR3E_SD_Plugin::WillDisappearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	// Check if it is a toggleoptionbutton, otherwise we exit the function.
	mConnectionManager->LogMessage("WillDisappearForAction: " + inAction);
	if (inAction.compare("com.vantdev.r3sd.toggleboxoptionbutton") == 0)
	{
		// Find the box-option value of the context we want to remove
		mVisibleToggleButtonsMutex.lock();
		auto foundContext = mVisibleToggleButtons.find(inContext);
		if (foundContext == mVisibleToggleButtons.end())
		{
			mVisibleToggleButtonsMutex.unlock();
			return;
		}
		int cBoxOption = foundContext->second;

		// Remove the context
		
		mVisibleToggleButtons.erase(inContext);

		// Search for other buttons with the same option this button currently has.
		bool bIsFound = false;
		for (auto iContext : mVisibleToggleButtons)
		{
			if (iContext.second == cBoxOption)
			{
				bIsFound = true;
				break;
			}
		}

		// If no button which has this box-option is found, and the option is set to true, set it to false.
		if (!bIsFound)
		{
			if (isBoxOptionActive(cBoxOption)) toggleOption(cBoxOption);
		}
		mVisibleToggleButtonsMutex.unlock();
	}
	else if (inAction.compare("com.vantdev.r3sd.requestboxbutton") == 0)
	{
		/*
		if (inContext.compare(sBlinkContext) == 0)
		{
			bIsSetPitOptionsRunning = false;
		}
		*/
	}
}

void FR3E_SD_Plugin::DeviceDidConnect(const std::string& inDeviceID, const json &inDeviceInfo)
{
	mConnectionManager->LogMessage("DeviceDidConnect recieved data");
	mConnectionManager->GetGlobalSettings();
	// Nothing to do
}

void FR3E_SD_Plugin::DeviceDidDisconnect(const std::string& inDeviceID)
{
	mConnectionManager->LogMessage("DeviceDidDisconnect recieved data");
	// Nothing to do
}

void FR3E_SD_Plugin::SendToPlugin(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID)
{
	mConnectionManager->LogMessage("SendToPlugin recieved data");
}

void FR3E_SD_Plugin::DidReceiveSettings(const std::string& inAction, const std::string& inContext, const json& inPayload,	const std::string& inDeviceID)
{
	mConnectionManager->LogMessage("DidReceiveSettings recieved data");

	if (inAction.compare("com.vantdev.r3sd.toggleboxoptionbutton") == 0)
	{
		json settings;
		if (!EPLJSONUtils::GetObjectByName(inPayload, "settings", settings)) return;


		// Search for our context
		mVisibleToggleButtonsMutex.lock();
		auto foundContext = mVisibleToggleButtons.find(inContext);
		if (foundContext == mVisibleToggleButtons.end())
		{
			mVisibleToggleButtonsMutex.unlock();
			return;
		}

		int cBoxOption = foundContext->second;

		// Search for other buttons with the same option this button currently has.
		bool bIsFound = false;
		for (auto iContext : mVisibleToggleButtons)
		{
			if (iContext.second == cBoxOption && iContext.first.compare(inContext) != 0)
			{
				bIsFound = true;
				break;
			}
		}

		// If no button is found, and the option is set to true, set it to false.
		if (!bIsFound)
		{
			if (isBoxOptionActive(cBoxOption)) toggleOption(cBoxOption);
		}

		int boxOption = EPLJSONUtils::GetIntByName(settings, "box_option", -1);
		if (boxOption < 0)
		{
			//foundContext->second = DO_NOTHING;
			//mConnectionManager->SetState(0, inContext);
			mVisibleToggleButtonsMutex.unlock();
			return;
		}

		// Assign the button for the new option
		foundContext->second = boxOption;

		// Set its state
		if (isBoxOptionActive(boxOption)) mConnectionManager->SetState(1, inContext);
		else mConnectionManager->SetState(0, inContext);
		mVisibleToggleButtonsMutex.unlock();
	}
}

void FR3E_SD_Plugin::DidReceiveGlobalSettings(const json& inPayload)
{
	mConnectionManager->LogMessage("DidReceiveGlobalSettings recieved data");
	if (!r3e_data) return;

	json settings;
	if (!EPLJSONUtils::GetObjectByName(inPayload, "settings", settings)) return;
	
	std::string sExec = EPLJSONUtils::GetStringByName(settings, "r3e_executable", "RRRE64.exe");
	int pitMenuUp = EPLJSONUtils::GetIntByName(settings, "pit_menu_up_key", 0x57);
	int pitMenuDown = EPLJSONUtils::GetIntByName(settings, "pit_menu_down_key", 0x53);
	int pitMenuLeft = EPLJSONUtils::GetIntByName(settings, "pit_menu_left_key", 0x41);
	int pitMenuRight = EPLJSONUtils::GetIntByName(settings, "pit_menu_right_key", 0x44);
	int pitMenuEnter = EPLJSONUtils::GetIntByName(settings, "pit_menu_enter_key", 0x45);
	int pitMenuBox = EPLJSONUtils::GetIntByName(settings, "pit_request_box_key", 0x52);
	int pitMenuToggle = EPLJSONUtils::GetIntByName(settings, "pit_toggle_menu_key", 0x51);
	int waitCmd = EPLJSONUtils::GetIntByName(settings, "millisec_between_each_cmd", 200);
	int waitCmdFuel = EPLJSONUtils::GetIntByName(settings, "millisec_between_each_fuel_cmd", 20);
	int holdTime = EPLJSONUtils::GetIntByName(settings, "millisec_key_holdtime", 10);

	r3e_data->setExecutable(sExec);
	r3e_data->setKeyCodes(pitMenuUp, pitMenuDown, pitMenuLeft, pitMenuRight, pitMenuEnter, pitMenuBox, pitMenuToggle);
	r3e_data->setTimings(holdTime, waitCmd, waitCmdFuel);
}
