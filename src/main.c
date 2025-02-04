/****************************************
 *
 * Copyright (c) 2025 Fahim Faisal
 *
 ****************************************/

#include <stdio.h>
#include "raylib.h"
#include "input_media.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: ./player [path to media file]\n");
        return 1;
    }

    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 600;

    int error_media = FALSE;
    int frame_available = TRUE;
    int texture_loaded = FALSE;

    if (!get_input_media_handlers(argv[1], screenWidth, screenHeight)) {
        fprintf(stderr, "Could not open input file %s\n", argv[1]);
        return 1;
    }

    InitWindow(screenWidth, screenHeight, "Player");

    // NOTE: Textures MUST be loaded after Window initialization (OpenGL context is required)

    Image screen = {
        .data = data[0],
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8   
    };
    
    // Image converted to texture, uploaded to GPU memory (VRAM)
    Texture2D texture = LoadTextureFromImage(screen);
    texture_loaded = TRUE;

    SetTargetFPS(60);
    //---------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        
        // Don't ask for further frames if decoding error or EOF
        if (!error_media && !file_ended) {
            frame_available = decode_next_frame();

            if (!frame_available) {
                // If no frames without EOF, decoding error.
                if (!file_ended) {
                    error_media = TRUE;
                    close_input_media();
                } 

                // Regardless of decoding error or EOF, Unload Texture
                UnloadTexture(texture);
                texture_loaded = FALSE;
            }
        }

        // data[0] remains the same since it is an address to an allocated buffer
        // so this assignment is redundant:
        // screen.data = (void *) data[0];

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if (error_media)  {
            DrawText("Encountered an error decoding media file", 190, 200, 20, BLACK);
        } else if (frame_available) {
            UpdateTexture(texture, screen.data);
            DrawTexture(texture, screenWidth/2 - texture.width/2, screenHeight/2 - texture.height/2, WHITE);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    if (texture_loaded) UnloadTexture(texture);
    if (media_open) close_input_media();

    CloseWindow();                      
    //--------------------------------------------------------------------------------------

    return 0;
}
