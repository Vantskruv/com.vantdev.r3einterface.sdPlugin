#include "FR3EData.h"
#include "FKeyCommand.h"

#include <tchar.h> //wprintf
#include <TlHelp32.h>
#include <thread>
#include <sstream>
#include <iomanip>


FR3EData::FR3EData()
{
    hStopEvent = CreateEvent(nullptr, true, false, nullptr);
    if (hStopEvent == NULL)
    {
        throw("In constructor FR3EData(): Failed to create event handle hStopEvent.");
    }
}

FR3EData::~FR3EData()
{
    mR3EMemory.lock();
    close_shared_memory();
    CloseHandle(hStopEvent);
    mR3EMemory.unlock();
}


// Closes current open data and threads, and tries to find R3E process.
// If found, it allocates shared memory, and starts threads.
// Make sure mR3EMemory mutex is locked before calling this function.
bool FR3EData::init_shared_memory()
{
    close_shared_memory();

    if (!is_r3e_running()) return false;
    
    
    hMap = OpenFileMapping(FILE_MAP_READ, FALSE, TEXT(R3E_SHARED_MEMORY_NAME));
    if (hMap == NULL)
    {
        close_shared_memory();
        return false;
    }

    r3eSharedData = (r3e_shared*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, sizeof(r3e_shared));
    if (r3eSharedData == NULL)
    {
        close_shared_memory();
        return false;
    }

    abortOnGoingPitOptions = true;
    bDoSetPitOptions = false;
    bExitThreadPitOptions = false;
    tSetPitOptions = std::thread(&FR3EData::thread_setPitOptions, this);
    while (abortOnGoingPitOptions); // Ugly wait until it is set. To make sure the thread is started before things happening.

    tWhileR3EIsRunning = std::thread(&FR3EData::thread_while_r3e_is_running, this);

    return true;
}

// Close current open data and threads.
// Make sure mR3EMemory mutex is locked before calling this function.
void FR3EData::close_shared_memory()
{
    SetEvent(hStopEvent);
    abortOnGoingPitOptions = true;
    if (tWhileR3EIsRunning.joinable()) tWhileR3EIsRunning.join();
    if (hProcess) CloseHandle(hProcess);
    if (r3eSharedData) UnmapViewOfFile(r3eSharedData);
    if (hMap) CloseHandle(hMap);
    hProcess = NULL;
    r3eSharedData = nullptr;
    hMap = NULL;
    ResetEvent(hStopEvent);
    abortOnGoingPitOptions = false;
}

void FR3EData::thread_while_r3e_is_running()
{
    HANDLE hEvents[] = { hProcess, hStopEvent };

    bIsR3ERunning = true;
    WaitForMultipleObjects(2, hEvents, false, INFINITE);

    if(tSetPitOptions.joinable())
    {
        bExitThreadPitOptions = true;
        cvRunPitOptions.notify_one();
        tSetPitOptions.join();

    }
    bIsR3ERunning = false;
}


// Search and check if RRRE64.exe is running. If it is running, assign a handle to process_handle and return true
// Should not be called while if handle hProcess is not closed.
bool FR3EData::is_r3e_running()
{
    BOOL result = FALSE;
    HANDLE snapshot = NULL;
    PROCESSENTRY32 entry;

    ZeroMemory(&entry, sizeof(entry));
    entry.dwSize = sizeof(PROCESSENTRY32);

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE)
    {
        if (Process32First(snapshot, &entry))
        {
            do
            {
                if (_tcscmp(entry.szExeFile, process_name) == 0)
                {
                    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
                    result = TRUE;
                    break;
                }
            } while (Process32Next(snapshot, &entry));
        }
        CloseHandle(snapshot);
    }

    return result;
}

