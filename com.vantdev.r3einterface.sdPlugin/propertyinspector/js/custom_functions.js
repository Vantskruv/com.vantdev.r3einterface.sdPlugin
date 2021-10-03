// Set default values of missing keys for the global settings
function addDefaultGlobalSettings(global_settings) {
    if (!global_settings.hasOwnProperty("r3e_executable")) {
        global_settings["r3e_executable"] = "RRRE64.exe";
    }


    if (!global_settings.hasOwnProperty("pit_menu_up_key")) {
        global_settings["pit_menu_up_key"] = 0x57;  // W
    }
    if (!global_settings.hasOwnProperty("pit_menu_down_key")) {
        global_settings["pit_menu_down_key"] = 0x53; // S
    }
    if (!global_settings.hasOwnProperty("pit_menu_left_key")) {
        global_settings["pit_menu_left_key"] = 0x41; // A
    }
    if (!global_settings.hasOwnProperty("pit_menu_right_key")) {
        global_settings["pit_menu_right_key"] = 0x44; // D
    }
    if (!global_settings.hasOwnProperty("pit_menu_enter_key")) {
        global_settings["pit_menu_enter_key"] = 0x45; // E
    }
    if (!global_settings.hasOwnProperty("pit_request_box_key")) {
        global_settings["pit_request_box_key"] = 0x52; // R
    }
    if (!global_settings.hasOwnProperty("pit_toggle_menu_key")) {
        global_settings["pit_toggle_menu_key"] = 0x51; // Q
    }

   
    if (!global_settings.hasOwnProperty("millisec_between_each_cmd")) {
        global_settings["millisec_between_each_cmd"] = 200;
    }
    if (!global_settings.hasOwnProperty("millisec_between_each_fuel_cmd")) {
        global_settings["millisec_between_each_fuel_cmd"] = 20;
    }
    if (!global_settings.hasOwnProperty("millisec_key_holdtime")) {
        global_settings["millisec_key_holdtime"] = 10;
    }

    return global_settings;
}

// Set default values of missing keys for this action-item.
function addDefaultSettings(action, settings) {
    if (action == "com.vantdev.r3sd.toggleboxoptionbutton")
    {
        if (!settings.hasOwnProperty("box_option")) {
            settings.box_option = BOX_OPTIONS.DO_NOTHING;
        }
    }
    else if (action == "com.vantdev.r3sd.requestboxbutton")
    {
        if (!settings.hasOwnProperty("serve_penalty")) settings.serve_penalty = false;
        if (!settings.hasOwnProperty("driver_change")) settings.driver_change = false;
        if (!settings.hasOwnProperty("front_tires")) settings.front_tires = false;
        if (!settings.hasOwnProperty("rear_tires")) settings.rear_tires = false;
        if (!settings.hasOwnProperty("fix_bodywork")) settings.fix_bodywork = false;
        if (!settings.hasOwnProperty("fix_front_aero")) settings.fix_front_aero = false;
        if (!settings.hasOwnProperty("fix_rear_aero")) settings.fix_rear_aero = false;
        if (!settings.hasOwnProperty("fix_suspension")) settings.fix_suspension = false;
        if (!settings.hasOwnProperty("refuel_option")) settings.refuel_option = BOX_OPTIONS.DO_NOTHING;
        if (!settings.hasOwnProperty("use_toggle_buttons_only")) settings.use_toggle_buttons_only = false;
        if (!settings.hasOwnProperty("request_box")) settings.request_box = false;
        if (!settings.hasOwnProperty("close_pit_menu")) settings.close_pit_menu = false;
    }

    return settings;
}

function openSettingsWindowsButtonPress() {
    if (!window.uisettingswindow || window.uisettingswindow.closed) {
        window.uisettingswindow = window.open('settings_window.html', 'Global Settings');
    }
}

function openHelpWindowButtonPress() {
    window.open("help.html", "Raceroom Interface help");
}

// Called by window.uisettingswindow when user presses save settings in the window.
function updateGlobalSettings(savedGlobalSettings) {
    // Add default keys if keys are missing (hence invalid data which is ignored in the settings_window)
    global_settings = addDefaultGlobalSettings(savedGlobalSettings);

    // Save the global settings, plugin will be notified by this in its didRecieveGlobalSettings function.
    // (note, the equal property inspector function will not recieve this notification).
    $SD.api.setGlobalSettings($SD.uuid, global_settings);
}


// Called by window.uisettingswindow when initilized
function getGlobalSettings() {
    return global_settings;
}