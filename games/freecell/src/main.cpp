#include <onut/Input.h>
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
    oSettings->setGameName("FreeCell");
    oSettings->setIsResizableWindow(false);
    oSettings->setResolution({1024, 768});
    oSettings->setShowFPS(false);
}

void init()
{
    g_resources = make_shared<Resources>();
    g_board = make_shared<Board>();
}

void shutdown()
{
}

void update()
{
    if (OInputPressed(OKeyLeftControl) && OInputJustPressed(OKeyN)) // New game
        g_board = make_shared<Board>();

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
}