// Executes one pitoption.
// Returns FALSE if menu or option is not accessable.
// Returns true if succeeded, or if abortOnGoinPitOptions is set.
bool FR3EData::setPitOption(int selection, bool bOn, int refuel_option)
{
    unsigned int navTries = 0;
    unsigned int navTriesMAX = 5;

    if (r3eSharedData->pit_menu_state[selection] == -1)
    {
        return true;
    }

    // If option is not refuel, and the selection already as requested, we do not need to do anything.
    if (selection != R3E_PIT_MENU_FUEL && r3eSharedData->pit_menu_state[selection] == (int)bOn) return true;

    int pSelection = r3eSharedData->pit_menu_selection;

    while (r3eSharedData->pit_menu_state[selection] != -1 && r3eSharedData->pit_menu_selection < selection)
    {
        // Go down until option is marked
        if (abortOnGoingPitOptions) return true;
        FKeyCommand::SendScanCodeKeyPress(PIT_MENU_DOWN, holdTimeMillisKeyPress);
        std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
        if (pSelection == r3eSharedData->pit_menu_selection)
        {
            navTries++;
            if (navTries > navTriesMAX) return false;
            continue;
        }
        pSelection = r3eSharedData->pit_menu_selection;
    }

    while (r3eSharedData->pit_menu_state[selection] != -1 && r3eSharedData->pit_menu_selection > selection)
    {
        // Go up until option is marked
        if (abortOnGoingPitOptions) return true;
        FKeyCommand::SendScanCodeKeyPress(PIT_MENU_UP, holdTimeMillisKeyPress);
        std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
        if (pSelection == r3eSharedData->pit_menu_selection)
        {
            navTries++;
            if (navTries > navTriesMAX) return false;
            continue;
        }
        pSelection = r3eSharedData->pit_menu_selection;
    }

    // Check if we really ended on the right selection
    if (r3eSharedData->pit_menu_selection != selection) return false;


    if (selection == R3E_PIT_MENU_FUEL)
    {
        if (r3eSharedData->pit_menu_state[selection] == 1)
        {
            FKeyCommand::SendScanCodeKeyPress(PIT_MENU_ENTER, holdTimeMillisKeyPress);
            std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
        }

        if (!bOn) return true;

        // Iterate to beginning of options
        float totalCapacity = r3eSharedData->fuel_capacity;
        for (int i = 0; i < (totalCapacity + 4); i++)
        {
            if (r3eSharedData->pit_menu_state[selection] == -1) return false;
            if (abortOnGoingPitOptions) return true;
            FKeyCommand::SendScanCodeKeyPress(PIT_MENU_RIGHT, holdTimeMillisKeyPress);
            std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachFuelStep));
        }

        // If refuel to optimum level
        if (refuel_option == 3)
        {
            float liters = calculateFuel();
            if (liters < 0.0f) return true;
            if (liters > totalCapacity) liters = totalCapacity;

            liters += 3; //Skip forward through safe, normal and risky options.
            for (int i = 0; i < liters; i++)
            {
                if (r3eSharedData->pit_menu_state[selection] == -1) return false;
                if (abortOnGoingPitOptions) return true;
                FKeyCommand::SendScanCodeKeyPress(PIT_MENU_LEFT, holdTimeMillisKeyPress);
                std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachFuelStep));
            }
        }
        else
        {
            // If refuel safe, normal or risky
            for (int i = 0; i < refuel_option; i++)
            {
                FKeyCommand::SendScanCodeKeyPress(PIT_MENU_LEFT, holdTimeMillisKeyPress);
                std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachFuelStep));
            }
        }
    }

    // Select the option
    FKeyCommand::SendScanCodeKeyPress(PIT_MENU_ENTER, holdTimeMillisKeyPress);

    return true;
}


bool FR3EData::isPitOptionsRunning()
{
    return bIsPitOptionsRunning;
}

bool FR3EData::setPitOptions(const std::vector<std::pair<int, bool>>& pitOptions, int refuel_option, bool bRequestBoxThisLap, bool bClosePitMenu, bool bAbortOngoingPitOptions)
{
    /*
    // Test code while R3e is not running
    FKeyCommand::SendScanCodeKeyPress(PIT_MENU_UP, holdTimeMillisKeyPress);
    std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
    FKeyCommand::SendScanCodeKeyPress(PIT_MENU_DOWN, holdTimeMillisKeyPress);
    std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
    FKeyCommand::SendScanCodeKeyPress(PIT_MENU_LEFT, holdTimeMillisKeyPress);
    std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
    FKeyCommand::SendScanCodeKeyPress(PIT_MENU_RIGHT, holdTimeMillisKeyPress);
    std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
    FKeyCommand::SendScanCodeKeyPress(PIT_MENU_ENTER, holdTimeMillisKeyPress);
    std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
    FKeyCommand::SendScanCodeKeyPress(PIT_REQUEST_BOX, holdTimeMillisKeyPress);
    std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
    FKeyCommand::SendScanCodeKeyPress(PIT_TOGGLE_MENU, holdTimeMillisKeyPress);
    std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));

    if (1 == 1) return false;
    */


    if (!bIsR3ERunning)
    {
        std::scoped_lock<std::mutex> lock(mR3EMemory);
        if (!init_shared_memory()) return false;
    }

    if (mSetCurrentPitOptions.try_lock())
    {
        currentPitOptions = pitOptions;
        current_refuel_option = refuel_option;
        current_request_boxthislap = bRequestBoxThisLap;
        current_close_pit_menu = bClosePitMenu;
        
        bDoSetPitOptions = true;
        mSetCurrentPitOptions.unlock();
        cvRunPitOptions.notify_one();
    }
    else if(bAbortOngoingPitOptions)
    {
        mSetQuedPitOptions.lock();
        quedPitOptions = pitOptions;
        qued_refuel_option = refuel_option;
        qued_request_boxthislap = bRequestBoxThisLap;
        qued_close_pit_menu = bClosePitMenu;

        abortOnGoingPitOptions = true;
        bDoSetPitOptions = true;
        mSetQuedPitOptions.unlock();
        cvRunPitOptions.notify_one();
    }

    return true;
}


