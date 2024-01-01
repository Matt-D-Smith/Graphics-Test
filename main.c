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
#include "rlgl.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define MAP_SIZE 1024.0f
#define CHUNK_SIZE 128.0f
#define CHUNK_TEX_SCALE 4.0f

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif


bool IsOnMesh(Vector3 position, float height, Mesh * mesh, Matrix transform) {
    Ray ray = (Ray){position, (Vector3){0, -1, 0}};
    RayCollision rayCollision = GetRayCollisionMesh(ray, *mesh, transform);
    if (((rayCollision.distance - height) <= EPSILON) && ((rayCollision.distance - height) >= -EPSILON)) return true;
    // if (((rayCollision.distance - height) <= EPSILON) && rayCollision.hit) return true;
    return false;
}

typedef struct IVector2 {
    int x;                // Vector x component
    int y;                // Vector y component
} IVector2;

//Make chunks
typedef struct Chunk {
    IVector2 chunkID;                // Chunk ID, indicates its location in the world
    Model* models;
    Vector3* modelLocs;
    int numModels;
} Chunk;

IVector2 GetPosChunk(Vector3 position) {
    return (IVector2){(int)floor(position.x/CHUNK_SIZE), (int)floor(position.z/CHUNK_SIZE)};
}

void LoadChunk(Chunk* chunk, IVector2 chunkID) { // Loads the chunk data for chunkID into chunk
    // for now all we will have in the chunk is the section of the height map within that chunk
    chunk->chunkID = chunkID;

    char filename[] = "C:/Users/Matt/Desktop/Hardware-Stuff/Noise Textures/heightmap1024.png";
    Image heightMapImage = LoadImage(filename);
    Rectangle chunkMapRec = {
        .x = chunkID.x * CHUNK_SIZE + MAP_SIZE / 2,
        .y = chunkID.y * CHUNK_SIZE + MAP_SIZE / 2,
        .width = CHUNK_SIZE + 1,
        .height = CHUNK_SIZE + 1,
    };
    if (chunkMapRec.x + chunkMapRec.width  > MAP_SIZE) chunkMapRec.width = CHUNK_SIZE;
    if (chunkMapRec.y + chunkMapRec.height > MAP_SIZE) chunkMapRec.height = CHUNK_SIZE;
    ImageCrop(&heightMapImage, chunkMapRec);
    Mesh heightMapMesh = GenMeshHeightmap(heightMapImage, (Vector3){CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE});
    
    char texfilename[] = "C:/Users/Matt/Desktop/Hardware-Stuff/Noise Textures/heightmaptexture4096.png";
    Rectangle chunkMapTexRec = {
        .x = (chunkID.x * CHUNK_SIZE + MAP_SIZE/2) * CHUNK_TEX_SCALE,
        .y = (chunkID.y * CHUNK_SIZE + MAP_SIZE/2) * CHUNK_TEX_SCALE,
        .width = CHUNK_SIZE * CHUNK_TEX_SCALE,
        .height = CHUNK_SIZE * CHUNK_TEX_SCALE,
    };
    
    Image heightMapTexImage = LoadImage(texfilename);
    ImageCrop(&heightMapTexImage, chunkMapTexRec);
    chunk->numModels = 1;
    chunk->modelLocs = (Vector3*)malloc(sizeof(Vector3) * chunk->numModels);
    chunk->modelLocs[0] = (Vector3){chunkID.x * CHUNK_SIZE, 0 , chunkID.y * CHUNK_SIZE}; // give chunk origin coordinate to ground mesh origin

    chunk->models = (Model*)malloc(sizeof(Model) * chunk->numModels);
    chunk->models[0] = LoadModelFromMesh(heightMapMesh);
    chunk->models[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(heightMapTexImage);
    UnloadImage(heightMapImage);
    UnloadImage(heightMapTexImage);
    // UnloadMesh(heightMapMesh);
}

void DrawChunk(Chunk chunk, int lodLevel) {
    // For now the first model will always be the ground, draw that in relation to the chunk origin
    DrawModel(chunk.models[0], chunk.modelLocs[0], 1.0f, WHITE);
}


Mesh skyMesh;
Model skyModel;
Shader skyShader;

void makeSky() {
	skyMesh = GenMeshSphere(45000, 32, 15);
	skyModel = LoadModelFromMesh(skyMesh);
	skyShader = LoadShader("shaders/sky.vs", "shaders/sky.fs");

    float turbidityVal = 4.7; // 4.75;
    float rayleighVal = 2.28; // 6.77;
    float mieCoefficientVal = 0.003; //0.005; // 0.0191;
    float mieDirectionalGVal = 0.82; // 0.793;
    float luminanceVal = 1.00; // 1.1735;
    float inclinationVal = 0.3; //0.4983; // 0.4956;
    float azimuthVal = 0.1979; // 0.2174;
    float refractiveIndexVal = 1.00029; // 1.000633;
    float numMoleculesVal = 2.542e25;
    float depolarizationFactorVal = 0.02; // 0.01;
    float rayleighZenithLengthVal = 8400; // 1425;
    float mieVVal = 3.936; // 4.042;
    float mieZenithLengthVal = 34000; // 1600;
    float sunIntensityFactorVal = 1000; // 2069;
    float sunIntensityFalloffSteepnessVal = 1.5; // 2.26;
    float sunAngularDiameterDegreesVal = 0.00933; // 0.01487;
    float tonemapWeightingVal = 9.50;
    Vector3 primariesVal = { 6.8e-7, 5.5e-7, 4.5e-7 }; // { 7.929e-7, 3.766e-7, 3.172e-7 };
    Vector3 mieKCoefficientVal = { 0.686, 0.678, 0.666 }; // { 0.686, 0.678, 0.666 };
    Vector3 cameraPosVal = { 1000, -400, 0 };
    Vector3 sunPositionVal;

    float theta = M_PI * (inclinationVal - 0.5);
    float phi = 2 * M_PI * (azimuthVal - 0.5);

    float distance = 4000;
    sunPositionVal.x = distance * cos(phi);
    sunPositionVal.y = distance * sin(phi) * sin(theta);
    sunPositionVal.z = distance * sin(phi) * cos(theta);

    int turbidityLoc                    = GetShaderLocation(skyShader, "turbidity");
    int rayleighLoc                     = GetShaderLocation(skyShader, "rayleigh");
    int mieCoefficientLoc               = GetShaderLocation(skyShader, "mieCoefficient");
    int mieDirectionalGLoc              = GetShaderLocation(skyShader, "mieDirectionalG");
    int luminanceLoc                    = GetShaderLocation(skyShader, "luminance");
    int inclinationLoc                  = GetShaderLocation(skyShader, "inclination");
    int azimuthLoc                      = GetShaderLocation(skyShader, "azimuth");
    int refractiveIndexLoc              = GetShaderLocation(skyShader, "refractiveIndex");
    int numMoleculesLoc                 = GetShaderLocation(skyShader, "numMolecules");
    int depolarizationFactorLoc         = GetShaderLocation(skyShader, "depolarizationFactor");
    int rayleighZenithLengthLoc         = GetShaderLocation(skyShader, "rayleighZenithLength");
    int mieVLoc                         = GetShaderLocation(skyShader, "mieV");
    int mieZenithLengthLoc              = GetShaderLocation(skyShader, "mieZenithLength");
    int sunIntensityFactorLoc           = GetShaderLocation(skyShader, "sunIntensityFactor");
    int sunIntensityFalloffSteepnessLoc = GetShaderLocation(skyShader, "sunIntensityFalloffSteepness");
    int sunAngularDiameterDegreesLoc    = GetShaderLocation(skyShader, "sunAngularDiameterDegrees");
    int tonemapWeightingLoc             = GetShaderLocation(skyShader, "tonemapWeighting");
    int primariesLoc                    = GetShaderLocation(skyShader, "primaries");
    int mieKCoefficientLoc              = GetShaderLocation(skyShader, "mieKCoefficient");
    int cameraPosLoc                    = GetShaderLocation(skyShader, "cameraPos");
    int sunPositionLoc                  = GetShaderLocation(skyShader, "sunPosition");

    SetShaderValue(skyShader, turbidityLoc,                    &turbidityVal,                    SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, rayleighLoc,                     &rayleighVal,                     SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, mieCoefficientLoc,               &mieCoefficientVal,               SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, mieDirectionalGLoc,              &mieDirectionalGVal,              SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, luminanceLoc,                    &luminanceVal,                    SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, inclinationLoc,                  &inclinationVal,                  SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, azimuthLoc,                      &azimuthVal,                      SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, refractiveIndexLoc,              &refractiveIndexVal,              SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, numMoleculesLoc,                 &numMoleculesVal,                 SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, depolarizationFactorLoc,         &depolarizationFactorVal,         SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, rayleighZenithLengthLoc,         &rayleighZenithLengthVal,         SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, mieVLoc,                         &mieVVal,                         SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, mieZenithLengthLoc,              &mieZenithLengthVal,              SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, sunIntensityFactorLoc,           &sunIntensityFactorVal,           SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, sunIntensityFalloffSteepnessLoc, &sunIntensityFalloffSteepnessVal, SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, sunAngularDiameterDegreesLoc,    &sunAngularDiameterDegreesVal,    SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, tonemapWeightingLoc,             &tonemapWeightingVal,             SHADER_UNIFORM_FLOAT);
    SetShaderValue(skyShader, primariesLoc,                    &primariesVal,                    SHADER_UNIFORM_VEC3);
    SetShaderValue(skyShader, mieKCoefficientLoc,              &mieKCoefficientVal,              SHADER_UNIFORM_VEC3);
    SetShaderValue(skyShader, cameraPosLoc,                    &cameraPosVal,                    SHADER_UNIFORM_VEC3);
    SetShaderValue(skyShader, sunPositionLoc,                  &sunPositionVal,                  SHADER_UNIFORM_VEC3);


	skyModel.materials[0].shader = skyShader;
}

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
Camera camera = { 0 };
Vector3 cubePosition = { 0 };

void moveSun(float az, float el, float* azimuthVal, float* inclinationVal) {
    int azimuthLoc = GetShaderLocation(skyShader, "azimuth");
    *azimuthVal += az;
    SetShaderValue(skyShader, azimuthLoc, &azimuthVal, SHADER_UNIFORM_FLOAT);

    int inclinationLoc = GetShaderLocation(skyShader, "inclination");
    *inclinationVal += el;
    SetShaderValue(skyShader, inclinationLoc, &inclinationVal, SHADER_UNIFORM_FLOAT);
}

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 2560;
    const int screenHeight = 1440;

    const float playerHeight = 2.0f;

    InitWindow(screenWidth, screenHeight, "bad game made by a bad gamer");
    ToggleFullscreen();

    //----------------------------------------------------------------------------------
    // Initialize Shaders
    //----------------------------------------------------------------------------------

    // Shader shaderFirst = LoadShader("shaders/first.vs", "shaders/first.fs");
    // Shader shaderBlur = LoadShader(0, "shaders/blur.fs");

    makeSky();
    float inclinationVal = 0.4983; // 0.4956;
    float azimuthVal = 0.1979; // 0.2174;


    // Default Shader for final render to screen
    Shader defaultShader = LoadShader(0, 0);
    //----------------------------------------------------------------------------------
    // End Shaders
    //----------------------------------------------------------------------------------

    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, playerHeight, 2.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
    Camera updateCamera = camera;

    float CAMERA_MOVE_SPEED = 0.1f;         // Camera movement speed
    float CAMERA_MOUSE_MOVE_SENSITIVITY = 0.06f;

    int cameraMode = CAMERA_FIRST_PERSON;
    // int cameraMode = CAMERA_FREE;

    // Load chunks
    Chunk chunks[64];
    for (int chunkx = -4; chunkx < 4; chunkx += 1) {
        for (int chunky = -4; chunky < 4; chunky += 1) {
            LoadChunk(&chunks[(chunkx+4) * 8 + (chunky+4)], (IVector2){chunkx, chunky});
        }
    }

    DisableCursor();                    // Limit cursor to relative movement inside the window

    // Create a RenderTexture2D to be used for render to texture
    RenderTexture2D target = LoadRenderTexture(screenWidth, screenHeight);

    //--------------------------------------------------------------------------------------
    // SetTargetFPS(144);               // Set our game to run at 144 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {

        if (IsKeyPressed(KEY_ONE)) {
            cameraMode = CAMERA_FIRST_PERSON;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_TWO)) {
            cameraMode = CAMERA_FREE;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        //----------------------------------------------------------------------------------
        // Update Camera
        //----------------------------------------------------------------------------------
        
        // Sprint
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            // CAMERA_MOVE_SPEED = 20.0f;
            CAMERA_MOVE_SPEED = 200.0f;
        } 
        else {
            // CAMERA_MOVE_SPEED = 10.0f;
            CAMERA_MOVE_SPEED = 100.0f;
        }

        // Directional Movement
        Vector2 mousePositionDelta = GetMouseDelta();
        Vector3 moveVec = (Vector3){IsKeyDown(KEY_W) - IsKeyDown(KEY_S),
                                    IsKeyDown(KEY_D) - IsKeyDown(KEY_A),
                                    0.0};
        moveVec = Vector3Scale(Vector3Normalize(moveVec), CAMERA_MOVE_SPEED);
        moveVec.z = (cameraMode == CAMERA_FREE) * (IsKeyDown(KEY_SPACE) - IsKeyDown(KEY_LEFT_ALT)) * CAMERA_MOVE_SPEED;

        // Scale moveVec by deltaTime to get a consistent speed
        moveVec = Vector3Scale(moveVec,GetFrameTime());

        // Check collision with ground -- THIS NEEDS UPDATE TO INCLUDE MOVEVEC
        updateCamera = camera;
        UpdateCameraPro(&camera,
                        moveVec,
                        (Vector3){  mousePositionDelta.x * CAMERA_MOUSE_MOVE_SENSITIVITY,
                                    mousePositionDelta.y * CAMERA_MOUSE_MOVE_SENSITIVITY,
                                    0.0},
                        0.0);

        // Bounds checking
        if (camera.position.x >  MAP_SIZE/2) {
            camera.position.x = updateCamera.position.x;
            camera.target.x = updateCamera.target.x;
        }
        if (camera.position.z >  MAP_SIZE/2) {
            camera.position.z = updateCamera.position.z;
            camera.target.z = updateCamera.target.z;
        }
        if (camera.position.x < -MAP_SIZE/2) {
            camera.position.x = updateCamera.position.x;
            camera.target.x = updateCamera.target.x;
        }
        if (camera.position.z < -MAP_SIZE/2) {
            camera.position.z = updateCamera.position.z;
            camera.target.z = updateCamera.target.z;
        }

        IVector2 moveChunk = GetPosChunk(camera.position);
        int chunkIdx = (moveChunk.x+4) * 8 + (moveChunk.y+4);
        if ((cameraMode != CAMERA_FREE) && !IsOnMesh(camera.position, playerHeight, &chunks[chunkIdx].models->meshes[0], MatrixTranslate(chunks[chunkIdx].modelLocs[0].x, chunks[chunkIdx].modelLocs[0].y, chunks[chunkIdx].modelLocs[0].z))) {
            // Move a player to their height above the mesh
            Ray ray = (Ray){Vector3Add(camera.position, (Vector3){0,1000000,0}), (Vector3){0.0, -1.0, 0.0}};
            RayCollision rayCollision = GetRayCollisionMesh(ray, 
                                                            chunks[chunkIdx].models->meshes[0], 
                                                            MatrixTranslate(chunks[chunkIdx].modelLocs[0].x, chunks[chunkIdx].modelLocs[0].y, chunks[chunkIdx].modelLocs[0].z));
            if (rayCollision.hit == true) {
                camera.position.y = rayCollision.point.y + playerHeight;
                camera.target.y += 1000000 - rayCollision.distance + playerHeight;
            }
        }

        // Sun shader controls

        if (IsKeyPressed(KEY_KP_8)) {
            moveSun(0.01,0,&azimuthVal,&inclinationVal);
        }

        if (IsKeyPressed(KEY_KP_2)) {
            moveSun(-0.01,0,&azimuthVal,&inclinationVal);
        }

        if (IsKeyPressed(KEY_KP_4)) {
            moveSun(0,-0.01,&azimuthVal,&inclinationVal);
        }
        
        if (IsKeyPressed(KEY_KP_6)) {
            moveSun(0,0.01,&azimuthVal,&inclinationVal);
        }

        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginTextureMode(target);       // Enable drawing to texture

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                // DrawModel(heightMap, heightMapPos, 1.0f, WHITE);
                // DrawMesh(heightMapMeshLow, LoadMaterialDefault(), heightMapTransform);

                for (int chunkx = -4; chunkx < 4; chunkx += 1) {
                    for (int chunky = -4; chunky < 4; chunky += 1) {
                        DrawModel(chunks[(chunkx+4) * 8 + (chunky+4)].models[0], chunks[(chunkx+4) * 8 + (chunky+4)].modelLocs[0], 1.0f, WHITE);
                        // //Draw chunk origin
                        // Ray ray = {
                        //     .position = (Vector3){chunkx * CHUNK_SIZE, 0.0, chunky * CHUNK_SIZE},
                        //     .direction = (Vector3){0.0, 1.0, 0.0},
                        // };
                        // DrawRay(ray, RED);
                    }
                }
                DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
                DrawGrid(10, 1.0f);

                rlDisableBackfaceCulling();
                    DrawModel(skyModel, (Vector3){0.0, 0.0, 0.0}, 1, WHITE);
                rlEnableBackfaceCulling();

            EndMode3D();

            char posText[40];
            sprintf(posText, "%f %f %f", camera.position.x, camera.position.y, camera.position.z);
            DrawText(posText, 20, 40, 20, BLACK);

            // BeginShaderMode(shaderFirst);
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