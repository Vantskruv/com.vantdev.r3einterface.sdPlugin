#pragma once

//#define _USE_MATH_DEFINES
#include <Windows.h>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "r3e.h"

/*
BODYWORK 6
FRONT AERO 7
REAR AERO 8
SUSPENSION 9
*/

class cmutex : public std::mutex
{
private:
    bool bIsLocked = false;

public:

    bool try_lock()
    {
        bIsLocked = std::mutex::try_lock();
        return bIsLocked;
    }

    void lock()
    {
        std::mutex::lock();
        bIsLocked = true;

    }

    inline bool is_locked()
    {
        return bIsLocked;
    }

    void unlock()
    {
        std::mutex::unlock();
        bIsLocked = false;
    }
};

class FR3EData
{
private:
    std::mutex mR3EMemory;
    class r3e_shared* r3eSharedData = nullptr;
    HANDLE hMap = NULL;            // Handle to R3E memory.
    HANDLE hProcess = NULL;        // When we detected that R3E is running, assign a handle for its process
    HANDLE hStopEvent = NULL;      // An event handle created to stop threads thread_while_r3e_is_running() and thread_setPitOptions()
    const TCHAR* process_name = TEXT("RRRE64.exe");

    std::array<unsigned short, 7> pitKeys;
    int holdTimeMillisKeyPress = 10;
    int waitBetweenEachCommands = 200;
    int waitBetweenEachFuelStep = 20;


    bool bIsR3ERunning = false;
    class std::thread tWhileR3EIsRunning;   // Thread object for thread_while_r3e_is_running()
    
    class std::thread tSetPitOptions;       // Thread object for thread_setPitOptions(...)
    bool abortOnGoingPitOptions = false;    // A flag to cancel executing currentPitOptions. If quedPitOptions is not empty, it will start executing that.
    bool bIsPitOptionsRunning = false;      // Is thread_setPitOptions executing currentPitOptions? (note, this does not mean that the thread is running or not).
    bool bDoSetPitOptions = false;          // Set it to true to start executing set pitoptions.
    bool bExitThreadPitOptions = false;     // Set this flag to true to exit thread_setPitOptions()


    /* Stores current pitoptions being applied. Lists can be empty.
    */
    std::vector<std::pair<int, bool>> currentPitOptions;
    int current_refuel_option = 0;
    bool current_request_boxthislap = false;
    bool current_close_pit_menu = true;
    std::vector<std::pair<int, bool>> quedPitOptions;
    int qued_refuel_option = 0;
    bool qued_request_boxthislap = false;
    bool qued_close_pit_menu = true;

    std::mutex mSetQuedPitOptions;              // Mutexs for setting the pitoptions from setPitOption(...)
    std::mutex mSetCurrentPitOptions;
    std::condition_variable cvRunPitOptions;    // Waits the tSetPitOptions thread if nothing needs to be done.
    
    
    bool is_r3e_running();
    bool init_shared_memory();
    void close_shared_memory();
    void thread_while_r3e_is_running(); // A thread running while R3e is running. It sets bIsR3ERunning to true, and when the thread ends, sets bIsR3ERunning to false aswell as calling close_shared_memory().
    
    void thread_setPitOptions();        // A thread running while R3E is running, executing pitoptions if currentPitOptions is not empty.
    bool setPitOption(int selection, bool bOn, int refuel_option = 0);  // Called for every pit-option from trhead_setPitOptions(...)
    float calculateFuel(); // Called by setPitOption() when needed

public:
    unsigned short PIT_MENU_UP = 0x57;        // W
    unsigned short PIT_MENU_DOWN = 0x53;      // S
    unsigned short PIT_MENU_LEFT = 0x41;      // A
    unsigned short PIT_MENU_RIGHT = 0x44;     // D
    unsigned short PIT_MENU_ENTER = 0x45;     // E
    unsigned short PIT_REQUEST_BOX = 0x52;    // R
    unsigned short PIT_TOGGLE_MENU = 0x51;    // Q

    bool setPitOptions(const std::vector<std::pair<int, bool>>& pitOptions, int refuel_option = 0, bool bRequestBoxThisLap = false, bool bClosePitMenu = true, bool bAbortOngoingPitOptions = false);
    bool isPitOptionsRunning();
    //std::string getPitStopDuration();

    FR3EData();
    ~FR3EData();
};

