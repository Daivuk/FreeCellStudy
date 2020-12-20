#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Defines
#define WIDTH 1024
#define HEIGHT 768

#define CARD_WIDTH 100
#define CARD_HEIGHT 145

#define FREE_HOME_H_SPACING 12
#define FREE_CELL_X 20
#define HOME_CELL_X (WIDTH - FREE_CELL_X - CARD_WIDTH * 4 - FREE_HOME_H_SPACING * 3)
#define FREE_HOME_Y 20
#define TABLEAU_X 20
#define TABLEAU_Y (FREE_HOME_Y + CARD_HEIGHT + 30)
#define TABLEAU_H_SPACING ((WIDTH - TABLEAU_X * 2 - CARD_WIDTH * 8) / 7)
#define COLUMN_V_OFFSET 28

#define MAX_CARD_IN_COLUMN (7 + 12)

#define CARD_NONE -1
#define CARD_VALUE_ACE 0
#define CARD_VALUE_KING 12
#define TEXTURE_CARDS 0
#define TEXTURE_CARD_BACK (TEXTURE_CARDS + 52)
#define TEXTURE_FREE_CELL (TEXTURE_CARD_BACK + 1)
#define TEXTURE_HOME_CELLS (TEXTURE_FREE_CELL + 1)
#define TEXTURE_COUNT (TEXTURE_HOME_CELLS + 4)

// Types
typedef struct
{
    int x, y;
} Point;

typedef struct
{
    float x, y;
} Vector;

typedef struct
{
    int id;
    Point position;
    Point target_position;
    Vector positionf;
    int draw_order;
    int target_draw_order;
} Card;

typedef struct
{
    int count;
    Card *cards[MAX_CARD_IN_COLUMN];
} Column;

typedef struct
{
    unsigned int seed;
    Card deck[52];
    Card *free_cells[4];
    Card *home_cells[4];
    Column tableau[8];
} Board;

typedef enum
{
    ORIGIN_NONE,
    ORIGIN_FREE_CELL,
    ORIGIN_HOME_CELL,
    ORIGIN_TABLEAU
} Origin;

typedef struct
{
    Card *card;
    Origin origin;
    int free_cell_index;
    int home_cell_index;
    int tableau_index;
    int column_index;
} Source;

typedef struct
{
    Point mouse_pos_on_down;
    Point card_pos_on_down;
    int draw_order_on_down;
    SDL_bool started_dragging;
    Point drag_offset;
    Source source;
} Drag;

// Variables
SDL_bool is_application_running = SDL_TRUE;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
Board board;
Drag drag = { 0 };
SDL_Texture *textures[TEXTURE_COUNT];

// Prototypes
void init();
void initSDL();
void loadTextures();
SDL_Texture *loadTexture(const char *filename);
void shutdown();

void mainLoop();
void pollEvents();
void update(float delta_time);
void render();
void clearScreen();
void drawBoard(Board *board);
void drawFreeCell(int index);
void drawHomeCell(int suit);
void drawTableau(int index);
void drawCard(int id, Point position);

void onMouseDown(Point position);
void onMouseMove(Point position);
void onMouseUp(Point position);
Source getSourceAt(Point position);
Source getDestAt(Point position);
void removeSource(Source source);
int getSourceCardCount(Source source);
void autoMoveCard(Source source);
SDL_bool canHomeCellAcceptSource(Source source, int suit);
SDL_bool canTableauAcceptSource(Source source, int tableau_index);
SDL_bool canFreeCellAcceptSource(Source source, int free_cell_index);
void moveCardToHomeCell(Source source, int suit);
void moveCardToTableau(Source source, int tableau_index);
void moveCardToFreeCell(Source source, int free_cell_index);

Point getFreeCellPosition(int index);
Point getHomeCellPosition(int index);
Point getTableauPosition(int index);
Point getPositionInColumn(Point column_position, int card_index);
SDL_bool cardContainsPoint(Card *card, Point point);
SDL_bool locationContainsPoint(Point position, Point point);

