#include "raylib.h"
#include "raymath.h"

Camera3D camera = { 0 };
Matrix RotateTilt = MatrixIdentity();

float DronePosX = 0.0f;
float DronePosY = 0.0f;
float DronePosZ = 0.0f;
float DroneRotAngle = 0.0f;
float PropellerSpin = 0.0f; //this is kind of like a "universal propeller clock" which constantly increases
float PropellerAngle; //and then some propellers go the opposite direction so this handles the conversion (later obviously)

float LeanForward = 0.0f; //both of these are the drone's tilt/lean; both can be negative too
float LeanLeft = 0.0f;

float speed = 30.0f; //drone's speed; this is a constant
float rotspeed = 3.0f; //drone's rotation speed
float tilt = 2.4f; //the speed in which the drone tilts
float maxtilt = 0.2f; //maximum tilt before it stops tilting

int CameraViewToggle = 0; //this toggles between the on-drone view (0), the drone POV (1) and the fixed view (2)
bool PropellersAreOn = false;

float dt; //deltatime
float gravity; //this is used when you turn off the propellers in midair

float ArmRotate;


//matrix for all transformations; just a typical localTRS thing
Matrix LocalTRS(Vector3 t, Matrix r, Vector3 s){
    return MatrixMultiply(MatrixMultiply(MatrixScale(s.x, s.y, s.z), r), MatrixTranslate(t.x, t.y, t.z));
}

//This is actually completely pointless because it's just a reimplementation of the MatrixMultiply function
//but the assignment details say I need to add it, so here it is. Every call of the ChildWorld function could've
//just been a MatrixMultiply function and the code would've been easier to read, but unfortunately I need to do
//exactly what the assignment says I must do.
Matrix ChildWorld(Matrix child, Matrix parent){
    return MatrixMultiply(child, parent);
}

