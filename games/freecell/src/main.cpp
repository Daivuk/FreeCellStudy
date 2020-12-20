#include <imgui/imgui.h>
#include <onut/Input.h>
#include <onut/onut.h>
#include <onut/Renderer.h>
#include <onut/Settings.h>
#include <onut/SpriteBatch.h>
#include <onut/Timing.h>
#include "common.h"
#include "board.h"
#include "globals.h"
#include "resources.h"

void initSettings()
{
    oSettings->setGameName("OFreeCell");
    oSettings->setIsResizableWindow(false);
    oSettings->setResolution({1024, 768});
    oSettings->setShowFPS(false);
}

void init()
{
    oSettings->setUserSettingDefault("skin", "0");
    oSettings->setUserSettingDefault("best_turn_count", "999");
    oSettings->setUserSettingDefault("best_time", "3599"); // 59:59, 1h

    g_resources = make_shared<Resources>();
    g_board = make_shared<Board>();

    g_resources->changeSkin(g_resources->skins[stoi(oSettings->getUserSetting("skin"))]);
}

void shutdown()
{
}

void update()
{
    if (OInputPressed(OKeyLeftControl) && OInputJustPressed(OKeyN))
        g_board = make_shared<Board>();

    if (OInputPressed(OKeyLeftControl) && OInputJustPressed(OKeyZ))
        g_board->undo();

    g_board->update(ODT);
}

void render()
{
    // Clear
    oRenderer->clear(CLEAR_COLOR);

    // Draw board
    oSpriteBatch->begin(Matrix::CreateScale((float)PIXEL_SIZE, (float)PIXEL_SIZE, 1));
    oRenderer->renderStates.sampleFiltering = OFilterNearest;
    g_board->draw();
    oSpriteBatch->end();
}

void postRender()
{
}

void renderUI()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Game"))
        {
            if (ImGui::MenuItem("New", "Ctrl + N"))
                g_board = make_shared<Board>();

            if (ImGui::MenuItem("Restart"))
                g_board = make_shared<Board>(g_board->seed);

            ImGui::Separator();

            if (ImGui::MenuItem("Undo", "Ctrl + Z"))
                g_board->undo();

            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Alt + F4"))
                OQuit();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Skin"))
        {
            for (int i = 0; i < (int)g_resources->skins.size(); ++i)
            {
                auto selected = g_resources->skin == g_resources->skins[i];
                if (ImGui::MenuItem(("Skin " + to_string(i + 1)).c_str(), 0, &selected))
                {
                    g_resources->changeSkin(g_resources->skins[i]);
                    oSettings->setUserSetting("skin", to_string(i));
                }
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}