void newGame(unsigned int seed);
Card *getTableauTopCard(int tableau_index);
Card *getColumnTopCard(Column *column);
int getCardValue(Card *card);
int getCardSuit(Card *card);
int getCardColor(Card *card);

int main(int argc, char *argv[])
{
    init();
    mainLoop();
    shutdown();

    return 0;
}

void init()
{
    initSDL();
    loadTextures();
    newGame((unsigned int)time(0));
}

void initSDL()
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("CFreeCell", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
}

void loadTextures()
{
    for (int i = 0; i < 58; i++)
    {
        char filename[120];
        sprintf(filename, "assets/cards/%d.png", i);
        textures[i] = loadTexture(filename);
    }
}

SDL_Texture *loadTexture(const char *filename)
{
    // Load image
    int w, h, comp;
    stbi_uc *image_data = stbi_load(filename, &w, &h, &comp, 4);

    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(image_data, w, h, comp * 8, w * comp, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FreeSurface(surface);
    stbi_image_free(image_data);

    return texture;
}

void shutdown()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void mainLoop()
{
    while (is_application_running)
    {
        pollEvents();
        update(1.f / 60.f);
        render();
    }
}

void pollEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
            case SDL_QUIT:
                is_application_running = SDL_FALSE;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT)
                    onMouseDown((Point){ e.button.x, e.button.y });
                break;
            case SDL_MOUSEMOTION:
                onMouseMove((Point){ e.motion.x, e.motion.y });
                break;
            case SDL_MOUSEBUTTONUP:
                if (e.button.button == SDL_BUTTON_LEFT)
                    onMouseUp((Point){ e.button.x, e.button.y });
                break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_n && e.key.keysym.mod & KMOD_LCTRL)
                    newGame((unsigned int)time(0));

                if (e.key.keysym.sym == SDLK_F5)
                    newGame(board.seed);

                break;
        }

        if (!is_application_running)
            break;
    }
}

void update(float delta_time)
{
    // Update card animations
    float speed = delta_time * 5.f;
    float tension = 3.5f;

    for (int i = 0; i < 52; i++)
    {
        Card *card = &board.deck[i];

        Vector targetf = { (float)card->target_position.x, (float)card->target_position.y };
        Vector dir = { 
            targetf.x - card->positionf.x,
            targetf.y - card->positionf.y
        };

        float distance = dir.x * dir.x + dir.y * dir.y;
        if (distance > 0.f)
        {
            dir.x /= distance;
            dir.y /= distance;

            float forward = distance * tension;
            if (forward < 1.f)
                forward = 1.f;
            forward *= speed;

            distance -= forward;
            if (distance < 0.f)
                distance = 0.f;

            card->positionf.x = targetf.x - dir.x * distance;
            card->positionf.y = targetf.y - dir.y * distance;
        }

        if (distance == 0.f)
            card->draw_order = card->target_draw_order;
        else
            card->draw_order = card->target_draw_order + 1000;

        card->position.x = (int)card->positionf.x;
        card->position.y = (int)card->positionf.y;
    }
}

void render()
{
    clearScreen();
    drawBoard(&board);
    SDL_RenderPresent(renderer);
}

void clearScreen()
{
    SDL_SetRenderDrawColor(renderer, 53, 101, 77, 255);
    SDL_RenderClear(renderer);
}

void drawBoard(Board *board)
{
    // Free Cells
    for (int i = 0; i < 4; i++)
        drawFreeCell(i);

    // Home Cells
    for (int i = 0; i < 4; i++)
        drawHomeCell(i);

    // Tableau
    for (int i = 0; i < 8; i++)
        drawTableau(i);

    // Ordered cards by draw order
    Card *ordered_cards[52];
    for (int i = 0; i < 52; i++)
        ordered_cards[i] = &board->deck[i];

    for (int i = 0; i < 51; i++)
    {
        for (int j = 0; j < 51; j++)
        {
            Card *a = ordered_cards[j];
            Card *b = ordered_cards[j + 1];

            if (b->draw_order < a->draw_order)
            {
                ordered_cards[j] = b;
                ordered_cards[j + 1] = a;
            }
        }
    }

    // Draw ordered cards
    for (int i = 0; i < 52; i++)
    {
        Card *card = ordered_cards[i];
        drawCard(card->id, card->position);
    }
}

