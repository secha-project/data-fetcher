#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

const int MIN_SLEEP_DURATION_MINUTES = 1;
const int TARGET_HOUR = 4;
const int MAX_FAILS = 5;
const int FAIL_SLEEP_DURATION_MINUTES = 5;


std::string tmToDateString(std::tm& inputTime)
{
    std::mktime(&inputTime);
    std::ostringstream oss;
    oss << std::put_time(&inputTime, "%Y-%m-%d");
    return oss.str();
}

// Function to get the next date given a current date in YYYY-MM-DD format
std::string getNextDate(const std::string& date)
{
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");

    tm.tm_mday += 1;
    return tmToDateString(tm);
}


// Function to get the current maximum date (yesterday or day before yesterday if before 4 AM)
std::string getCurrentMaximumDate()
{
    // Get the current time
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);

    // Convert to local time
    std::tm now_tm = *std::localtime(&now_time_t);

    if (now_tm.tm_hour < TARGET_HOUR)
    {
        now_tm.tm_mday -= 2;
    }
    else
    {
        now_tm.tm_mday -= 1;
    }
    return tmToDateString(now_tm);
}


auto getSleepDuration()
{
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);

    std::tm nextRun_tm = now_tm;
    nextRun_tm.tm_hour = TARGET_HOUR;
    nextRun_tm.tm_min = 0;
    nextRun_tm.tm_sec = 0;

    if (now_tm.tm_hour >= TARGET_HOUR) {
        nextRun_tm.tm_mday += 1;
    }

    // Calculate the time difference
    auto nextRun_time_t = std::mktime(&nextRun_tm);
    auto nextRun = std::chrono::system_clock::from_time_t(nextRun_time_t);
    return nextRun - now;
}


// Function to execute the data handling script with a given date
int executeScriptWithDate(const std::string& date)
{
    std::string command = "bash handle_data.sh " + date;
    std::cout << "\nExecuting command: \"" << command << "\"" << std::endl;

    return std::system(command.c_str());
}

void printErrorMessage(const std::string& date) {
    std::cerr << "\nError executing script for date " << date << ". Exiting." << std::endl;
}

bool handleError(const std::string currentDate, int currentFailCount)
{
    if (currentFailCount > MAX_FAILS)
    {
        printErrorMessage(currentDate);
        return true;
    }

    std::cout << "\nTrying again in " << FAIL_SLEEP_DURATION_MINUTES << " minute(s)..." << std::endl;
    std::this_thread::sleep_for(std::chrono::minutes(FAIL_SLEEP_DURATION_MINUTES));
    return false;
}


int mainLoop(const std::string& initialDate)
{
    std::string currentDate = initialDate;
    int failCount = 0;

    // Initial catch-up loop to process any missed dates
    while (currentDate <= getCurrentMaximumDate()) {
        failCount = 0;
        auto returnCode = executeScriptWithDate(currentDate);
        if (returnCode != 0) {
            ++failCount;
            if (handleError(currentDate, failCount))
            {
                // too many fails in a row
                return returnCode;
            }
            continue;
        }

        currentDate = getNextDate(currentDate);
        std::cout << "\nWaiting for " << MIN_SLEEP_DURATION_MINUTES << " minute(s) before next execution..." << std::endl;
        std::this_thread::sleep_for(std::chrono::minutes(MIN_SLEEP_DURATION_MINUTES));
    }

    // Main loop to execute the script at 4 AM every day
    while (true) {
        failCount = 0;
        auto sleepDuration = getSleepDuration();
        std::cout << "\nSleeping for "
                  << std::chrono::duration_cast<std::chrono::hours>(sleepDuration).count() << " hours and "
                  << (std::chrono::duration_cast<std::chrono::minutes>(sleepDuration).count() % 60) << " minutes until next " << TARGET_HOUR << " AM."
                  << std::endl;

        std::this_thread::sleep_for(sleepDuration);

        auto returnCode = executeScriptWithDate(currentDate);
        if (returnCode != 0) {
            ++failCount;
            if (handleError(currentDate, failCount))
            {
                // too many fails in a row
                return returnCode;
            }
            continue;
        }

        currentDate = getNextDate(currentDate);
    }

    return 0;
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <initial-date>" << std::endl;
        return 1;
    }

    return mainLoop(argv[1]);
}
