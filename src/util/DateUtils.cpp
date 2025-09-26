#include "../../include/util/DateUtils.h"

std::tm DateUtils::getDateFromString(const std::string& dateString)
{
    std::tm tm = {};
    std::istringstream ss(dateString);

    // Parse the date string using std::get_time
    ss >> std::get_time(&tm, "%Y%m%d");

    if (ss.fail()) {
        std::cout << "Date parsing failed!" << std::endl;
        return tm;
    }
    return tm;
}