Point getFreeCellPosition(int index)
{
    return (Point){
        FREE_CELL_X + (CARD_WIDTH + FREE_HOME_H_SPACING) * index,
        FREE_HOME_Y
    };
}

Point getHomeCellPosition(int index)
{
    return (Point){
        HOME_CELL_X + (CARD_WIDTH + FREE_HOME_H_SPACING) * index,
        FREE_HOME_Y
    };
}

Point getTableauPosition(int index)
{
    return (Point){
        TABLEAU_X + (CARD_WIDTH + TABLEAU_H_SPACING) * index,
        TABLEAU_Y
    };
}

Point getPositionInColumn(Point column_position, int card_index)
{
    return (Point){
        column_position.x,
        column_position.y + COLUMN_V_OFFSET * card_index
    };
}

void drawFreeCell(int index)
{
    drawCard(TEXTURE_FREE_CELL, getFreeCellPosition(index));
}

void drawHomeCell(int suit)
{
    drawCard(TEXTURE_HOME_CELLS + suit, getHomeCellPosition(suit));
}

void drawTableau(int index)
{
    drawCard(TEXTURE_FREE_CELL, getTableauPosition(index));
}

void drawCard(int id, Point position)
{
    SDL_Rect src_rect = { position.x, position.y, CARD_WIDTH, CARD_HEIGHT };
    SDL_RenderCopy(renderer, textures[id], NULL, &src_rect);
}

void onMouseDown(Point position)
{
    // Get clicked card
    drag.source = getSourceAt(position);
    if (drag.source.origin == ORIGIN_NONE)
        return;

    drag.drag_offset.x = drag.source.card->position.x - position.x;
    drag.drag_offset.y = drag.source.card->position.y - position.y;

    drag.mouse_pos_on_down = position;
    drag.card_pos_on_down = drag.source.card->position;
    drag.draw_order_on_down = drag.source.card->draw_order;
    drag.started_dragging = SDL_FALSE;

    drag.source.card->draw_order += 1000;
    drag.source.card->target_draw_order += 1000;

    // If from tableau, set the draw order for the rest of the stack
    if (drag.source.origin == ORIGIN_TABLEAU)
    {
        Column *col = &board.tableau[drag.source.tableau_index];

        for (int i = drag.source.column_index + 1; i < col->count; i++)
        {
            Card *card = col->cards[i];
            card->draw_order += 1000;
            card->target_draw_order += 1000;
        }
    }
}

void onMouseMove(Point position)
{
    if (drag.source.origin == ORIGIN_NONE)
        return;

    Point drag_diff = {
        position.x - drag.mouse_pos_on_down.x,
        position.y - drag.mouse_pos_on_down.y
    };

    if (abs(drag_diff.x) > 3 || abs(drag_diff.y) > 3)
        drag.started_dragging = SDL_TRUE;

    if (drag.started_dragging)
    {
        drag.source.card->target_position.x = position.x + drag.drag_offset.x;
        drag.source.card->target_position.y = position.y + drag.drag_offset.y;
        drag.source.card->positionf.x = (float)drag.source.card->target_position.x;
        drag.source.card->positionf.y = (float)drag.source.card->target_position.y;

        // If from tableau, move the rest of the stack along
        if (drag.source.origin == ORIGIN_TABLEAU)
        {
            Column *col = &board.tableau[drag.source.tableau_index];
            Point position = drag.source.card->target_position;

            for (int i = drag.source.column_index + 1; i < col->count; i++)
            {
                position.y += COLUMN_V_OFFSET;

                Card *card = col->cards[i];
                card->target_position = position;
                card->positionf.x = (float)position.x;
                card->positionf.y = (float)position.y;
            }
        }
    }
}