/* Loop polling if currentPitOptions is set, and if set, execute the pitOptions.*/
void FR3EData::thread_setPitOptions()
{
    //HANDLE hEvents[] = { hProcess, hStopEvent, hExecutePitOptions };
    
    // If R3E process is not running, or the program wants to end, exit the thread.
    //while (WaitForMultipleObjects(3, hEvents, false, INFINITE))
    while(!bExitThreadPitOptions)
    {
        std::unique_lock<std::mutex> lckCurrent(mSetCurrentPitOptions, std::defer_lock);
        std::unique_lock<std::mutex> lckQued(mSetQuedPitOptions, std::defer_lock);
        std::lock(lckCurrent, lckQued);
        cvRunPitOptions.wait(lckCurrent, [&]
        { 
            if (abortOnGoingPitOptions)
            {
                currentPitOptions.clear();
                current_request_boxthislap = false;
                abortOnGoingPitOptions = false;
            }

            if (bDoSetPitOptions || bExitThreadPitOptions) return true;
            bIsPitOptionsRunning = false;
            return false;
        });

        bDoSetPitOptions = false;
        if (bExitThreadPitOptions) break;
        bIsPitOptionsRunning = true;

        if (quedPitOptions.size())
        {
            currentPitOptions = quedPitOptions;
            current_refuel_option = qued_refuel_option;
            current_request_boxthislap = qued_request_boxthislap;
            current_close_pit_menu = qued_close_pit_menu;
            
            qued_close_pit_menu = true;
            qued_request_boxthislap = false;
            quedPitOptions.clear();
        }
        lckQued.unlock();

        if (r3eSharedData->session_type == R3E_SESSION_UNAVAILABLE || r3eSharedData->control_type != R3E_CONTROL_PLAYER)
        {
            currentPitOptions.clear();
            current_request_boxthislap = false;
            current_close_pit_menu = true;
            continue;
        }

        // Enter pit-menu if we are not already there. If the attempt is unsuccessfull, we return false.
        if (r3eSharedData->pit_menu_selection == R3E_PIT_MENU_UNAVAILABLE)
        {
            FKeyCommand::SendScanCodeKeyPress(PIT_TOGGLE_MENU, holdTimeMillisKeyPress);
            std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
            if (r3eSharedData->pit_menu_selection == R3E_PIT_MENU_UNAVAILABLE)
            {
                currentPitOptions.clear();
                current_request_boxthislap = false;
                current_close_pit_menu = true;
                continue;
            }
        }

        //Note, if option 10 is 1, option 11 is 0, but not selectable.
        //If we press E on option 10 while it is 1, it changes its value to -1, while 11 will be 1, and 11 will be the current selection.
        //If we then press E on option 11, option 10 will be the current selection, which is set to 1, and 11 will be set to 0 and not selectable as before.
        //If option 10 is selectable, and we press that one, we have requested pitin.
        //If option 11 is selectable, and we press that one, the pitin in canceled.

        //While the fuel option is marked, pressing D scrolls the option to the right, and S to the left.
        //From the right we have, SAFE, NORMAL, RISKY, and the N liters (up to the total fuel capacity of the car)

        bool bSuccess = true;
        for (auto iPitOption : currentPitOptions)
        {
            if (abortOnGoingPitOptions) break;
            if (!setPitOption(iPitOption.first, iPitOption.second, current_refuel_option)) { bSuccess = false; break; }
            std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
        }
        
        if (bSuccess && !abortOnGoingPitOptions)
        {
            if (current_request_boxthislap && r3eSharedData->pit_state <1) // Request pitstop
            {
                // Current vehicle pit state (-1 = N/A, 0 = None, 1 = Requested stop, 2 = Entered pitlane heading for pitspot, 3 = Stopped at pitspot, 4 = Exiting pitspot heading for pit exit)
                FKeyCommand::SendScanCodeKeyPress(PIT_REQUEST_BOX, holdTimeMillisKeyPress);
                std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
            }
            else if (!current_request_boxthislap && r3eSharedData->pit_state == 1)  // Cancel pitstop request
            {
                FKeyCommand::SendScanCodeKeyPress(PIT_REQUEST_BOX, holdTimeMillisKeyPress);
                std::this_thread::sleep_for(std::chrono::milliseconds(waitBetweenEachCommands));
            }
            
            if(current_close_pit_menu) FKeyCommand::SendScanCodeKeyPress(PIT_TOGGLE_MENU, holdTimeMillisKeyPress);
        }

        currentPitOptions.clear();
        current_request_boxthislap = false;
        current_close_pit_menu = true;
    }
}

