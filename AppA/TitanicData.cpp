#include "TitanicData.h"
#include <fstream>
#include <sstream>

std::vector<Passenger> TitanicData::s_passengers;

static std::wstring Wide(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

static std::vector<std::string> ParseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                field += '"'; i++; 
            }
            else {
                inQuotes = !inQuotes; 
            }
        }
        else if (c == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        }
        else {
            field += c;
        }
    }
    fields.push_back(field); 
    return fields;
}

bool TitanicData::LoadCSV(const std::wstring& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    std::getline(file, line); 

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        auto fields = ParseCSVLine(line);
        if (fields.size() < 5) continue;

        Passenger p{};
        try {
            p.passengerId = fields[0].empty() ? 0 : std::stoi(fields[0]);
            p.survived = fields[1].empty() ? 0 : std::stoi(fields[1]);
            p.pclass = fields[2].empty() ? 0 : std::stoi(fields[2]);
            p.name = Wide(fields[3]);
            p.sex = Wide(fields[4]);
            p.age = fields[5].empty() ? -1.f : std::stof(fields[5]);
            p.sibSp = fields[6].empty() ? 0 : std::stoi(fields[6]);
            p.parch = fields[7].empty() ? 0 : std::stoi(fields[7]);
            p.ticket = Wide(fields[8]);
            p.fare = fields[9].empty() ? 0.f : std::stof(fields[9]);
            p.cabin = fields.size() > 10 ? Wide(fields[10]) : L"";
            p.embarked = fields.size() > 11 ? Wide(fields[11]) : L"";
        }
        catch (...) {}

        s_passengers.push_back(p);
    }
    return !s_passengers.empty();
}

const std::vector<Passenger>& TitanicData::GetPassengers() {
    return s_passengers;
}

std::wstring TitanicData::GetSummaryLine(const Passenger& p) {
    wchar_t buf[300];
    swprintf(buf, 300,
        L"[%3d] %-30s | Cl:%d | %-6s | Age:%4.0f | Fare:%7.2f | %s",
        p.passengerId, p.name.c_str(), p.pclass, p.sex.c_str(),
        p.age, p.fare, p.survived ? L"SURVIVED" : L"died");
    return buf;
}

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