void resetDragDrawOrder()
{
    drag.source.card->target_draw_order = drag.draw_order_on_down;

    // If from tableau, move the rest of the stack along
    if (drag.source.origin == ORIGIN_TABLEAU)
    {
        Column *col = &board.tableau[drag.source.tableau_index];
        for (int i = drag.source.column_index + 1; i < col->count; i++)
        {
            Card *card = col->cards[i];
            card->target_draw_order = i;
        }
    }
}

void resetDragPosition()
{
    drag.source.card->target_position = drag.card_pos_on_down;

    // If from tableau, move the rest of the stack along
    if (drag.source.origin == ORIGIN_TABLEAU)
    {
        Column *col = &board.tableau[drag.source.tableau_index];
        Point position = drag.source.card->target_position;

        for (int i = drag.source.column_index + 1; i < col->count; i++)
        {
            position.y += COLUMN_V_OFFSET;

            Card *card = col->cards[i];
            card->target_position = position;
        }
    }
}

void onMouseUp(Point position)
{
    if (drag.source.origin == ORIGIN_NONE)
        return;

    resetDragDrawOrder();

    if (drag.started_dragging)
    {
        // Check if the mouse over card accepts our card
        Source dest = getDestAt(position);

        switch (dest.origin)
        {
            case ORIGIN_FREE_CELL:
                if (canFreeCellAcceptSource(drag.source, dest.free_cell_index))
                    moveCardToFreeCell(drag.source, dest.free_cell_index);
                else
                    resetDragPosition();
                break;

            case ORIGIN_HOME_CELL:
                if (canHomeCellAcceptSource(drag.source, dest.home_cell_index))
                    moveCardToHomeCell(drag.source, dest.home_cell_index);
                else
                    resetDragPosition();
                break;

            case ORIGIN_TABLEAU:
                if (canTableauAcceptSource(drag.source, dest.tableau_index))
                    moveCardToTableau(drag.source, dest.tableau_index);
                else
                    resetDragPosition();
                break;

            case ORIGIN_NONE:
            default:
                resetDragPosition();
                break;
        }
    }
    else
    {
        autoMoveCard(drag.source);
    }

    drag.source.origin = ORIGIN_NONE;
}

void autoMoveCard(Source source)
{
    // Home cells priority
    for (int i = 0; i < 4; i++)
    {
        if (canHomeCellAcceptSource(source, i))
        {
            moveCardToHomeCell(source, i);
            return;
        }
    }

    // Check if can move somewhere in the tableau
    for (int i = 0; i < 8; i++)
    {
        if (canTableauAcceptSource(source, i))
        {
            moveCardToTableau(source, i);
            return;
        }
    }

    // Finally, if single card, move to Free Cells
    for (int i = 0; i < 4; i++)
    {
        if (canFreeCellAcceptSource(source, i))
        {
            moveCardToFreeCell(source, i);
            return;
        }
    }
}

SDL_bool canHomeCellAcceptSource(Source source, int suit)
{
    // Needs to be same suit
    if (getCardSuit(source.card) != suit)
        return SDL_FALSE;

    // If the source has more than 1 card (From the tableau, not last card of column),
    // then we cannot accept it
    if (getSourceCardCount(source) > 1)
        return SDL_FALSE;

    Card *home_cell_card = board.home_cells[suit];
    int source_value = getCardValue(source.card);

    // If home cell empty and source is ACE, we're good
    if (home_cell_card == NULL)
    {
        if (source_value == CARD_VALUE_ACE)
            return SDL_TRUE;
        else
            return SDL_FALSE; // Otherwise, we don't accept it
    }

    int home_value = getCardValue(home_cell_card);

    // If source value is one more than home value, put it there.
    if (source_value == home_value + 1)
        return SDL_TRUE;

    return SDL_FALSE;
}