void UpdateDrone(){

    //toggle between camera views on shift key press
    if (IsKeyPressed(KEY_RIGHT_SHIFT) || IsKeyPressed(KEY_LEFT_SHIFT)){
        CameraViewToggle = (CameraViewToggle + 1) % 3;
    }
    //toggle propellers
    if (IsKeyPressed(KEY_SPACE)){
        PropellersAreOn = !PropellersAreOn;
        gravity = -0.01;
    }


    //only update the drone if the propellers are on
    if (PropellersAreOn){

        //R and F are the easiest, they just move the drone up and down
        if (IsKeyDown(KEY_F)){DronePosY -= speed * dt;}
        if (IsKeyDown(KEY_R)){DronePosY += speed * dt;}
        //and prevent it from going lower if it hits the ground
        if (DronePosY < 0){DronePosY = 0;}

        //and these ones change the rotation of the drone
        if (IsKeyDown(KEY_Q)){DroneRotAngle += rotspeed * dt;}
        if (IsKeyDown(KEY_E)){DroneRotAngle -= rotspeed * dt;}

        //These ones are a bit harder since the horizontal speed depends on the rotation of the drone, but you can just multiply
        //the values by the cosine and the sine of the angle respectively. These also calculate the tilt (represented by LeanForward
        //and LeanLeft, which can be positive or negative)
        if (IsKeyDown(KEY_W)){
            DronePosZ -= speed * cos(DroneRotAngle) * dt;
            DronePosX -= speed * sin(DroneRotAngle) * dt;
            if (LeanForward > maxtilt * -1){LeanForward -= tilt * dt;}
        }
        if (IsKeyDown(KEY_S)){
            DronePosZ += speed * cos(DroneRotAngle) * dt;
            DronePosX += speed * sin(DroneRotAngle) * dt;
            if (LeanForward < maxtilt){LeanForward += tilt * dt;}
        }

        if (IsKeyDown(KEY_A)){
            DronePosX -= speed * cos(DroneRotAngle) * dt;
            DronePosZ += speed * sin(DroneRotAngle) * dt;
            if (LeanLeft < maxtilt){LeanLeft += tilt * dt;}
        }
        if (IsKeyDown(KEY_D)){
            DronePosX += speed * cos(DroneRotAngle) * dt;
            DronePosZ -= speed * sin(DroneRotAngle) * dt;
        if (LeanLeft > maxtilt * -1){LeanLeft -= tilt * dt;}
        }

        //This moves the tilt back to normal if the drone isn't moving. Note that both these conditionals are meant to be xnor values
        if (IsKeyDown(KEY_S) == IsKeyDown(KEY_W)){
            if (LeanForward < 0.0f){LeanForward += tilt * dt;}
            if (LeanForward > 0.0f){LeanForward -= tilt * dt;}
            if (abs(LeanForward - 0.0f) < 0.05f){LeanForward = 0.0f;}
        }
        if (IsKeyDown(KEY_A) == IsKeyDown(KEY_D)){
            if (LeanLeft < 0.0f){LeanLeft += tilt * dt;}
            if (LeanLeft > 0.0f){LeanLeft -= tilt * dt;}
            if (abs(LeanLeft - 0.0f) < 0.05f){LeanLeft = 0.0f;}
        }

        //Then we just have a few calculations we need to do

        //This is the thing that changes the orientation the propellers are supposed to be in (since it updates every frame).
        //It only updates if the propellers are on, obviously. Then, the speed depends on whether the drone is ascending or
        //descending, or staying still
        if (PropellersAreOn){
            if (IsKeyDown(KEY_R) == IsKeyDown(KEY_F)){PropellerSpin += 15.0f * dt;}
            else if (IsKeyDown(KEY_R)){PropellerSpin += 30.0f * dt;}
            else if (IsKeyDown(KEY_F)){PropellerSpin += 7.5f * dt;}
        }

        //And then this calculates the matrix for the tilt the drone needs to be at, given the previous LeanForward and LeanLeft vars
        RotateTilt = MatrixRotateXYZ((Vector3){LeanForward, 0.0f, LeanLeft});

    }

    //do this if the propellers are off instead. This just makes the drone fall down over time
    else{
        DronePosY += gravity;
        gravity -= 1.2f * dt;
        if (DronePosY < 0){DronePosY = 0;}
    }

    //And then if the X key is pressed, discard everything and reset all variables
    if (IsKeyPressed(KEY_X)){
        CameraViewToggle = 0; PropellersAreOn = false; DronePosX = 0.0f; DronePosY = 0.0f; DronePosZ = 0.0f;
        DroneRotAngle = 0.0f; PropellerSpin = 0.0f; LeanForward = 0.0f; LeanLeft = 0.0f;
        RotateTilt = MatrixRotateXYZ((Vector3){0.0f, 0.0f, 0.0f});
    }

    //take a screenshot
    if (IsKeyPressed(KEY_ONE)){
        TakeScreenshot("DroneScreenshot.png");
    }

}

/*

----------------------------------------------
DRAWING THE DRONE  it's fun
----------------------------------------------

*/


//I'm declaring these here to mean "no translation", "no rotation" and "no scaling" respectively.
//I ended up using these too many times when doing matrix transformations and I wanted a shorthand
Vector3 xt = (Vector3){0, 0, 0};
Matrix xr = MatrixIdentity();
Vector3 xs = (Vector3){1, 1, 1};

//This is the full hierarchy. They update every frame, which is why I set their actual values in the functions
//instead of just setting them during these declarations.
Matrix CentreOfDrone;
Matrix CentreWithTilt;
Matrix ArmBase;
Matrix Arm;
Matrix MotorHousing;
Matrix RotorShaft;
Matrix PropellerHub;
Matrix BladeA;
Matrix BladeB;

void PlaceCamera(){
    //standard on-drone view
    if (CameraViewToggle == 0){
        camera.target = Vector3{ DronePosX, DronePosY, DronePosZ };
        camera.position = (Vector3){DronePosX + (10.0f * sin(DroneRotAngle)), 5.0f + DronePosY, DronePosZ + 10.0f * cos(DroneRotAngle)};
    }
    //drone POV
    else if (CameraViewToggle == 1){
        camera.target = Vector3{ DronePosX - (3.5f * sin(DroneRotAngle)), DronePosY - 4.5f, DronePosZ - 3.5f * cos(DroneRotAngle)};
        camera.position = (Vector3){DronePosX, DronePosY - 1.0f, DronePosZ};
    }
    //fixed camera
    else{
        camera.target = (Vector3){0.0f,0.0f,0.0f};
        camera.position = (Vector3){100.0f,100.0f,100.0f};
    }
}

