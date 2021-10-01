//let global_settings = {};
let elKeyDetect = null;

function initializeSettingsWindow() {

    // Retrieve a cooy of global_settings
    //var global_settings = opener.global_settings;
    var global_settings = opener.getGlobalSettings();

    // Elements are assigned stored data from global_settings in the opener-document (calling window).

     var e = document.getElementById("r3e_executable");
     e.value = global_settings["r3e_executable"];

    var elements = document.getElementsByName("pit_menu_key_button");
    for (var i = 0; i < elements.length; i++) {
        elements[i].r3e_keycode = global_settings[elements[i].id];
        if (keyCodeList.has(global_settings[elements[i].id])) {
            elements[i].innerHTML = keyCodeList.get(global_settings[elements[i].id]);
        }
        else {
            elements[i].innerHTML = "[" + global_settings[elements[i].id] + "]";
        }
    }

    var e = document.getElementById("millisec_between_each_cmd");
    e.value = global_settings["millisec_between_each_cmd"];
    e = document.getElementById("millisec_between_each_fuel_cmd");
    e.value = global_settings["millisec_between_each_fuel_cmd"];
    e = document.getElementById("millisec_key_holdtime");
    e.value = global_settings["millisec_key_holdtime"];

    document.addEventListener('keydown', keyDownEventListener);
}

// Save the the settings from the elements in to the global_settings in the calling window.
// Invalid values will be set to default in the updateGlobalSettings function in the calling window.
function updateSettings() {
    var global_settings = {};

    var e = document.getElementById("r3e_executable");
    if (e.value.length) global_settings["r3e_executable"] = e.value;

    var elements = document.getElementsByName("pit_menu_key_button")
    for (var i = 0; i < elements.length; i++) {
        if (elements[i].r3e_keycode) global_settings[elements[i].id] = elements[i].r3e_keycode;
    }

    e = document.getElementById("millisec_between_each_cmd");
    var iValue = parseInt(e.value);
    if (iValue > 0 && iValue <= 1000)  global_settings["millisec_between_each_cmd"] = iValue;
    e = document.getElementById("millisec_between_each_fuel_cmd");
    iValue = parseInt(e.value);
    if (iValue > 0 && iValue <= 1000)  global_settings["millisec_between_each_fuel_cmd"] = iValue;
    e = document.getElementById("millisec_key_holdtime");
    iValue = parseInt(e.value);
    if (iValue > 0 && iValue <= 1000)  global_settings["millisec_key_holdtime"] = iValue;

    opener.updateGlobalSettings(global_settings);
}

function onButtonInputClick(e) {
    elKeyDetect = e;
    elKeyDetect.innerHTML = "Press key ...";
}

function keyDownEventListener(event)
{
    if (elKeyDetect == null) return;

    event.preventDefault();
    event.stopImmediatePropagation();

    // Assign the keycode
    //elKeyDetect.r3e_keycode = event.keyCode;

    if (keyCodeList.has(event.keyCode)) {
        elKeyDetect.r3e_keycode = event.keyCode;
        elKeyDetect.innerHTML = keyCodeList.get(event.keyCode);

        if (event.keyCode == 17) // Control
        {
            if (event.location == 1) {
                elKeyDetect.innerHTML = "LCONTROL";
                elKeyDetect.r3e_keycode = 162;
            }
            else if (event.location == 2) {
                elKeyDetect.innerHTML = "RCONTROL";
                elKeyDetect.r3e_keycode = 163;
            }
            else {
                elKeyDetect.innerHTML = event.code + "[" + event.keyCode + "]";
                elKeyDetect.r3e_keycode = event.keyCode;
            }
        }
        else if (event.keyCode == 16) // Shift
        {
            if (event.location == 1) {
                elKeyDetect.innerHTML = "LSHIFT";
                elKeyDetect.r3e_keycode = 160;
            }
            else if (event.location == 2) {
                elKeyDetect.innerHTML = "RSHIFT";
                elKeyDetect.r3e_keycode = 161;
            }
            else {
                elKeyDetect.innerHTML = keyCodeList[event.keyCode];
                elKeyDetect.r3e_keycode = event.keyCode;
            }
        }
        else if (event.keyCode == 91) {
            if (event.location == 2) {
                elKeyDetect.innerHTML = "RWIN";
                elKeyDetect.r3e_keycode = 92;
            }
        }
    }
    else {
        // We are not sure what it is, but support it anyway ...
        //elKeyDetect.innerHTML = "[" + event.keyCode + "]";

        // We are not sure what the key is, we ignore the key.
        elKeyDetect.r3e_keycode = 0;
        elKeyDetect.innerHTML = "N/A";
    }

    elKeyDetect = null;
}


