// Copyright (c) 2018 David Athay
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>
#include <OpenGL/gl3.h>


#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

#include "jsmn.h"

#define WINDOW_WIDTH 1440
#define WINDOW_HEIGHT 900

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

typedef enum {
    MENU_STATE = 0,
    OFFICE_STATE,
    GAME_STATE,
    EXIT_STATE
} State;

typedef struct Param
{
    char *name;
    char *value;
} Param;

typedef struct Object
{
    Param **params;
    int numParams;
} Object;

struct Gui;

typedef struct Game
{
    State state;
    struct Gui *gui;

    Object **objects;
    int numObjects;
} Game;

#include "gui.h"
#include "menu.h"
#include "office.h"

#include "gui.c"
#include "menu.c"
#include "office.c"

int loadFile(Game *game, const char *filename);
void getNextToken(char *string, Param *param, jsmntok_t *token);

int main(void)
{
    /* Platform */
    SDL_Window *win;
    SDL_GLContext glContext;
    int win_width, win_height;
    int running = 1;

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_EVENTS);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    win = SDL_CreateWindow("RLCS Manager",
                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);
    glContext = SDL_GL_CreateContext(win);
    SDL_GetWindowSize(win, &win_width, &win_height);

    /* OpenGL setup */
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    /* GUI */
    Game game = {};
    Gui gui;
    initUI(&gui, win);

    Object *objects[1024] = {};
    game.gui = &gui;
    game.state = MENU_STATE;
    game.objects = objects;
    game.numObjects = 0;

    setBackgroundColour(&gui, 0.23f, 0.42f, 0.57f);

    /* Load files */
    loadFile(&game, "../players");

    while (running)
    {
        /* Input */
        SDL_Event evt;
        beginInput(&gui);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) goto cleanup;
            handleEvent(&gui, &evt);
        }

        endInput(&gui);

        /* GUI */
        switch (game.state)
        {
            case MENU_STATE:
                {
                    menuState(&game, win);
                } break;
            case OFFICE_STATE:
                {
                    officeState(&game, win);
                } break;
            case GAME_STATE:
                {
                    game.state = MENU_STATE;
                } break;
            case EXIT_STATE:
            {
                running = false;
            } break;
            default:
                break;
        }

        /* Draw */
        glViewport(0, 0, win_width, win_height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(gui.bg.r, gui.bg.g, gui.bg.b, gui.bg.a);
        render();
        SDL_GL_SwapWindow(win);
    }

    cleanup:
    kill();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

// Returns success
int loadFile(Game *game, const char *filename)
{
    FILE *file = fopen(filename, "r");
    int numTokens = 0;
    char buffer[1024] = {};

    // read the file
    fread(buffer, 1024, 1, file);

    // init jsmn
    jsmn_parser parser;
    jsmn_init(&parser);
    numTokens = jsmn_parse(&parser, buffer, strlen(buffer), NULL, 0);

    jsmntok_t *tokens = (jsmntok_t*)malloc(sizeof(jsmntok_t) * numTokens);

    jsmn_init(&parser);
    jsmn_parse(&parser, buffer, strlen(buffer), tokens, numTokens);

    printf("File: %s contains %d tokens\n", filename, numTokens);

    for (int i = 0; i < numTokens; ++i)
    {
        jsmntok_t tok = tokens[i];
        switch (tok.type)
        {
            case JSMN_OBJECT:
            {
                printf("Object found\n");
                Object *obj = (Object*)malloc(sizeof(Object));
                obj->numParams = tok.size;
                obj->params = (Param**)malloc(sizeof(Param*) * obj->numParams);
                game->objects[game->numObjects++] = obj;

                int index = i + 1; // start from token after the object

                for (int j = 0; j < tok.size; ++j)
                {
                    Param *param = (Param*)malloc(sizeof(Param));
                    param->name = 0;
                    param->value = 0;
                    getNextToken(buffer, param, &tokens[index]);
                    printf("Loaded parameter: %s: %s\n", param->name, param->value);
                    obj->params[j] = param;
                    index += 2; // processed name, value so skip those tokens now
                }
                // go passed this object
                i += tok.size + 1;
            } break;
            default:
                ;
        }
    }

    return 0; // NO ERRORS

    // create object -> assign string name -> assign string value

    // TODO:
    // create divisions
    // create teams
    // assign teams to divisions
    // create players
    // assign players to teams
}

void getNextToken(char *string, Param *param, jsmntok_t *token)
{
    switch (token->type)
    {
        case JSMN_OBJECT:
            return;
        case JSMN_ARRAY:
        {
            printf("Array found\n");
            if (token->size > 1)
            {
                getNextToken(string, param, token + 1);
            }
        } break;
        case JSMN_STRING:
        {
            unsigned int len = token->end - token->start;
            char *s = (char *) malloc(sizeof(char) * len + 1);
            strncpy(s, &string[token->start], len);
            s[len] = '\0';

            if (param->name == 0)
            {
                param->name = s;
                getNextToken(string, param, token + 1);
            }
            else if (param->value == 0)
            {
                param->value = s;
            }

            if (token->size > 1)
            {
                getNextToken(string, param, token + 1);
            }

        } break;
        default:
        {
            printf("Expected array or string, found %d\n", token->type);
        }
    }
}