void DrawGround(Mesh Cube, Mesh Cylinder){

    Material Grey = LoadMaterialDefault();
    Material Black = LoadMaterialDefault();
    Grey.maps[MATERIAL_MAP_DIFFUSE].color = GRAY;
    Black.maps[MATERIAL_MAP_DIFFUSE].color = BLACK;

    //ground
    DrawMesh(Cube, Grey, LocalTRS((Vector3){0.0f, -1.0f, 0.0f}, xr, (Vector3){300.0f, 0.0f, 300.0f}));

    for (int i = 1; i <= 400; i++){
        //grass. The sine functions are just meant to be pseudorandom
        DrawMesh(Cube, Black, LocalTRS((Vector3){150.0f * sin(2000.1f * i), -0.5f, 150.0f * sin(1500.2f * i)}, xr, (Vector3){0.5f, 1.0f, 0.5f}));
    }
    
    //drop shadow (not with a shader. Hopefully this is alright)
    //the material is a transparent black which gets more transparent as the drone goes higher
    Material BlackTransparent = Black;
    BlackTransparent.maps[MATERIAL_MAP_DIFFUSE].color = (Color){0, 0, 0, (40 / (int)(DronePosY + 1.0f))};
    DrawMesh(Cylinder, BlackTransparent, LocalTRS((Vector3){DronePosX, -1.0f, DronePosZ}, xr, (Vector3){7.0f, 0.01f, 7.0f}));

}

