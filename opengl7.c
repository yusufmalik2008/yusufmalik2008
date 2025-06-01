
/*
soo the step for my current project:
FIRST SECTION
1. BSP grid to work on my project,
2. load a model and draw which uses BSP grid soo enchances very performance.
3. add mouse controls, and collision detection using the grid BSP.
4. we add basic lighting (basic opengl one. in this opengl 2.1 win32 c)
SECOND SECTION
5. we add a whole city, a big city, and a whole game map, so that it feels 'lively' (just design the whole maps - the whole models)
a) we can also add lightings everywhere that we want. (maybe we can use bsp for placing the lightings, to place it correctly.)


ohh my - thats very easy to understand, - and im glad my renderer is looking so, like so good rn it looked like morrowind, i swear - on top of that, its very simple - the code, im gonna publish it on Github for "My cheat sheet for making OpenGL on C - free to use!" but thatll be later



in the function:

        // Calculate radius for sphere frustum culling (assuming the model fits in a sphere)

        float radius = calculateBoundingSphereRadius(model.vertices, model.numVertices); // Adjust this value based on your model's size
        if(SphereInFrustum(planes, model.position.x, model.position.y, model.position.z, radius) && distance < 100.0f) {
        // For the model position


        you can change distance to 70-100, for LOD options
*/
// ------------------------------------------------- IMPORT LIBS -----------------------------------------------------

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_VERTICES 1000000
#define MAX_FACES    2000000
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
//GLuint fbo, fboTex, fboDepth;

// ------------------------------------------------- CAMERA POSITION AND ROTATION ----------------------------------



// Camera position and rotation
float camX = 0.0f, camY = 0.0f, camZ = 0.0f;
float camPitch = 0.0f, camYaw = 0.0f;
// float objX = 0.0f, objY = 0.0f, objZ = 0.0f;



// ---------------------------------------------- WINDOW GLOBALS AND KEY STATES ----------------------------------


// Window globals
HWND g_hWnd;
HDC g_hDC;
HGLRC g_hRC;

// Key states
bool keys[256] = { false };


// MATH FUNCTIONS AND FRUSTUM CULLING -------------------------------------------------------------------------

// This function
void ExtractFrustumPlanes(float planes[6][4], float proj[16], float modl[16]) {
    float clip[16];
    float t;

    // Combine projection and modelview matrices, from the 3D game we're making here - we've made Projection and modelview matrices
    // before, in any programming 3d games it exists - you can google it.
    for(int i =0;i < 16; i++) {
        int row = i / 4, col = i % 4;
        clip[i] = modl[row * 4 + 0] * proj[col + 0] +
                  modl[row * 4 + 1] * proj[col + 4] +
                  modl[row * 4 + 2] * proj[col + 8] +
                  modl[row * 4 + 3] * proj[col + 12];

    }


    // Extract and normalize planes (Left, right, bottom, top, near, far)
    for(int i = 0; i < 6; ++i) {
        int a = i % 2 ? -1 : 1;
        int row = i / 2;
        for(int j = 0; j < 4; ++j)
            planes[i][j] = clip[3 + j * 4] + a * clip[row + j * 4];

            t = sqrtf(planes[i][0]*planes[i][0] +
                      planes[i][1]*planes[i][1] +
                      planes[i][2]*planes[i][2]);
            for(int j=0; j<4; ++j)
                planes[i][j] /= t;
    }
}


int SphereInFrustum(float planes[6][4], float x, float y, float z, float radius) {
    for(int i =0;i<6;i++)
       if(planes[i][0]*x + planes[i][1]*y + planes[i][2]*z + planes[i][3] <= -radius)
          return 0;
    
    return 1;
}



// Clamp function
float clamp(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}


// ----- --------------------------------------setting Up the Window!-------------------------------------------


// Set up pixel format for OpenGL context
bool SetupPixelFormat(HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1 };
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int pf = ChoosePixelFormat(hdc, &pfd);
    if (pf == 0) return false;
    if (!SetPixelFormat(hdc, pf, &pfd)) return false;
    return true;
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        keys[wParam] = true;
        if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
        return 0;
    case WM_KEYUP:
        keys[wParam] = false;
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }


}


// --------------------------------------------------- MODELS AND SCENE ----------------------------------------------


typedef struct {
    GLfloat x, y, z;
} Vec3D;

typedef struct {
    GLfloat r, g, b;
} Color;

