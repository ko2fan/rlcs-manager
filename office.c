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

void officeState(Game *game, SDL_Window *win)
{
    int win_width;
    int win_height;
    int numPlayers = 6;
    SDL_GetWindowSize(win, &win_width, &win_height);

    float centreX = win_width / 2;
    float centreY = win_height / 2;

    Gui *gui = game->gui;

    if (nk_begin(gui->ctx, "Players", nk_rect(centreX - 500 / 2, centreY - 400, 500, 600),
                 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(gui->ctx, 35, 2);
        nk_label_colored(gui->ctx, "Player Name", NK_TEXT_CENTERED, nk_rgb(255, 255, 255));
        nk_label_colored(gui->ctx, "Team",  NK_TEXT_CENTERED, nk_rgb(255, 255, 255));
        for (int i = 1; i < game->numObjects; ++i)
        {
            Object *obj = game->objects[i];
            for (int j = 0; j < obj->numParams; ++j)
            {
                if (strncmp(obj->params[j]->name, "name", 4) == 0)
                {
                    nk_label_wrap(gui->ctx, obj->params[j]->value);
                }
                else if (strncmp(obj->params[j]->name, "team", 4) == 0)
                {
                    nk_label_wrap(gui->ctx, obj->params[j]->value);
                }
            }
        }

        nk_layout_row_dynamic(gui->ctx, 50, 1);
        if (nk_button_label(gui->ctx, "Back"))
            game->state = MENU_STATE;
    }
    nk_end(gui->ctx);
}