void DrawDrone(Mesh Cube, Mesh Cylinder){

    Material Gold = LoadMaterialDefault();
    Material LightGrey = LoadMaterialDefault();
    Material LessLightGrey = LoadMaterialDefault();
    Material Grey = LoadMaterialDefault();
    Material Black = LoadMaterialDefault();

    Gold.maps[MATERIAL_MAP_DIFFUSE].color = GOLD;
    LightGrey.maps[MATERIAL_MAP_DIFFUSE].color = LIGHTGRAY;
    LessLightGrey.maps[MATERIAL_MAP_DIFFUSE].color = Color{170, 170, 170, 255};
    Grey.maps[MATERIAL_MAP_DIFFUSE].color = GRAY;
    Black.maps[MATERIAL_MAP_DIFFUSE].color = BLACK;

    ///hierarchy bit
    CentreOfDrone = LocalTRS((Vector3){DronePosX, DronePosY, DronePosZ}, MatrixRotateY(DroneRotAngle), xs);
    CentreWithTilt = LocalTRS((Vector3){DronePosX, DronePosY, DronePosZ}, ChildWorld(RotateTilt, MatrixRotateY(DroneRotAngle)), xs);

    //main drone body
    DrawMesh(Cube, LightGrey, ChildWorld(LocalTRS(xt, xr, (Vector3){1.5f, 1.0f, 4.0f}), CentreWithTilt));
    DrawMesh(Cylinder, LightGrey, ChildWorld(LocalTRS((Vector3){0.0f, -0.5f, -2.0f}, xr, (Vector3){1.5f, 1.0f, 1.5f}), CentreWithTilt));
    
    //landing pad thing
    DrawMesh(Cube, Black, ChildWorld(LocalTRS((Vector3){-0.4f, -0.9f, 0.0f}, xr, (Vector3){0.3f, 0.2f, 4.0f}), CentreWithTilt));
    DrawMesh(Cube, Black, ChildWorld(LocalTRS((Vector3){0.4f, -0.9f, 0.0f}, xr, (Vector3){0.3f, 0.2f, 4.0f}), CentreWithTilt));
    DrawMesh(Cube, Black, ChildWorld(LocalTRS((Vector3){-0.4f, -0.45f, 0.0f}, xr, (Vector3){0.3f, 0.9f, 1.5f}), CentreWithTilt));
    DrawMesh(Cube, Black, ChildWorld(LocalTRS((Vector3){0.4f, -0.45f, 0.0f}, xr, (Vector3){0.3f, 0.9f, 1.5f}), CentreWithTilt));

    //black stripy band (Super Fun Decoration)
    DrawMesh(Cube, Black, ChildWorld(LocalTRS(xt, xr, (Vector3){1.51f, 0.3f, 4.01f}), CentreWithTilt));
    DrawMesh(Cylinder, Black, ChildWorld(LocalTRS((Vector3){0.0f, -0.15f, -2.0f}, xr, (Vector3){1.51f, 0.3f, 1.51f}), CentreWithTilt));
    
    //circles (also fun decoration but also where the navigations lights go)
    DrawMesh(Cylinder, Grey, ChildWorld(LocalTRS((Vector3){0.0f, 0.5f, 0.0f}, xr, (Vector3){1.3f, 0.01f, 1.3f}), CentreWithTilt));
    DrawMesh(Cylinder, LessLightGrey, ChildWorld(LocalTRS((Vector3){0.0f, 0.5f, 1.2f}, xr, (Vector3){0.8f, 0.01f, 0.8f}), CentreWithTilt));
    DrawMesh(Cylinder, LessLightGrey, ChildWorld(LocalTRS((Vector3){0.0f, 0.5f, -1.2f}, xr, (Vector3){0.8f, 0.01f, 0.8f}), CentreWithTilt));

    //navigation lights
    if (PropellersAreOn){
        //The first one is the central light that's always on when the drone is on. The rest are for their respective key inputs
        DrawMesh(Cylinder, Gold, ChildWorld(LocalTRS((Vector3){0.0f, 0.5f, 0.0f}, xr, (Vector3){0.5f, 0.03f, 0.5f}), CentreWithTilt));
        if (IsKeyDown(KEY_W)){
            DrawMesh(Cube, Gold, ChildWorld(LocalTRS((Vector3){0.0f, 0.5f, -0.45f}, xr, (Vector3){0.2f, 0.03f, 0.2f}), CentreWithTilt));
        }
        if (IsKeyDown(KEY_S)){
            DrawMesh(Cube, Gold, ChildWorld(LocalTRS((Vector3){0.0f, 0.5f, 0.45f}, xr, (Vector3){0.2f, 0.03f, 0.2f}), CentreWithTilt));
        }
        if (IsKeyDown(KEY_D)){
            DrawMesh(Cube, Gold, ChildWorld(LocalTRS((Vector3){0.45f, 0.5f, 0.0f}, xr, (Vector3){0.2f, 0.03f, 0.2f}), CentreWithTilt));
        }
        if (IsKeyDown(KEY_A)){
            DrawMesh(Cube, Gold, ChildWorld(LocalTRS((Vector3){-0.45f, 0.5f, 0.0f}, xr, (Vector3){0.2f, 0.03f, 0.2f}), CentreWithTilt));
        }
        if (IsKeyDown(KEY_Q)){
            DrawMesh(Cube, Gold, ChildWorld(LocalTRS((Vector3){-0.32f, 0.5f, -0.32f}, MatrixRotateY(PI/4.0f), (Vector3){0.2f, 0.03f, 0.2f}), CentreWithTilt));
        }
        if (IsKeyDown(KEY_E)){
            DrawMesh(Cube, Gold, ChildWorld(LocalTRS((Vector3){0.32f, 0.5f, -0.32f}, MatrixRotateY(PI/4.0f), (Vector3){0.2f, 0.03f, 0.2f}), CentreWithTilt));
        }
        if (IsKeyDown(KEY_R)){
            DrawMesh(Cylinder, Gold, ChildWorld(LocalTRS((Vector3){0.0f, 0.5f, -1.2f}, xr, (Vector3){0.3f, 0.03f, 0.3f}), CentreWithTilt));
        }
        if (IsKeyDown(KEY_F)){
            DrawMesh(Cylinder, Gold, ChildWorld(LocalTRS((Vector3){0.0f, 0.5f, 1.2f}, xr, (Vector3){0.3f, 0.03f, 0.3f}), CentreWithTilt));
        }
    }
    
}