float FR3EData::calculateFuel()
{
    /*
    wprintf_s(L"FuelCapactiy: %f\n", map_buffer->fuel_capacity); //How much the car can have
    wprintf_s(L"FuelLeft: %f\n", map_buffer->fuel_left);         //How much fuel we currently have
    wprintf_s(L"FuelPerLap: %f\n", map_buffer->fuel_per_lap);    //Calculated fuel use per lap
    wprintf_s(L"FuelUseActive: %d\n", map_buffer->fuel_use_active); //1x, 2x, 3x and so on
    wprintf_s(L"FuelPressure: %f\n", map_buffer->fuel_pressure);
    wprintf_s(L"LapDistanceFraction: %f\n", map_buffer->lap_distance_fraction); //0.0 to 1.0
    wprintf_s(L"TotalNumberOfLapsInRace: %d\n", map_buffer->number_of_laps); //Sames as SessionFormatLaps -1 when time session, also when time+laps session.
    wprintf_s(L"CompletedLaps: %d\n", map_buffer->completed_laps);
    wprintf_s(L"SessionLengthFormat: %d\n", map_buffer->session_length_format); // TIME_BASED, LAP_BASED, or BOTH
    wprintf_s(L"SessionFormatMinutes: %d\n", map_buffer->race_session_minutes[0]);
    wprintf_s(L"SessionFormatLaps: %d\n", map_buffer->race_session_laps[0]); //-1 when time, if time + laps, shows extra laps after time
    wprintf_s(L"SessionTimeDuration: %f\n", map_buffer->session_time_duration); //Total session time (same as SessuibFormatMinutes but in seconds). -1 when laps
    wprintf_s(L"SessionTimeRemaining: %f\n", map_buffer->session_time_remaining); //Seconds of current session remaining. -1 when laps
    wprintf_s(L"LapTimeBestSelf: %f\n", map_buffer->lap_time_best_self);
    */

    int sessionFormat = r3eSharedData->session_length_format;
    
    if (sessionFormat == R3E_SESSION_LENGTH_TIME_BASED || sessionFormat == R3E_SESSION_LENGTH_TIME_AND_LAP_BASED)
    {
        float sessionTimeRemaining = r3eSharedData->session_time_remaining;
        float bestLapTime = r3eSharedData->lap_time_best_self;
        if (sessionTimeRemaining < 0 || bestLapTime < 0) return -1.0f;
        
        float approxLaps = sessionTimeRemaining / bestLapTime;
        int sessionIterator = r3eSharedData->session_iteration - 1;
        if (sessionFormat == R3E_SESSION_LENGTH_TIME_AND_LAP_BASED && sessionIterator >= 0) approxLaps += r3eSharedData->race_session_laps[sessionIterator];
        float fuelPerLap = r3eSharedData->fuel_per_lap;
        float fuelCalc = approxLaps * fuelPerLap;

        //Extra margin
        fuelCalc += fuelPerLap;
        float fuelLeft = r3eSharedData->fuel_left;
        if (fuelCalc < fuelLeft) return -1.0f;

        return fuelCalc;
    }
    
    if (sessionFormat == R3E_SESSION_LENGTH_LAP_BASED)
    {
        int totalNumberOfLaps = r3eSharedData->number_of_laps;
        int completedLaps = r3eSharedData->completed_laps + 1;
        if (totalNumberOfLaps < 0 || completedLaps < 0) return -1.0f;

        float lapsRemaining = totalNumberOfLaps - completedLaps - r3eSharedData->lap_distance_fraction;
        float fuelPerLap = r3eSharedData->fuel_per_lap;

        float fuelCalc = lapsRemaining*fuelPerLap;
        fuelCalc += fuelPerLap; //Extra margin

        float fuelLeft = r3eSharedData->fuel_left;
        if (fuelCalc < fuelLeft) return -1.0f;

        return fuelCalc;
    }

    return -1.0f;
}


/*
std::string FR3EData::getPitStopDuration()
{
    float pitStopDuration = (bIsR3ERunning ? r3eSharedData->pit_min_duration_total : -666.6);
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << pitStopDuration << "s";
    return stream.str();
}
*/