SDL_bool canTableauAcceptSource(Source source, int tableau_index)
{
    // Avoid move to same column
    if (source.origin == ORIGIN_TABLEAU &&
        tableau_index == source.tableau_index)
        return SDL_FALSE;

    Card *top_card = getTableauTopCard(tableau_index);

    int source_color = getCardColor(source.card);
    int source_value = getCardValue(source.card);

    // If source is king and the column is empty, that's a valid move
    if (source_value == CARD_VALUE_KING && top_card == NULL)
        return SDL_TRUE;

    int top_color = getCardColor(top_card);
    int top_value = getCardValue(top_card);

    // If same color, invalid move
    if (source_color == top_color)
        return SDL_FALSE;

    // Valid if the source value is stack - 1
    if (source_value == top_value - 1)
        return SDL_TRUE;

    return SDL_FALSE;
}

SDL_bool canFreeCellAcceptSource(Source source, int free_cell_index)
{
    // Avoid move to same free cell
    if (source.origin == ORIGIN_FREE_CELL &&
        free_cell_index == source.free_cell_index)
        return SDL_FALSE;

    // If the source has more than 1 card (From the tableau, not last card of column),
    // then we cannot accept it
    if (getSourceCardCount(source) > 1)
        return SDL_FALSE;

    if (board.free_cells[free_cell_index] == NULL)
        return SDL_TRUE;

    return SDL_FALSE;
}

void moveCardToHomeCell(Source source, int suit)
{
    source.card->target_position = getHomeCellPosition(suit);
    source.card->target_draw_order = getCardValue(source.card);

    board.home_cells[suit] = source.card;

    removeSource(source);
}

void moveCardToTableau(Source source, int tableau_index)
{
    Column *dest_column = &board.tableau[tableau_index];
    Point dest_column_position = getTableauPosition(tableau_index);

    // If source is from tableau, we move a stack
    if (source.origin == ORIGIN_TABLEAU)
    {
        Column *source_column = &board.tableau[source.tableau_index];

        for (int i = source.column_index; i < source_column->count; i++)
        {
            Card *src_card = source_column->cards[i];

            src_card->target_position = getPositionInColumn(dest_column_position, dest_column->count);
            src_card->target_draw_order = dest_column->count;

            dest_column->cards[dest_column->count++] = src_card;
        }
    }
    else
    {
        source.card->target_position = getPositionInColumn(dest_column_position, dest_column->count);
        source.card->target_draw_order = dest_column->count;

        dest_column->cards[dest_column->count++] = source.card;
    }

    removeSource(source);
}

void moveCardToFreeCell(Source source, int free_cell_index)
{
    source.card->target_position = getFreeCellPosition(free_cell_index);
    source.card->target_draw_order = 0;

    board.free_cells[free_cell_index] = source.card;

    removeSource(source);
}

Source getSourceAt(Point position)
{
    Source source;

    // Free Cells
    for (int i = 0; i < 4; i++)
    {
        Card *card = board.free_cells[i];
        if (card == NULL)
            continue;

        if (cardContainsPoint(card, position))
        {
            source.card = card;
            source.origin = ORIGIN_FREE_CELL;
            source.free_cell_index = i;

            return source;
        }
    }

    // Tableau
    for (int i = 0; i < 8; i++)
    {
        Column *column = &board.tableau[i];
        Point tableau_position = getTableauPosition(i);
        Card *prev_card = NULL;

        for (int j = column->count - 1; j >= 0; j--)
        {
            Card *card = column->cards[j];

            // Make sure this is a valid stack
            if (prev_card)
                if (getCardValue(card) != getCardValue(prev_card) + 1 ||
                    getCardColor(card) == getCardColor(prev_card))
                    break;

            if (cardContainsPoint(card, position))
            {
                source.card = card;
                source.origin = ORIGIN_TABLEAU;
                source.tableau_index = i;
                source.column_index = j;

                return source;
            }

            prev_card = card;
        }
    }

    source.origin = ORIGIN_NONE;
    source.card = NULL;
    return source;
}