void DrawDroneApparatus(Mesh Cube, Mesh Cylinder, float angle, int index){

    Material Red = LoadMaterialDefault();
    Material LightGrey = LoadMaterialDefault();
    Material LessLightGrey = LoadMaterialDefault();

    Red.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    LightGrey.maps[MATERIAL_MAP_DIFFUSE].color = LIGHTGRAY;
    LessLightGrey.maps[MATERIAL_MAP_DIFFUSE].color = Color{170, 170, 170, 255};

    //These need to be declared inside this function because the angle of the arm is different every time
    ArmBase = ChildWorld(LocalTRS(xt, MatrixRotateY(angle), xs), CentreWithTilt);
    Arm = ChildWorld(LocalTRS((Vector3){-2.2f, 0.0f, 0.0f}, xr, xs), ArmBase);
    MotorHousing = ChildWorld(LocalTRS((Vector3){-1.5f, -0.5f, 0.0f}, xr, xs), Arm);
    RotorShaft = ChildWorld(LocalTRS((Vector3){0.0f, 1.0f, 0.0f}, xr, xs), MotorHousing);
    PropellerHub = ChildWorld(LocalTRS((Vector3){0.0f, 1.0f, 0.0f}, xr, xs), RotorShaft);

    //and the blade position depends on the PropellerSpinvariable, so that's accounted for here
    if (index % 2 == 0){PropellerAngle = PropellerSpin;}
    else{PropellerAngle = -PropellerSpin;}
    BladeA = ChildWorld(LocalTRS(xt, MatrixRotateY(PropellerAngle), xs), PropellerHub);
    BladeB = ChildWorld(LocalTRS(xt, MatrixRotateY(PropellerAngle + (PI / 2.0f)), xs), PropellerHub);
    
    //just draw them. The only transformation I'm using here is scaling since all the above things are derived from
    //each other since they're in a hierarchy
    DrawMesh(Cube, LightGrey, ChildWorld(LocalTRS(xt, xr, (Vector3){3.0f, 0.4f, 0.4f}), Arm)); //arm
    DrawMesh(Cylinder, LessLightGrey, MotorHousing); //motor housing
    DrawMesh(Cylinder, LightGrey, ChildWorld(LocalTRS(xt, xr, (Vector3){0.3f, 1.0f, 0.3f}), RotorShaft)); //rotor shaft
    DrawMesh(Cylinder, Red, ChildWorld(LocalTRS(xt, xr, (Vector3){0.7f, 0.4f, 0.7f}), PropellerHub)); //propeller hub
    DrawMesh(Cube, Red, ChildWorld(LocalTRS(xt, xr, (Vector3){3.0f, 0.4f, 0.4f}), BladeA)); //blade A 
    DrawMesh(Cube, Red, ChildWorld(LocalTRS(xt, xr, (Vector3){3.0f, 0.4f, 0.4f}), BladeB)); //blade B

}

int main()
{
    const int screenWidth = 1600;
    const int screenHeight = 900;

    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    InitWindow(screenWidth, screenHeight, "Drone Thing");
    SetTargetFPS(60);
    
    //all the shapes I will use
    Mesh Cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Mesh Cylinder = GenMeshCylinder(0.5f, 1.0f, 32);

    while (!WindowShouldClose())
    {

        //deltatime
        dt = GetFrameTime();

        UpdateDrone();

        PlaceCamera();

        BeginDrawing();
        ClearBackground(SKYBLUE);
        BeginMode3D(camera);

        DrawGround(Cube, Cylinder);
        if (CameraViewToggle != 1){
            DrawDrone(Cube, Cylinder);
            
            ArmRotate = PI / 4.0f; //We will be running all the next code 6 times, for the 6 arm propellor thingies
            for (int i = 0; i < 4; i++){
                DrawDroneApparatus(Cube, Cylinder, ArmRotate, i);
                ArmRotate += PI / 2.0f;
            }
        }

        EndMode3D();
        EndDrawing();
    }

    UnloadMesh(Cube);
    UnloadMesh(Cylinder);

    CloseWindow();
    return 0;
}
