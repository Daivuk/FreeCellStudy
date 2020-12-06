#include <imgui/imgui.h>
#include <onut/ContentManager.h>
#include <onut/Renderer.h>
#include <onut/Settings.h>
#include <onut/Strings.h>

void initSettings()
{
    oSettings->setGameName("Advent of Code 3");
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
#include <onut/Async.h>
#include <onut/Files.h>
#include <onut/Dispatcher.h>
#include <onut/Point.h>

using namespace std;
using namespace chrono_literals;

int processing_count = 0;
int result1 = -1;
uint64_t result2 = 0;
int result_slopes[5] = {0};

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
    return std::move(onut::splitString(data_string, '\n'));
}

#define SNOW 0
#define TREE 1

int level_w = 0;
int level_h = 0;

vector<vector<int>> level;

void parseData()
{
    auto lines = getDataLines();

    level_w = (int)lines[0].size();
    level_h = (int)lines.size();
    level.resize(level_h);

    int y = 0;
    for (const auto& line : lines)
    {
        level[y].resize(level_w);
        for (int x = 0; x < level_w; ++x)
        {
            switch (line[x])
            {
                case '.': level[y][x] = SNOW; break;
                case '#': level[y][x] = TREE; break;
            }
        }
        y++;
    }
}

int checkSlope(int right, int down)
{
    Point pos;

    int tree_encountered = 0;
    int x = 0;
    for (int y = 0; y < level_h; y += down)
    {
        if (level[y][x % level_w] == TREE)
            tree_encountered++;
        x += right;
    }

    return tree_encountered;
}

int calculate1()
{
    return checkSlope(3, 1);
}

uint64_t calculate2()
{
    result_slopes[0] = checkSlope(1, 1);
    result_slopes[1] = checkSlope(3, 1);
    result_slopes[2] = checkSlope(5, 1);
    result_slopes[3] = checkSlope(7, 1);
    result_slopes[4] = checkSlope(1, 2);

    return (uint64_t)result_slopes[0] * 
           (uint64_t)result_slopes[1] * 
           (uint64_t)result_slopes[2] * 
           (uint64_t)result_slopes[3] * 
           (uint64_t)result_slopes[4];
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
    stringstream ss;
    ss << result2;
    ImGui::InputText("Result 2", (char*)ss.str().c_str(), ss.str().size());
    ImGui::Indent();
    for (auto slope : result_slopes) ImGui::Text("%i", slope);
    ImGui::Unindent();

    ImGui::End();
}
