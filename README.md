
# PROJECTS | JOSEPH | FREE TO USE!


--------------------------------------------------- INTRO ---------------------------------------------------------


Name: Joseph
Projects below.
You can share my projects with anyone ^^. 
My projects are **mostly** free to use, only **several projects** that i will *define* are like games, or AI ready projects that are not open-source, but the code is there for anyone to *read*, so a half open-source. - but it will be only several projects that are **ready**, that are more invested in *using* the application not **modifying**.

Projects:
# 1. OpenGL 2.1 Win32 - Cheat sheet: free use! (open-source, free use)
**Version:** 1.0.0-1.0.3 (next versions upcoming, check the list)
will start from version v1.0.0;
all versions are open-source, free use.
**very easy to understand and implement!**

# 2. BSP spliter
v1.0.0
v1.0.1 - fixed missing parts.
**very easy too to understand and implement!**


Screenshots of the OpenGL2.1 program 
V.1.0.4: this is still V.1.0.4, later versions upcoming.
![image](https://github.com/user-attachments/assets/5ef36566-e0b5-46b4-a595-3e21a69c1b72)
![image](https://github.com/user-attachments/assets/7b2530cd-2936-439f-a8c5-d54d0fc3bc75)
![image](https://github.com/user-attachments/assets/5064a6d3-2bbe-4221-b328-8646e1db949b)
![image](https://github.com/user-attachments/assets/e0a6fce4-4255-47ef-ad01-ec704c3ea3e2)







Note: don't worry, because there shouldn't be missing files.
even if there is, its only 3D models loaded into the renderer which is blocked by the Github because too large size;

the following code can be moified.



     Model cubeModel, barrelModel;
     cubeModel.position = (Vec3D){0.0f, 0.0f, 0.0f};

     cubeModel.vertices = cubeVertices; 
     cubeModel.colors = cubeColors;
     cubeModel.faces = cubeFaces;
     cubeModel.numVertices = 8;
     cubeModel.numColors = 8;
     cubeModel.numFaces = 12;
     cubeModel.name = 'C';
     
    barrelModel.position = (Vec3D){5.0f, 0.0f, 0.0f};
    barrelModel.numVertices = 32;
    barrelModel.numColors = 16;
    barrelModel.numFaces = 32;
    barrelModel.name = 'B';
      
barrelModel.vertices = (Vec3D[]) {
    // Top circle
    {0.5, 1, 0},
    {0.46, 1, 0.19},
    {0.35, 1, 0.35},
    {0.19, 1, 0.46},
    {0, 1, 0.5},
    {-0.19, 1, 0.46},
    {-0.35, 1, 0.35},
    {-0.46, 1, 0.19},
    {-0.5, 1, 0},
    {-0.46, 1, -0.19},
    {-0.35, 1, -0.35},
    {-0.19, 1, -0.46},
    {0, 1, -0.5},
    {0.19, 1, -0.46},
    {0.35, 1, -0.35},
    {0.46, 1, -0.19},//after this Bottom circle now
    {0.5, -1, 0},
    {0.46, -1, 0.19},
    {0.35, -1, 0.35},
    {0.19, -1, 0.46},
    {0, -1, 0.5},
    {-0.19, -1, 0.46},
    {-0.35, -1, 0.35},
    {-0.46, -1, 0.19},
    {-0.5, -1, 0},
    {-0.46, -1, -0.19},
    {-0.35, -1, -0.35},
    {-0.19, -1, -0.46},
    {0, -1, -0.5},
    {0.19, -1, -0.46},
    {0.35, -1, -0.35},
    {0.46, -1, -0.19},
};

barrelModel.faces = (GLuint[]) {
    // Sides
    0, 16, 1, 
    1, 16, 17,
    1, 17, 2,
    2, 17, 18,
    2, 18, 3,
    3, 18, 19,
    3, 19, 4,
    4, 19, 20,
    4, 20, 5,
    5, 20, 21,
    5, 21, 6,
    6, 21, 22,
    6, 22, 7,
    7, 22, 23,
    7, 23, 8,
    8, 23, 24,
    8, 24, 9,
    9, 24, 25,
    9, 25, 10,
    10, 25, 26,
    10, 26, 11,
    11, 26, 27,
    11, 27, 12,
    12, 27, 28,
    12, 28, 13,
    13, 28, 29,
    13, 29, 14,
    14, 29, 30,
    14, 30, 15,
    15, 30, 31,
    15, 31, 0,
    0, 31, 16,
};

barrelModel.colors = (Color[]) {
    {0.55, 0.27, 0.07}, // Brown
    {0.55, 0.27, 0.07},
    {0.55, 0.27, 0.07},
    {0.55, 0.27, 0.07},
    {0.55, 0.27, 0.07},
    {0.55, 0.27, 0.07},
    {0.55, 0.27, 0.07},
    {0.55, 0.27, 0.07},
    {0.55, 0.27, 0.07},
    {0.55, 0.27, 0.07},
    {0.55, 0.27, 0.07},
    {0.55, 0.27, 0.07},
    {0.55, 0.27, 0.07}
};

     Model city_1 = loadModel("cell_4_0_0.obj");
     Model city_2 = loadModel("cell_5_0_0.obj");
     Model city_3 = loadModel("cell_3_0_0.obj");
     Model city_4 = loadModel("cell_2_0_0.obj");
     Model city_5 = loadModel("cell_1_0_0.obj");
     Model city_6 = loadModel("cell_0_0_0.obj");

     scene1.models = malloc(8 * sizeof(Model)); //?
     scene1.models[0] = cubeModel;
     scene1.models[1] = barrelModel;
     scene1.models[2] = city_1;
     scene1.models[3] = city_2;
     scene1.models[4] = city_3;
     scene1.models[5] = city_4;
     scene1.models[6] = city_5;
     scene1.models[7] = city_6;
     
     scene1.numModels = 8;

    // Timing


**HOW TO COMPILE**
Do  gcc opengl8.c -o opengl8.exe -lglu32 -lgdi32 -lopengl32
 -mwindows -luser32
 .\opengl8.exe
![image](https://github.com/user-attachments/assets/925e31d4-f7cc-49c7-af28-d3e285fcf4e0)


---------------------------------------------------------------------------------------------------------------------
