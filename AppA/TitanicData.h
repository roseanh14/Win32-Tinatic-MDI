#pragma once
#include <windows.h>
#include <string>
#include <vector>

struct Passenger {
    int          passengerId;   // Unique number for this passenger
    int          survived;      // 1 = survived, 0 = died
    int          pclass;        // Ticket class: 1, 2, or 3
    std::wstring name;          // Full name (wstring = wide string = supports Unicode)
    std::wstring sex;           // "male" or "female"
    float        age;           // Age in years (-1 if unknown)
    int          sibSp;         // Number of siblings/spouses aboard
    int          parch;         // Number of parents/children aboard
    std::wstring ticket;        // Ticket number
    float        fare;          // Ticket price
    std::wstring cabin;         // Cabin number (can be empty)
    std::wstring embarked;      // Port: C=Cherbourg, Q=Queenstown, S=Southampton
};

class TitanicData {
public:
    static bool                          LoadCSV(const std::wstring& path);
    static const std::vector<Passenger>& GetPassengers();   // Returns all passengers
    static std::wstring                  ToTabDelimited();   // For clipboard/pipe/DDE
    static std::wstring                  GetSummaryLine(const Passenger& p); // One display line

private:
    static std::vector<Passenger> s_passengers; // The actual data lives here
};