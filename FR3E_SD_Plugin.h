//==============================================================================
/**
@file       MyStreamDeckPlugin.h

@brief      CPU plugin

@copyright  (c) 2018, Corsair Memory, Inc.
			This source code is licensed under the MIT-style license found in the LICENSE file.

**/
//==============================================================================

#include "Common/ESDBasePlugin.h"
#include <set>
#include <map>
#include <mutex>
#include <thread>
#include <vector>


class FR3E_SD_Plugin : public ESDBasePlugin
{
public:
	
	FR3E_SD_Plugin();
	virtual ~FR3E_SD_Plugin();
	
	void KeyDownForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	void KeyUpForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	
	void WillAppearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	void WillDisappearForAction(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	
	void DeviceDidConnect(const std::string& inDeviceID, const json &inDeviceInfo) override;
	void DeviceDidDisconnect(const std::string& inDeviceID) override;
	
	void SendToPlugin(const std::string& inAction, const std::string& inContext, const json &inPayload, const std::string& inDeviceID) override;
	void DidReceiveSettings(const std::string& inAction, const std::string& inContext, const json& inPayload, const std::string& inDeviceID) override;

private:
	const static int DO_NOTHING = 0;
	const static int SERVE_PENALTY = 1;
	const static int DRIVER_CHANGE = 2;
	const static int FRONT_TIRES = 3;
	const static int REAR_TIRES = 4;
	const static int BODYWORK = 5;
	const static int FRONT_WING = 6;
	const static int REAR_WING = 7;
	const static int SUSPENSION = 8;
	const static int REFUEL_SAFE = 9;
	const static int REFUEL_NORMAL = 10;
	const static int REFUEL_RISKY = 11;
	const static int REFUEL_OPTIMUM = 12;
	const static int NO_REFUEL = 13;
	
	// Interface to poll data from raceroom
	class FR3EData* r3e_data;
	std::thread tBlinkContext;
	bool bRunBlink = false;
	std::string currentInContext="";
	std::mutex mCurrentInContext;
	void thread_blinkContext();

	// Check if a box-option is active, set by toggle buttons.
	bool isBoxOptionActive(int boxOption);

	// Toggle a boxoption. If the box option is REFUEL_??? option, set that option to true if it is not active, otherwise set it to false.
	void toggleOption(int _boxOption);
	
	// Mutex when adding, deleting or reading/changing any contexts.
	std::mutex mVisibleToggleButtonsMutex;
	//std::mutex mVisibleRequestBoxButtonsMutex;
	
	// Toggle buttons bound to an boxoption.
	std::map<std::string, int> mVisibleToggleButtons;
	//std::set<std::string> mVisibleRequestBoxButtons;
	
	//Settings set by toggle-switches
	bool serve_penalty = false;
	bool driver_change = false;
	bool front_tires = false;
	bool rear_tires = false;
	bool bodywork = false;
	bool front_wing = false;
	bool rear_wing = false;
	bool suspension = false;
	int refuel_option = NO_REFUEL;
};
