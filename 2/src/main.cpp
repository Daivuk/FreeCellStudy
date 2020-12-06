#include <imgui/imgui.h>
#include <onut/Renderer.h>
#include <onut/Settings.h>

void initSettings()
{
    oSettings->setGameName("Advent of Code 2");
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
#include <onut/Async.h>
#include <onut/Dispatcher.h>

using namespace std;
using namespace chrono_literals;

#include "data.h"

int processing_count = 0;
int result1 = -1;
int result2 = -1;

int calculate1()
{
    int valid_count = 0;

    for (auto& password : puzzle_data)
    {
        int c_count = 0;
        for (auto c : password.password)
        {
            if (c == password.c)
                c_count++;
        }
        if (c_count >= password.min_count &&
            c_count <= password.max_count)
            valid_count++;
    }

    return valid_count;
}

int calculate2()
{
    int valid_count = 0;

    for (auto& password : puzzle_data)
    {
        int match_count = 0;

        if (password.password[password.min_count - 1] == password.c)
            match_count++;
        if (password.password[password.max_count - 1] == password.c)
            match_count++;

        if (match_count == 1)
            valid_count++;
    }

    return valid_count;
}

void init()
{
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
