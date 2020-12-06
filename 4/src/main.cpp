#include <imgui/imgui.h>
#include <onut/ContentManager.h>
#include <onut/Renderer.h>
#include <onut/Settings.h>
#include <onut/Strings.h>

void initSettings()
{
    oSettings->setGameName("Advent of Code 4");
    oSettings->setIsResizableWindow(true);
    oSettings->setResolution({1280, 720});
    oSettings->setShowFPS(true);
    oSettings->setShowOnScreenLog(true);
}

void shutdown()
{
}

void update()
{
}

void render()
{
    oRenderer->clear();
}

void postRender()
{
}

#include <chrono>
#include <future>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <onut/Async.h>
#include <onut/Files.h>
#include <onut/Dispatcher.h>
#include <onut/Point.h>

using namespace std;
using namespace chrono_literals;

int processing_count = 0;
int result1 = -1;
int result2 = -1;

std::vector<std::string> getDataLines()
{
    // Load content
    auto filename = oContentManager->findResourceFile("puzzle_data.txt");
    auto raw_data = onut::getFileData(filename);
    std::string data_string;
    data_string.resize(raw_data.size());
    memcpy(data_string.data(), raw_data.data(), raw_data.size());

    onut::replace(data_string, "\r", "");

    // Split lines
    return std::move(onut::splitString(data_string, '\n', false));
}

struct Field
{
    bool optional = false;
    string value;
    bool valid = false;
};

struct Passport
{
    string raw;
    map<string, Field> fields = {
        { "byr", {}},
        { "iyr", {}},
        { "eyr", {}},
        { "hgt", {}},
        { "hcl", {}},
        { "ecl", {}},
        { "pid", {}},
        { "cid", { true }}
    };
};

vector<Passport> passports;

string findField(const string& name, const string& raw)
{
    auto from = raw.find(name + ":");
    if (from == string::npos)
        return "";

    auto to = raw.find(' ', from);
    if (to == string::npos)
        to = raw.size();

    return raw.substr(from + 4, to - (from + 4));
}

bool validateBYR(const string& value)
{
    if (value.size() != 4)
        return false;

    try
    {
        int year = stoi(value);
        return year >= 1920 && year <= 2002;
    }
    catch (...) {}

    return false;
}

bool validateIYR(const string& value)
{
    if (value.size() != 4)
        return false;

    try
    {
        int year = stoi(value);
        return year >= 2010 && year <= 2020;
    }
    catch (...) {}

    return false;
}

bool validateEYR(const string& value)
{
    if (value.size() != 4)
        return false;

    try
    {
        int year = stoi(value);
        return year >= 2020 && year <= 2030;
    }
    catch (...) {}

    return false;
}

bool validateHGT(const string& value)
{
    auto cm_pos = value.find("cm");
    if (cm_pos != string::npos)
    {
        if (cm_pos != value.size() - 2)
            return false;

        try
        {
            int cm = stoi(value.substr(0, value.size() - 2));
            return cm >= 150 && cm <= 193;
        }
        catch (...) {}
        return false;
    }

    auto in_pos = value.find("in");
    if (in_pos != string::npos)
    {
        if (in_pos != value.size() - 2)
            return false;

        try
        {
            int in = stoi(value.substr(0, value.size() - 2));
            return in >= 59 && in <= 76;
        }
        catch (...) {}
        return false;
    }

    return false;
}

bool validateHCL(const string& value)
{
    if (value.size() != 7)
        return false;

    if (value[0] != '#')
        return false;

    for (int i = 1; i < 7; ++i)
    {
        auto c = value[i];
        if ((c >= '0' && c <= '9') || 
            (c >= 'a' && c <= 'f'))
            continue;
        return false;
    }

    return true;
}

bool validateECL(const string& value)
{
    return 
        value == "amb" ||
        value == "blu" ||
        value == "brn" ||
        value == "gry" ||
        value == "grn" ||
        value == "hzl" ||
        value == "oth";
}