typedef struct {
    Vec3D position;
    Vec3D* vertices;
    Color* colors;
    GLuint* faces;
    GLuint numVertices;
    GLuint numColors;
    GLuint numFaces;
    char name;
} Model;

typedef struct {
    Model* models;
    GLuint numModels;
} Scene;




bool loadOBJ(const char* filename, Vec3D** outVertices, GLuint** outFaces, int* outNumVertices, int* outNumFaces) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;

    Vec3D* vertices = malloc(MAX_VERTICES * sizeof(Vec3D));
    GLuint* faces = malloc(MAX_FACES * 3 * sizeof(GLuint));
    int vcount = 0, fcount = 0;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            vertices[vcount++] = (Vec3D){x, y, z};
        } else if (line[0] == 'f') {
            int a, b, c, d;
            int count = sscanf(line, "f %d %d %d %d", &a, &b, &c, &d);
            if (count == 3) {
                // Triangle
                faces[fcount++] = a - 1;
                faces[fcount++] = b - 1;
                faces[fcount++] = c - 1;
            } else if (count == 4) {
                faces[fcount++] = a - 1;
                faces[fcount++] = b - 1;
                faces[fcount++] = c - 1;

                faces[fcount++] = a - 1;
                faces[fcount++] = c - 1;
                faces[fcount++] = d - 1;
            }
        }
    }
    fclose(file);
    *outVertices = vertices;
    *outFaces = faces;
    *outNumVertices = vcount;
    *outNumFaces = fcount / 3;
    return true;
}


Model loadModel(const char* filename) {
    Vec3D* verts = NULL;
    GLuint* faces = NULL;
    int vcount = 0, fcount = 0;
    Model model;
    if(loadOBJ(filename, &verts, &faces, &vcount, &fcount)) {
        // Compute center of geometry
        Vec3D center = {0,0,0};
        for(int i=0;i<vcount;i++) {
            center.x += verts[i].x;
            center.y += verts[i].y;
            center.z += verts[i].z;
            
        }
        center.x /= vcount;
        center.y /= vcount;
        center.z /= vcount;
        model.position = center;

        model.vertices = verts;
        model.faces    = faces;
        model.numVertices = vcount;
        model.numFaces    = fcount;

        // Assign random color
        Color* colors = malloc(vcount * sizeof(Color));
        for(int i=0;i<vcount;i++) {
            colors[i].r = (float)rand() / RAND_MAX;
            colors[i].g = (float)rand() / RAND_MAX;
            colors[i].b = (float)rand() / RAND_MAX;
        }
        model.colors = colors;
        model.numColors = vcount;
    }

    return model;
}

float calculateBoundingSphereRadius(Vec3D* vertices, int numVertices) {
    /// Calculate the center
    Vec3D center = {0,0,0};
    for(int i=0;i<numVertices;i++ ) {
        center.x += vertices[i].x;
        center.y += vertices[i].y;
        center.z += vertices[i].z;
    }

    center.x /= numVertices;
    center.y /= numVertices;
    center.z /= numVertices;
    
    float radius = 0;
    for(int i = 0;i< numVertices;i++) {
        float dx = vertices[i].x - center.x;
        float dy = vertices[i].y - center.y;
        float dz = vertices[i].z - center.z;
        float distance = sqrtf(dx*dx + dy*dy + dz*dz);
        if(distance > radius) {
            radius = distance;
        }
    }

    return radius;
}
// -------------------------- ------------------------------------------ ----------------------------

// CUBE
Vec3D cubeVertices[] = {
     {-1, -1, -1},
    {1, -1, -1},
    {1, 1, -1},
    {-1, 1, -1},
    {-1, -1, 1},
    {1, -1, 1},
    {1, 1, 1},
    {-1, 1, 1}
};


Color cubeColors[] = {
    {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0},
    {1, 0, 1}, {0, 1, 1}, {1, 1, 1}, {0, 0, 0}
};


GLuint cubeFaces[] = {
    0, 1, 2, 2, 3, 0, // Front face
    1, 5, 6, 6, 2, 1, // Right face
    5, 4, 7, 7, 6, 5, // Back face
    4, 0, 3, 3, 7, 4, // Left face
    4, 5, 1, 1, 0, 4, // Bottom face
    3, 2, 6, 6, 7, 3  // Top face
};



//static Model cubeModel, barrelModel;
static Scene scene1;