const keyCodeList = new Map([
    [176, "MEDIA_NEXT_TRACK"],
    [179, "MEDIA_PLAY_PAUSE"],
    [177, "MEDIA_PREV_TRACK"],
    [178, "MEDIA_STOP"],
    [107, "ADD"],
    [106, "MULTIPLY"],
    [111, "DIVIDE"],
    [109, "SUBTRACT"],
    [166, "BROWSER_BACK"],
    [171, "BROWSER_FAVORITES"],
    [167, "BROWSER_FORWARD"],
    [172, "BROWSER_HOME"],
    [168, "BROWSER_REFRESH"],
    [170, "BROWSER_SEARCH"],
    [169, "BROWSER_STOP"],
    [96, "NUMPAD0"],
    [97, "NUMPAD1"],
    [98, "NUMPAD2"],
    [99, "NUMPAD3"],
    [100, "NUMPAD4"],
    [101, "NUMPAD5"],
    [102, "NUMPAD6"],
    [103, "NUMPAD7"],
    [104, "NUMPAD8"],
    [105, "NUMPAD9"],
    [112, "F1"],
    [121, "F10"],
    [122, "F11"],
    [123, "F12"],
    [124, "F13"],
    [125, "F14"],
    [126, "F15"],
    [127, "F16"],
    [128, "F17"],
    [129, "F18"],
    [130, "F19"],
    [113, "F2"],
    [131, "F20"],
    [132, "F21"],
    [133, "F22"],
    [134, "F23"],
    [135, "F24"],
    [114, "F3"],
    [115, "F4"],
    [116, "F5"],
    [117, "F6"],
    [118, "F7"],
    [119, "F8"],
    [120, "F9"],
    [186, "OEM_1"],
    [226, "OEM_102"],
    [191, "OEM_2"],
    [192, "OEM_3"],
    [219, "OEM_4"],
    [220, "OEM_5"],
    [221, "OEM_6"],
    [222, "OEM_7"],
    [223, "OEM_8"],
    [254, "OEM_CLEAR"],
    [188, "OEM_COMMA"],
    [189, "OEM_MINUS"],
    [190, "OEM_PERIOD"],
    [187, "OEM_PLUS"],
    [48, "KEY_0"],
    [49, "KEY_1"],
    [50, "KEY_2"],
    [51, "KEY_3"],
    [52, "KEY_4"],
    [53, "KEY_5"],
    [54, "KEY_6"],
    [55, "KEY_7"],
    [56, "KEY_8"],
    [57, "KEY_9"],
    [65, "KEY_A"],
    [66, "KEY_B"],
    [67, "KEY_C"],
    [68, "KEY_D"],
    [69, "KEY_E"],
    [70, "KEY_F"],
    [71, "KEY_G"],
    [72, "KEY_H"],
    [73, "KEY_I"],
    [74, "KEY_J"],
    [75, "KEY_K"],
    [76, "KEY_L"],
    [77, "KEY_M"],
    [78, "KEY_N"],
    [79, "KEY_O"],
    [80, "KEY_P"],
    [81, "KEY_Q"],
    [82, "KEY_R"],
    [83, "KEY_S"],
    [84, "KEY_T"],
    [85, "KEY_U"],
    [86, "KEY_V"],
    [87, "KEY_W"],
    [88, "KEY_X"],
    [89, "KEY_Y"],
    [90, "KEY_Z"],
    [174, "VOLUME_DOWN"],
    [173, "VOLUME_MUTE"],
    [175, "VOLUME_UP"],
    [44, "SNAPSHOT"],
    [93, "RightClick"],
    [8, "BACKSPACE"],
    [3, "CANCEL"],
    [20, "CAPS_LOCK"],
    [17, "CONTROL"],
    [18, "ALT"],
    [110, "DECIMAL"],
    [46, "DEL"],
    [40, "DOWN"],
    [35, "END"],
    [27, "ESC"],
    [36, "HOME"],
    [45, "INSERT"],
    [182, "LAUNCH_APP1"],
    [183, "LAUNCH_APP2"],
    [180, "LAUNCH_MAIL"],
    [181, "LAUNCH_MEDIA_SELECT"],
    [162, "LCONTROL"],
    [37, "LEFT"],
    [160, "LSHIFT"],
    [91, "LWIN"],
    [34, "PAGEDOWN"],
    [144, "NUMLOCK"],
    [33, "PAGE_UP"],
    [163, "RCONTROL"],
    [13, "ENTER"],
    [39, "RIGHT"],
    [161, "RSHIFT"],
    [92, "RWIN"],
    [16, "SHIFT"],
    [32, "SPACE_BAR"],
    [9, "TAB"],
    [38, "UP"]
]);


