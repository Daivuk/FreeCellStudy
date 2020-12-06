#include <imgui/imgui.h>
#include <onut/ContentManager.h>
#include <onut/Renderer.h>
#include <onut/Settings.h>
#include <onut/Strings.h>

void initSettings()
{
    oSettings->setGameName("Advent of Code 5");
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

#define ROW_COUNT 128
#define COL_COUNT 8

struct Ticket
{
    int row = -1;
    int col = -1;
    int id = -1;
};

vector<Ticket> tickets;

bool seats[ROW_COUNT * COL_COUNT] = {false};

void parseData()
{
    auto lines = getDataLines();

    for (const auto& line : lines)
    {
        Ticket ticket;

        int row_min = 0;
        int row_max = ROW_COUNT;
        int col_min = 0;
        int col_max = COL_COUNT;

        for (auto c : line)
        {
            if (c == 'F')
                row_max = row_min + (row_max - row_min) / 2;
            else if (c == 'B')
                row_min = row_min + (row_max - row_min) / 2;
            else if (c == 'L')
                col_max = col_min + (col_max - col_min) / 2;
            else if (c == 'R')
                col_min = col_min + (col_max - col_min) / 2;
        }

        ticket.row = row_min;
        ticket.col = col_min;
        ticket.id = ticket.row * COL_COUNT + ticket.col;

        seats[ticket.id] = true;

        tickets.push_back(ticket);
    }
}

int calculate1()
{
    int highest = -1;

    for (const auto& ticket : tickets)
        highest = std::max(highest, ticket.id);

    return highest;
}

int calculate2()
{
    for (int i = 1; i < ROW_COUNT * COL_COUNT - 1; ++i)
    {
        if (seats[i - 1] == true &&
            seats[i] == false &&
            seats[i + 1] == true)
            return i;
    }

    return -1;
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

    ImGui::Columns(COL_COUNT);
    for (int row = 0; row < ROW_COUNT; ++row)
    {
        for (int col = 0; col < COL_COUNT; ++col)
        {
            int id = row * COL_COUNT + col;
            if (seats[id])
            {
                ImGui::TextColored({1, 0, 0, 1}, "%i", id);
            }
            else
            {
                ImGui::TextColored({1, 1, 1, 1}, "FREE");
            }
            ImGui::NextColumn();
        }
    }

    ImGui::End();
}
