#include <imgui/imgui.h>
#include <onut/ContentManager.h>
#include <onut/Renderer.h>
#include <onut/Settings.h>
#include <onut/Strings.h>

void initSettings()
{
    oSettings->setGameName("Advent of Code 6");
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
#include <set>
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

struct Group
{
    vector<string> lines;
    set<char> question_set;
};

vector<Group> groups;

void parseData()
{
    auto lines = getDataLines();

    Group current_group;
    for (const auto& line : lines)
    {
        if (line.empty())
        {
            if (!current_group.lines.empty())
            {
                groups.push_back(current_group);
                current_group = {};
            }
        }
        else
        {
            current_group.lines.push_back(line);
        }
    }
    if (!current_group.lines.empty())
        groups.push_back(current_group);

    for (auto& group : groups)
        for (const auto& line : group.lines)
            for (auto c : line)
                group.question_set.insert(c);
}

bool allAnswered(const Group& group, char question)
{
    for (const auto& line : group.lines)
    {
        bool found = false;
        for (auto c : line)
        {
            if (c == question)
            {
                found = true;
                break;
            }
        }
        
        if (!found)
            return false;
    }

    return true;
}

int calculate1()
{
    int sum = 0;

    for (const auto& group : groups)
        sum += (int)group.question_set.size();

    return sum;
}

int calculate2()
{
    int sum = 0;

    for (const auto& group : groups)
        for (auto c : group.question_set)
            if (allAnswered(group, c))
                sum++;

    return sum;
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

    ImGui::InputInt("Result 1", &result1);
    ImGui::InputInt("Result 2", &result2);

    ImGui::End();
}
