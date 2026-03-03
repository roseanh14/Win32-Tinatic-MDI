#include "TitanicData.h"
#include <fstream>      // For reading files
#include <sstream>      // For splitting strings

// The actual storage (defined here, declared as static in .h)
std::vector<Passenger> TitanicData::s_passengers;

// Helper: convert regular string to wide string (Windows prefers wide strings)
static std::wstring Wide(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

// Helper: remove surrounding quotes from CSV fields like "Smith, John"
static std::string StripQuotes(std::string s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        s = s.substr(1, s.size() - 2);
    return s;
}

bool TitanicData::LoadCSV(const std::wstring& path) {
    std::ifstream file(path);           // Open the file
    if (!file.is_open()) return false;  // Return false if it doesn't exist

    std::string line;
    std::getline(file, line);           // Skip the header row (ID,Survived,Pclass,...)

    while (std::getline(file, line)) {  // Read one row at a time
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string token;
        Passenger p{};                  // Create an empty passenger
        int col = 0;

        while (std::getline(ss, token, ',')) {   // Split by comma
            token = StripQuotes(token);
            try {
                switch (col) {
                    case 0:  p.passengerId = token.empty() ? 0    : std::stoi(token); break;
                    case 1:  p.survived   = token.empty() ? 0    : std::stoi(token); break;
                    case 2:  p.pclass     = token.empty() ? 0    : std::stoi(token); break;
                    case 3:  p.name       = Wide(token); break;
                    case 4:  p.sex        = Wide(token); break;
                    case 5:  p.age        = token.empty() ? -1.f : std::stof(token); break;
                    case 6:  p.sibSp      = token.empty() ? 0    : std::stoi(token); break;
                    case 7:  p.parch      = token.empty() ? 0    : std::stoi(token); break;
                    case 8:  p.ticket     = Wide(token); break;
                    case 9:  p.fare       = token.empty() ? 0.f  : std::stof(token); break;
                    case 10: p.cabin      = Wide(token); break;
                    case 11: p.embarked   = Wide(token); break;
                }
            } catch (...) {}  // Skip bad values silently
            col++;
        }
        if (col >= 4) s_passengers.push_back(p);  // Save if row had enough columns
    }
    return !s_passengers.empty();
}

const std::vector<Passenger>& TitanicData::GetPassengers() {
    return s_passengers;
}

// One nicely formatted line per passenger (for OA1 listbox display)
std::wstring TitanicData::GetSummaryLine(const Passenger& p) {
    wchar_t buf[300];
    swprintf(buf, 300,
        L"[%3d] %-28s | Cl:%d | %-6s | Age:%4.0f | Fare:%6.2f | %s",
        p.passengerId, p.name.c_str(), p.pclass, p.sex.c_str(),
        p.age, p.fare, p.survived ? L"SURVIVED" : L"died");
    return buf;
}

// Tab-separated text (for clipboard, DDE, pipe transfer)
std::wstring TitanicData::ToTabDelimited() {
    std::wstring out = L"ID\tSurvived\tClass\tName\tSex\tAge\tFare\r\n";
    for (auto& p : s_passengers) {
        wchar_t buf[512];
        swprintf(buf, 512, L"%d\t%d\t%d\t%s\t%s\t%.1f\t%.2f\r\n",
            p.passengerId, p.survived, p.pclass,
            p.name.c_str(), p.sex.c_str(), p.age, p.fare);
        out += buf;
    }
    return out;
}