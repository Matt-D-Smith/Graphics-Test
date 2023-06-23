/*******************************************************************************************
*
*   raylib [core] example - Basic 3d example
*
*   Welcome to raylib!
*
*   To compile example, just press F5.
*   Note that compiled executable is placed in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   This example has been created using raylib 1.0 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2013-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

#define MAP_SIZE 1024.0f

bool isOnMesh(Vector3 position, float height, Mesh * mesh, Matrix transform) {
    Ray ray = (Ray){position, (Vector3){0, -1, 0}};
    RayCollision rayCollision = GetRayCollisionMesh(ray, *mesh, transform);
    if (((rayCollision.distance - height) <= EPSILON) && ((rayCollision.distance - height) >= -EPSILON)) return true;
    // if (((rayCollision.distance - height) <= EPSILON) && rayCollision.hit) return true;
    return false;
}

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
Camera camera = { 0 };
Vector3 cubePosition = { 0 };

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    const float playerHeight = 4.0f;

    InitWindow(screenWidth, screenHeight, "bad game made by a bad gamer");

    Shader shader = LoadShader(0, "shaders/first.fs");
    Shader shaderblur = LoadShader(0, "shaders/blur.fs");
    Shader defaultShader = LoadShader(0, 0);

    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, playerHeight };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    float CAMERA_MOVE_SPEED = 0.1f;         // Camera movement speed
    float CAMERA_MOUSE_MOVE_SENSITIVITY = 0.06f;

    int cameraMode = CAMERA_FIRST_PERSON;

    char filename[] = "C:/Users/Matt/Desktop/Hardware-Stuff/Noise Textures/heightmapfull.png";
    Image heightMapImage = LoadImage(filename);
    Texture2D heightMapTexture = LoadTextureFromImage(heightMapImage);
    Mesh heightMapMesh = GenMeshHeightmap(heightMapImage, (Vector3){MAP_SIZE, MAP_SIZE/8, MAP_SIZE});
    Model heightMap = LoadModelFromMesh(heightMapMesh);
    heightMap.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = heightMapTexture;

    char filenameLow[] = "C:/Users/Matt/Desktop/Hardware-Stuff/Noise Textures/heightmap256.png";
    Image heightMapImageLow = LoadImage(filenameLow);
    Texture2D heightMapTextureLow = LoadTextureFromImage(heightMapImageLow);
    Mesh heightMapMeshLow = GenMeshHeightmap(heightMapImageLow, (Vector3){MAP_SIZE, MAP_SIZE/8, MAP_SIZE});
    Model heightMapLow = LoadModelFromMesh(heightMapMeshLow);
    heightMapLow.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = heightMapTextureLow;

    Vector3 heightMapPos = (Vector3){-MAP_SIZE/2, 0, -MAP_SIZE/2};
    Matrix heightMapTranslation = MatrixTranslate(heightMapPos.x, heightMapPos.y, heightMapPos.z);
    Matrix heightMapTransform = MatrixMultiply(heightMapLow.transform, heightMapTranslation);

    // float vertices[] = {
    //     -0.5f, -0.5f, 0.0f,
    //     0.5f, -0.5f, 0.0f,
    //     0.0f,  0.5f, 0.0f
    // };

    DisableCursor();                    // Limit cursor to relative movement inside the window

    // Create a RenderTexture2D to be used for render to texture
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);

    //--------------------------------------------------------------------------------------
    SetTargetFPS(144);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

        if (IsKeyPressed(KEY_ONE))
        {
            cameraMode = CAMERA_FIRST_PERSON;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_TWO))
        {
            cameraMode = CAMERA_FREE;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        //----------------------------------------------------------------------------------
        // Update Camera
        //----------------------------------------------------------------------------------
        
        // Sprint
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            CAMERA_MOVE_SPEED = 0.2f;
        } 
        else {
            CAMERA_MOVE_SPEED = 0.1f;
        }

        // Directional Movement
        Vector2 mousePositionDelta = GetMouseDelta();
        Vector3 moveVec = (Vector3){IsKeyDown(KEY_W) - IsKeyDown(KEY_S),
                                    IsKeyDown(KEY_D) - IsKeyDown(KEY_A),
                                    0.0};
        moveVec = Vector3Scale(Vector3Normalize(moveVec), CAMERA_MOVE_SPEED);
        moveVec.z = (cameraMode == CAMERA_FREE) * (IsKeyDown(KEY_SPACE) - IsKeyDown(KEY_LEFT_ALT)) * CAMERA_MOVE_SPEED;

        // Check collision with ground
        if (!isOnMesh(camera.position, playerHeight, &heightMapMeshLow, heightMapTransform)) {
            // Move a player to their height above the mesh
            Ray ray = (Ray){Vector3Add(camera.position, (Vector3){0,1000000,0}), (Vector3){0.0, -1.0, 0.0}};
            RayCollision rayCollision = GetRayCollisionMesh(ray, heightMapMeshLow, heightMapTransform);
            camera.position.y = rayCollision.point.y + playerHeight;
            camera.target.y += 1000000 - rayCollision.distance + playerHeight;
        }

        UpdateCameraPro(&camera,
                        moveVec,
                        (Vector3){  mousePositionDelta.x * CAMERA_MOUSE_MOVE_SENSITIVITY,
                                    mousePositionDelta.y * CAMERA_MOUSE_MOVE_SENSITIVITY,
                                    0.0},
                        0.0);

        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginTextureMode(target);       // Enable drawing to texture

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawModel(heightMap, heightMapPos, 1.0f, WHITE);
                // DrawMesh(heightMapMeshLow, LoadMaterialDefault(), heightMapTransform);
                DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
                DrawGrid(10, 1.0f);

            EndMode3D();

            // BeginShaderMode(shaderblur);
            //     DrawTextureRec(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height }, (Vector2){ 0, 0 }, WHITE); // render texture with shader applied
            // EndShaderMode();
            
            
        EndTextureMode();               // End drawing to texture (now we have a texture available for next passes)

        BeginDrawing();

            BeginShaderMode(defaultShader);
                DrawTextureRec(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height }, (Vector2){ 0, 0 }, WHITE); // render texture with shader applied
            EndShaderMode();
            
            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}