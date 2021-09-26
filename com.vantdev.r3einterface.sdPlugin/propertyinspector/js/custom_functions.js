function addDefaultGlobalSettings(settings) {
    if (!settings.hasOwnProperty("r3e_executable")) {
        settings["r3e_executable"] = "RRR64.exe";
    }
    if (!settings.hasOwnProperty("millisec_between_each_cmd")) {
        settings["millisec_between_each_cmd"] = "200";
    }
    return settings;
}

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
    }

    return settings;
}