void drawModels(Scene scene, float planes[6][4], float camX, float camY, float camZ) {
    for(int i=0;i< scene.numModels;i++) {
        Model model = scene.models[i];
        char name = model.name;

        // Calculate distance from the camera to the model
        float dx = model.position.x - camX;
        float dy = model.position.y - camY;
        float dz = model.position.z - camZ;
        float distance = sqrtf(dx*dx + dy*dy + dz*dz);

        // Calculate radius for sphere frustum culling (assuming the model fits in a sphere)

        float radius = calculateBoundingSphereRadius(model.vertices, model.numVertices); // Adjust this value based on your model's size
        if(SphereInFrustum(planes, model.position.x, model.position.y, model.position.z, radius) && distance < 100.0f) {
        // For the model position
        glPushMatrix(); // Save the current matrix
       // glTranslatef(model.position.x, model.position.y, model.position.z);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(Vec3D), model.vertices);
        glColorPointer(3, GL_FLOAT, sizeof(Color), model.colors);
        glDrawElements(GL_TRIANGLES, model.numFaces*3, GL_UNSIGNED_INT, model.faces);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glPopMatrix(); // Restore the previous matrix.
        } else {
            if(distance > 100.0f) printf("\r(F) Model %d: is too far away!\n", i);
            else printf("\r(N) Model, SphereRadius: %d: is not visible!", i); printf("%f, calculated bounding sphere radius of the current model.", calculateBoundingSphereRadius(model.vertices, model.numVertices));
            fflush(stdout);
        }
        
    }
}

// --------------------------------------- TEST (DEMO) -------------------------------------------------------------


 static const GLfloat vertices[8][3] = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1,1},  {1,-1,1},  {1,1,1},  {-1,1,1}
    };



    // Cube faces (quads), each face uses 4 vertex indices
    static const GLint faces[6][4] = {
        {0,1,2,3}, // Back face
        {4,5,6,7}, // Front face
        {0,4,7,3}, // Left face
        {1,5,6,2}, // Right face
        {3,2,6,7}, // Top face
        {0,1,5,4}  // Bottom face
    };

    static const GLuint tri_faces[12][3] = {
    {0,1,2}, {0,2,3}, // Back face
    {4,5,6}, {4,6,7}, // Front face
    {0,4,7}, {0,7,3}, // Left face
    {1,5,6}, {1,6,2}, // Right face
    {3,2,6}, {3,6,7}, // Top face
    {0,1,5}, {0,5,4}  // Bottom face
};

    // Colors for each face
    static const GLfloat colors[6][3] = {
        {1,0,0}, {0,1,0}, {0,0,1}, {1,1,0}, {1,0,1}, {0,1,1}
    };

  static const GLfloat tri_colors[12][3] = {
    {0.98f, 0.23f, 0.19f}, // Red (Apple Red)
    {1.00f, 0.58f, 0.00f}, // Orange
    {1.00f, 0.80f, 0.20f}, // Yellow
    {0.30f, 0.85f, 0.39f}, // Green
    {0.00f, 0.48f, 1.00f}, // Blue
    {0.35f, 0.35f, 0.81f}, // Indigo
    {0.56f, 0.27f, 0.68f}, // Purple
    {1.00f, 0.36f, 0.64f}, // Pink
    {0.60f, 0.60f, 0.60f}, // Silver/Gray
    {0.20f, 0.80f, 0.80f}, // Teal
    {0.90f, 0.49f, 0.13f}, // Gold
    {0.94f, 0.94f, 0.96f}  // Light (off-white)
};
// Draw cube function (like Benny's cube, using immediate mode)
void DrawCube() {
    
   // -------------- VERTEX ARRAY MODE ------------------
   // ----------------- DRAW USING TRIANGLES ------------------
   
   
   // glEnableClientState makes OpenGL
   // use vertex arrays instead of
   // immediate mode, which is more 
   // efficient for rendering.
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   
   for(int i=0;i<12;i++) {
    glColor3fv(tri_colors[i]); // Set color for the current face.
    // Easily explained:
    // glVertexPointer specifies the location
    // and data format of the array of vertex coordinates.
    // glDrawElements draws the specified primitives
    // using the vertex array and the indices
    // meaning it will draw the vertices
    // specified in the faces array.
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, tri_faces[i]);
   }

   // Disable vertex array state after drawing
   // to avoid affecting other rendering operations.
   
    glDisableClientState(GL_VERTEX_ARRAY);
    
   /*
   // glEnableClientState makes OpenGL
   // use vertex arrays instead of
   // immediate mode, which is more 
   // efficient for rendering.
   glEnableClientState(GL_VERTEX_ARRAY);

   for(int i=0;i<6;i++) {
    glColor3fv(colors[i]); // Set color for the current face.
    // Easily explained:
    // glVertexPointer specifies the location
    // and data format of the array of vertex coordinates.
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    // glDrawElements draws the specified primitives
    // using the vertex array and the indices
    // meaning it will draw the vertices
    // specified in the faces array.
    glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, faces[i]);
   }

   // Disable vertex array state after drawing
   // to avoid affecting other rendering operations.
   
    glDisableClientState(GL_VERTEX_ARRAY);
   *//* 
   
   ------------------ IMMEDIATE MODE---------------
   
   // Cube vertices
    static const GLfloat vertices[8][3] = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1,1},  {1,-1,1},  {1,1,1},  {-1,1,1}
    };

    // Cube faces (quads), each face uses 4 vertex indices
    static const GLint faces[6][4] = {
        {0,1,2,3}, // Back face
        {4,5,6,7}, // Front face
        {0,4,7,3}, // Left face
        {1,5,6,2}, // Right face
        {3,2,6,7}, // Top face
        {0,1,5,4}  // Bottom face
    };

    // Colors for each face
    static const GLfloat colors[6][3] = {
        {1,0,0}, {0,1,0}, {0,0,1}, {1,1,0}, {1,0,1}, {0,1,1}
    };

    glBegin(GL_QUADS);
    for (int i = 0; i < 6; i++) {
        glColor3fv(colors[i]);
        for (int j = 0; j < 4; j++) {
            glVertex3fv(vertices[faces[i][j]]);
        }
    }
    glEnd();
    */
}
// --------------------------------------------------DEMO -------------------------------------------------------------


// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASS wc = { 0 };
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "OpenGLWinClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClass(&wc)) return -1;

    AllocConsole();  // Creates a console window
freopen("CONOUT$", "w", stdout);  // Redirect stdout to the console
freopen("CONOUT$", "w", stderr);  // Redirect stderr (optional)

    // To position the window in the center
    int pcWidth = GetSystemMetrics(SM_CXSCREEN);
    int pcHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowX = (pcWidth - 800) / 2;
    int windowY = (pcHeight - 600) / 2;


    // Create window
    g_hWnd = CreateWindow(
        wc.lpszClassName,
        "OpenGL",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        windowX, windowY,
        800, 600,
        NULL, NULL, hInstance, NULL
    );
    if (!g_hWnd) return -2;

    g_hDC = GetDC(g_hWnd);
    if (!SetupPixelFormat(g_hDC)) return -3;

    g_hRC = wglCreateContext(g_hDC);
    wglMakeCurrent(g_hDC, g_hRC);

    // --------------------------------------- LOAD FUNCTIONS  -----------------------------
    

    // Enable depth test and smooth shading
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    // --------------------------------------- MODELS AND SCENERY - what's here?? --------------------------
    srand((unsigned int)time(NULL));
        

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
    DWORD lastTime = GetTickCount();


    //  ---------------------------------------Main Loop---------------------------------------------------


    // Main loop
    MSG msg;
    bool running = true;
    while (running) {
        // Process messages
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                running = false;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Time delta
        DWORD currentTime = GetTickCount();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        static DWORD lastPrint = 0;
if (GetTickCount() - lastPrint > 1000) {
    printf("Camera: (%.2f, %.2f, %.2f)\n", camX, camY, camZ);
    for (int i = 0; i < scene1.numModels; i++) {
        Model m = scene1.models[i];
        float dx = m.position.x - camX;
        float dy = m.position.y - camY;
        float dz = m.position.z - camZ;
        float dist = sqrtf(dx*dx + dy*dy + dz*dz);
        printf("Model %d pos: (%.2f, %.2f, %.2f), dist: %.2f\n", i, m.position.x, m.position.y, m.position.z, dist);
       }
     lastPrint = GetTickCount();
     }
        // Movement speed
        float moveSpeed = 5.0f * deltaTime;
        float rotSpeed = 90.0f * deltaTime;

        // Camera rotation with arrow keys
        if (keys[VK_LEFT]) camYaw -= rotSpeed;
        if (keys[VK_RIGHT]) camYaw += rotSpeed;
        if (keys[VK_UP]) camPitch -= rotSpeed;
        if (keys[VK_DOWN]) camPitch += rotSpeed;

        camPitch = clamp(camPitch, -89.0f, 89.0f);

        // Calculate forward vector
        float yawRad = camYaw * (3.14159265f / 180.0f);
        float pitchRad = camPitch * (3.14159265f / 180.0f);

        float forwardX = cosf(pitchRad) * sinf(yawRad);
        float forwardY = sinf(pitchRad);
        float forwardZ = cosf(pitchRad) * cosf(yawRad);

        // Right vector
        float rightX = sinf(yawRad - 3.14159265f / 2.0f);
        float rightZ = cosf(yawRad - 3.14159265f / 2.0f);

        // Movement with WASD
        if (keys['W']) {
            camX += forwardX * moveSpeed;
            camY += forwardY * moveSpeed;
            camZ += forwardZ * moveSpeed;
        }
        if (keys['S']) {
            camX -= forwardX * moveSpeed;
            camY -= forwardY * moveSpeed;
            camZ -= forwardZ * moveSpeed;
        }
        if (keys['A']) {
            camX += rightX * moveSpeed;
            camZ += rightZ * moveSpeed;
        }
        if (keys['D']) {
            camX -= rightX * moveSpeed;
            camZ -= rightZ * moveSpeed;
        }

        // Clear screen
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup viewport and projection
        //RECT rect;
        //GetClientRect(g_hWnd, &rect);
        //int width = rect.right - rect.left;
        //int height = rect.bottom - rect.top;


        //int width = 320, height = 240;
       /*
        int winWidth = 1360, winHeight = 720;
        int baseWidth = 320, baseHeight = 240;

        float scaleX = (float)winWidth / baseWidth;
        float scaleY = (float)winHeight / baseHeight;
        float scale = (scaleX < scaleY) ? scaleX : scaleY;

        int viewportWidth = (int)(baseWidth * scale);
        int viewportHeight = (int)(baseHeight * scale);
        int viewportX = (winWidth - viewportWidth) / 2;
        int viewportY = (winHeight - viewportHeight) / 2;
        */

        glViewport(0, 0, 800, 600);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0, 320.0f / 240.0f, 0.1, 100.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Calculate camera lookAt target
        float lookX = camX + forwardX;
        float lookY = camY + forwardY;
        float lookZ = camZ + forwardZ;

        gluLookAt(camX, camY, camZ,
                  lookX, lookY, lookZ,
                  0, 1, 0);

        // Rotate cube over time
        static float cubeAngle = 0.0f;
        cubeAngle += 45.0f * deltaTime; // 45 degrees per second
        if (cubeAngle > 360.0f) cubeAngle -= 360.0f;

        // Extract frustum planes
        float proj[16], modl[16], planes[6][4]; //Get projection and modelview matrices, planes for frustum culling
        // easily explained, planes: are the 6 planes of the frustum, which is the viewable area of the camera.
        // glGetFloatv is used to retrieve the current OpenGL matrices
        glGetFloatv(GL_PROJECTION_MATRIX, proj); 
        glGetFloatv(GL_MODELVIEW_MATRIX, modl);
        ExtractFrustumPlanes(planes, proj, modl); // extract the planes from the projection and modelview matrices
        drawModels(scene1, planes, camX, camY, camZ);
        /*
        // LOD if 20 or 25 meters away, dont draw at all
        float dx = objX - camX;
        float dy = objY - camY;
        float dz = objZ - camZ;
        float distance = sqrtf(dx * dx + dy * dy + dz * dz);
        


        // Check if cube is visible in the frustum
        float radius = 1.732f; // Radius of the sphere we want to check
        if(SphereInFrustum(planes, objX, objY, objZ, radius) && distance < 20.0f) {
        // This is to rotate, can be deleted (not necessary.)
        //glPushMatrix();
        //glRotatef(cubeAngle, 1, 1, 0);
        drawModels(scene1);
        //glPopMatrix();
        } else {
            if(distance > 20.0f) printf("Cube is too far away!");
            else printf("Cube is not visible!");
        }

        */
        





        SwapBuffers(g_hDC);
    }

    
    free(scene1.models);
    // Cleanup
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    DestroyWindow(g_hWnd);

    return 0;
}

// gcc opengl3.c -o opengl3.exe -lopengl32 -lglu32 -lgdi32 -mwindows -luser32
// actually working opengl 2.1 program.