Source getDestAt(Point position)
{
    Source source;

    // Free Cells
    for (int i = 0; i < 4; i++)
    {
        if (locationContainsPoint(getFreeCellPosition(i), position))
        {
            source.origin = ORIGIN_FREE_CELL;
            source.free_cell_index = i;

            return source;
        }
    }

    // Home Cells
    for (int i = 0; i < 4; i++)
    {
        if (locationContainsPoint(getHomeCellPosition(i), position))
        {
            source.origin = ORIGIN_HOME_CELL;
            source.home_cell_index = i;

            return source;
        }
    }

    // Tableau
    for (int i = 0; i < 8; i++)
    {
        Column *col = &board.tableau[i];
        Point tableau_position = getTableauPosition(i);

        tableau_position.y += COLUMN_V_OFFSET * col->count;
        if (locationContainsPoint(tableau_position, position))
        {
            source.origin = ORIGIN_TABLEAU;
            source.tableau_index = i;
            source.column_index = col->count;

            return source;
        }
    }

    source.origin = ORIGIN_NONE;
    source.card = NULL;
    return source;
}

void removeSource(Source source)
{
    switch (source.origin)
    {
        case ORIGIN_FREE_CELL:
            board.free_cells[source.free_cell_index] = NULL;
            break;
        case ORIGIN_TABLEAU:
            board.tableau[source.tableau_index].count = source.column_index;
            break;
    }
}

int getSourceCardCount(Source source)
{
    switch (source.origin)
    {
        case ORIGIN_FREE_CELL:
            return 1;
        case ORIGIN_TABLEAU:
            return board.tableau[source.tableau_index].count - source.column_index;
        default:
        case ORIGIN_NONE:
            return 0;
    }
}

SDL_bool cardContainsPoint(Card *card, Point point)
{
    return locationContainsPoint(card->position, point);
}

SDL_bool locationContainsPoint(Point position, Point point)
{
    return (point.x >= position.x &&
            point.y >= position.y &&
            point.x < position.x + CARD_WIDTH &&
            point.y < position.y + CARD_HEIGHT) ? SDL_TRUE : SDL_FALSE;
}

void newGame(unsigned int seed)
{
    memset(&board, 0, sizeof(board));

    for (int i = 0; i < 4; i++)
        board.free_cells[i] = NULL;

    for (int i = 0; i < 4; i++)
        board.home_cells[i] = NULL;

    // Init deck
    for (int i = 0; i < 52; i++)
        board.deck[i].id = i;

    // Randomize seed using current time
    board.seed = seed;
    srand(board.seed);

    // Shuffle
    Card *shuffled_deck[52];
    for (int i = 0; i < 52; i++)
        shuffled_deck[i] = &board.deck[i];

    for (int i = 52; i > 0; i--)
    {
        int random_index = rand() % i;

        Card *tmp = shuffled_deck[random_index];
        shuffled_deck[random_index] = shuffled_deck[i - 1];
        shuffled_deck[i - 1] = tmp;
    }

    // Distribute shuffled deck into the tableau
    for (int i = 0; i < 52; i++)
    {
        Column *col = &board.tableau[i % 8];
        Card *card = shuffled_deck[i];
        Point column_position = getTableauPosition(i % 8);
        card->target_position = getPositionInColumn(column_position, col->count);
        card->target_draw_order = col->count;
        col->cards[col->count++] = card;
    }
}

Card *getTableauTopCard(int tableau_index)
{
    Column *column = &board.tableau[tableau_index];
    return getColumnTopCard(column);
}

Card *getColumnTopCard(Column *column)
{
    Card *top_card = NULL;
    if (column->count)
        top_card = column->cards[column->count - 1];

    return top_card;
}

int getCardValue(Card *card)
{
    return card->id % 13;
}

int getCardSuit(Card *card)
{
    return card->id / 13;
}

int getCardColor(Card *card)
{
    return card->id / 26;
}