bool validatePID(const string& value)
{
    if (value.size() != 9)
        return false;

    for (int i = 0; i < 9; ++i)
    {
        auto c = value[i];
        if (c >= '0' && c <= '9')
            continue;
        return false;
    }
    return true;
}

bool validateField(const string& name, const string& value)
{
    if (name == "byr")
        return validateBYR(value);
    if (name == "iyr")
        return validateIYR(value);
    if (name == "eyr")
        return validateEYR(value);
    if (name == "hgt")
        return validateHGT(value);
    if (name == "hcl")
        return validateHCL(value);
    if (name == "ecl")
        return validateECL(value);
    if (name == "pid")
        return validatePID(value);

    return !value.empty();
}

void parseData()
{
    auto lines = getDataLines();

    // Collect grouped lines into raw passport
    Passport current_passport;
    for (auto& line : lines)
    {
        if (line.empty())
        {
            if (!current_passport.raw.empty())
                passports.push_back(current_passport);
            current_passport = {};
            continue;
        }
        if (current_passport.raw.empty())
            current_passport.raw = line;
        else
            current_passport.raw += " " + line;
    }
    if (!current_passport.raw.empty())
        passports.push_back(current_passport);

    // Parse fields (We could have regex this, but its faster to write it than re-figure out C++11 regex)
    for (auto& passport : passports)
    {
        for (auto& kv : passport.fields)
        {
            kv.second.value = findField(kv.first, passport.raw);
            kv.second.valid = validateField(kv.first, kv.second.value);
        }
    }
}

string to_string(const Passport& passport)
{
    string ret;
    
    for (auto& kv : passport.fields)
    {
        if (kv.second.value.empty())
            continue;

        ret += kv.first + ":" + kv.second.value + " ";
    }

    return ret;
}

bool isValid1(const Passport& passport)
{
    bool valid = true;

    for (auto& kv : passport.fields)
    {
        if (kv.second.optional)
            continue;

        valid &= !kv.second.value.empty();
    }

    return valid;
}

bool isValid2(const Passport& passport)
{
    bool valid = true;

    for (auto& kv : passport.fields)
    {
        if (kv.second.optional)
            continue;

        valid &= kv.second.valid;
    }

    return valid;
}

int calculate1()
{
    int count = 0;
    for (const auto& passport : passports)
    {
        if (isValid1(passport))
            ++count;
    }

    return count;
}

int calculate2()
{
    int count = 0;
    for (const auto& passport : passports)
    {
        if (isValid2(passport))
            ++count;
    }

    return count;
}

void init()
{
    parseData();

    processing_count = 2;
    
    thread thread1([]()
    {
        auto result = calculate1();
        OSync([=]()
        {
            result1 = result;
            processing_count--;
        });
    });
    thread1.detach();

    thread thread2([]()
    {
        auto result = calculate2();
        OSync([=]()
        {
            result2 = result;
            processing_count--;
        });
    });
    thread2.detach();
}

void renderUI()
{
    ImGui::Begin("Result");

    if (processing_count > 0)
    {
        const char spinner[] = {'/', '-', '\\', '|'};
        static int spinner_anim = 0;
        spinner_anim++;
        ImGui::LabelText("Processing", "%c", spinner[(spinner_anim / 4) % 4]);
    }

    ImGui::InputInt("Valid Passports 1", &result1);
    ImGui::InputInt("Valid Passports 2", &result2);

    if (ImGui::CollapsingHeader("All Passports"))
    {
        for (const auto& passport : passports)
        {
            ImGui::TextColored({1.f, isValid1(passport) ? 1.f : 0.f, isValid2(passport) ? 1.f : 0.f, 1.f}, "%s", to_string(passport).c_str());
        }
    }

    if (ImGui::CollapsingHeader("Valid Passports"))
    {
        ImGui::Columns(8);
        for (const auto& passport : passports)
        {
            if (isValid2(passport))
            {
                for (const auto& kv : passport.fields)
                {
                    ImGui::Text("%s:%s", kv.first.c_str(), kv.second.value.c_str());
                    ImGui::NextColumn();
                }
            }
        }
    }

    ImGui::End();
}