/*
       for (const [key, value] of Object.entries(keyCodeList)) {
           document.write("[" + value + ",  \"" + key + "\"],<br>");
       }
       return;

       var keyCodeList =
       {
           MEDIA_NEXT_TRACK: 0xb0,
           MEDIA_PLAY_PAUSE: 0xb3,
           MEDIA_PREV_TRACK: 0xb1,
           MEDIA_STOP: 0xb2,
           ADD: 0x6b,
           MULTIPLY: 0x6a,
           DIVIDE: 0x6f,
           SUBTRACT: 0x6d,
           BROWSER_BACK: 0xa6,
           BROWSER_FAVORITES: 0xab,
           BROWSER_FORWARD: 0xa7,
           BROWSER_HOME: 0xac,
           BROWSER_REFRESH: 0xa8,
           BROWSER_SEARCH: 170,
           BROWSER_STOP: 0xa9,
           NUMPAD0: 0x60,
           NUMPAD1: 0x61,
           NUMPAD2: 0x62,
           NUMPAD3: 0x63,
           NUMPAD4: 100,
           NUMPAD5: 0x65,
           NUMPAD6: 0x66,
           NUMPAD7: 0x67,
           NUMPAD8: 0x68,
           NUMPAD9: 0x69,
           F1: 0x70,
           F10: 0x79,
           F11: 0x7a,
           F12: 0x7b,
           F13: 0x7c,
           F14: 0x7d,
           F15: 0x7e,
           F16: 0x7f,
           F17: 0x80,
           F18: 0x81,
           F19: 130,
           F2: 0x71,
           F20: 0x83,
           F21: 0x84,
           F22: 0x85,
           F23: 0x86,
           F24: 0x87,
           F3: 0x72,
           F4: 0x73,
           F5: 0x74,
           F6: 0x75,
           F7: 0x76,
           F8: 0x77,
           F9: 120,
           OEM_1: 0xba,
           OEM_102: 0xe2,
           OEM_2: 0xbf,
           OEM_3: 0xc0,
           OEM_4: 0xdb,
           OEM_5: 220,
           OEM_6: 0xdd,
           OEM_7: 0xde,
           OEM_8: 0xdf,
           OEM_CLEAR: 0xfe,
           OEM_COMMA: 0xbc,
           OEM_MINUS: 0xbd,
           OEM_PERIOD: 190,
           OEM_PLUS: 0xbb,
           KEY_0: 0x30,
           KEY_1: 0x31,
           KEY_2: 50,
           KEY_3: 0x33,
           KEY_4: 0x34,
           KEY_5: 0x35,
           KEY_6: 0x36,
           KEY_7: 0x37,
           KEY_8: 0x38,
           KEY_9: 0x39,
           KEY_A: 0x41,
           KEY_B: 0x42,
           KEY_C: 0x43,
           KEY_D: 0x44,
           KEY_E: 0x45,
           KEY_F: 70,
           KEY_G: 0x47,
           KEY_H: 0x48,
           KEY_I: 0x49,
           KEY_J: 0x4a,
           KEY_K: 0x4b,
           KEY_L: 0x4c,
           KEY_M: 0x4d,
           KEY_N: 0x4e,
           KEY_O: 0x4f,
           KEY_P: 80,
           KEY_Q: 0x51,
           KEY_R: 0x52,
           KEY_S: 0x53,
           KEY_T: 0x54,
           KEY_U: 0x55,
           KEY_V: 0x56,
           KEY_W: 0x57,
           KEY_X: 0x58,
           KEY_Y: 0x59,
           KEY_Z: 90,
           VOLUME_DOWN: 0xae,
           VOLUME_MUTE: 0xad,
           VOLUME_UP: 0xaf,
           SNAPSHOT: 0x2c,
           RightClick: 0x5d,
           BACKSPACE: 8,
           CANCEL: 3,
           CAPS_LOCK: 20,
           CONTROL: 0x11,
           ALT: 18,
           DECIMAL: 110,
           DEL: 0x2e,
           DOWN: 40,
           END: 0x23,
           ESC: 0x1b,
           HOME: 0x24,
           INSERT: 0x2d,
           LAUNCH_APP1: 0xb6,
           LAUNCH_APP2: 0xb7,
           LAUNCH_MAIL: 180,
           LAUNCH_MEDIA_SELECT: 0xb5,
           LCONTROL: 0xa2,
           LEFT: 0x25,
           LSHIFT: 160,
           LWIN: 0x5b,
           PAGEDOWN: 0x22,
           NUMLOCK: 0x90,
           PAGE_UP: 0x21,
           RCONTROL: 0xa3,
           ENTER: 13,
           RIGHT: 0x27,
           RSHIFT: 0xa1,
           RWIN: 0x5c,
           SHIFT: 0x10,
           SPACE_BAR: 0x20,
           TAB: 9,
           UP: 0x26,
       }; //class enum KeyCode
      
       */