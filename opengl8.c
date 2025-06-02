
/*
its mindblowing to think i only made this in a day!
and im even writing while im sick today!



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

/**
 * @note
 * ============================================================================
 *  NOTE: We can do very many and very lots of debug printing to troubleshoot
 *  (especially glitches, bugs, errors, etc).
 *  Examples: printing normals, printing if a function works, printing at
 *  specific points the application crashes, etc.
 * ============================================================================
 */

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

float camX = 0.0f, camY = 0.0f, camZ = 0.0f;
float camPitch = 0.0f, camYaw = 0.0f;

int lastMouseX = 0, lastMouseY = 0;
bool mouseCaptured = false;
bool keys[256] = { false };

HWND g_hWnd;
HDC g_hDC;
HGLRC g_hRC;


typedef struct {
    GLfloat x, y, z;
} Vec3D;

typedef struct {
    GLfloat r, g, b;
} Color;

typedef struct {
    GLfloat u, v;
} Vec2D;

typedef struct {
    Vec3D position;
    Vec3D* vertices;
    Vec3D* normals;
    Color* colors;
    GLuint* faces;
    GLuint numVertices;
    GLuint numColors;
    GLuint numFaces;
    Vec2D* texcoords;
    GLuint* faceTexcoords;
    GLuint numTexcoords;
    char name;
} Model;

typedef struct {
    Model* models;
    GLuint numModels;
} Scene;


void getVertexNormals(Model* model) {
    // Free previous normals if any
    if (model->normals) {
        free(model->normals);
        model->normals = NULL;
    }
    if (!model->vertices || !model->faces || model->numVertices == 0 || model->numFaces == 0)
        return;

    // Allocate and zero normals
    model->normals = (Vec3D*)calloc(model->numVertices, sizeof(Vec3D));
    if (!model->normals) return;

    // For each face, compute face normal and add to each vertex normal
    for (unsigned int i = 0; i < model->numFaces; i++) {
        GLuint i0 = model->faces[i * 3 + 0];
        GLuint i1 = model->faces[i * 3 + 1];
        GLuint i2 = model->faces[i * 3 + 2];

        Vec3D v0 = model->vertices[i0];
        Vec3D v1 = model->vertices[i1];
        Vec3D v2 = model->vertices[i2];

        // Compute face normal (cross product)
        Vec3D u = {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
        Vec3D v = {v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
        Vec3D normal = {
            u.y * v.z - u.z * v.y,
            u.z * v.x - u.x * v.z,
            u.x * v.y - u.y * v.x
        };

        // Normalize face normal
        float len = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
        if (len > 0.00001f) {
            normal.x /= len;
            normal.y /= len;
            normal.z /= len;
        }

        // Add face normal to each vertex normal
        model->normals[i0].x += normal.x;
        model->normals[i0].y += normal.y;
        model->normals[i0].z += normal.z;

        model->normals[i1].x += normal.x;
        model->normals[i1].y += normal.y;
        model->normals[i1].z += normal.z;

        model->normals[i2].x += normal.x;
        model->normals[i2].y += normal.y;
        model->normals[i2].z += normal.z;
    }

    // Normalize all vertex normals
    for (unsigned int i = 0; i < model->numVertices; i++) {
        float len = sqrtf(
            model->normals[i].x * model->normals[i].x +
            model->normals[i].y * model->normals[i].y +
            model->normals[i].z * model->normals[i].z
        );
        if (len > 0.00001f) {
            model->normals[i].x /= len;
            model->normals[i].y /= len;
            model->normals[i].z /= len;
        } else {
            // If degenerate, set to up
            model->normals[i].x = 0.0f;
            model->normals[i].y = 1.0f;
            model->normals[i].z = 0.0f;
        }
    }
}


Color vaporPalette[] = {
    {0.95f, 0.65f, 0.85f}, // Soft Pink
    {0.65f, 0.85f, 0.95f}, // Soft Cyan
    {0.85f, 0.95f, 0.65f}, // Soft Mint
    {0.95f, 0.90f, 0.65f}, // Soft Peach
    {0.85f, 0.65f, 0.95f}, // Soft Purple
    {0.65f, 0.95f, 0.85f}, // Soft Aqua
    {0.95f, 0.65f, 0.65f}, // Soft Light Red
    {0.98f, 0.98f, 0.98f}, // White
};
// ORIGINAL VAPOR
/*Color vaporPalette[] = {
    {0.98f, 0.53f, 0.80f}, // Pink
    {0.53f, 0.80f, 0.98f}, // Cyan
    {0.80f, 0.98f, 0.53f}, // Mint
    {0.98f, 0.80f, 0.53f}, // Peach
    {0.80f, 0.53f, 0.98f}, // Purple
    {0.53f, 0.98f, 0.80f}, // Aqua
    {0.98f, 0.53f, 0.53f}, // Light Red
    {0.98f, 0.98f, 0.98f}, // White
};*/

/*
MODIFIED VAPOR
Color vaporPalette[] = {
    {0.92f, 0.60f, 0.78f}, // Muted Pink
    {0.60f, 0.78f, 0.92f}, // Muted Cyan
    {0.78f, 0.92f, 0.60f}, // Muted Mint
    {0.92f, 0.86f, 0.60f}, // Muted Peach
    {0.78f, 0.60f, 0.92f}, // Muted Purple
    {0.60f, 0.92f, 0.78f}, // Muted Aqua
    {0.92f, 0.60f, 0.60f}, // Muted Light Red
    {0.96f, 0.96f, 0.96f}, // Soft White
};


*/

// static Model cubeModel, barrelModel;
static Scene scene1;


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

float clamp(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}



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
    case WM_MOUSEMOVE: {
        if(!mouseCaptured) break;
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hwnd, &pt);
        int x = pt.x;
        int y = pt.y;

        RECT rect;
        GetClientRect(hwnd, &rect);
        int centerX = (rect.right - rect.left)/2;
        int centerY = (rect.bottom - rect.top)/2;
        
        
        int dx = x - centerX;
        int dy = y - centerY;

        if(dx != 0 || dy != 0) {
            camYaw -= dx * 0.3f; // Adjust sensitivity
            camPitch -= dy * 0.3f;
            camPitch = clamp(camPitch, -89.0f, 89.0f);

            // Recenter the cursor
            POINT ptScreen = {centerX, centerY};
            ClientToScreen(hwnd, &ptScreen);
            SetCursorPos(ptScreen.x, ptScreen.y);
        }
        return 0;
    }
    case WM_KEYDOWN:
        keys[wParam] = true;
        if (wParam == VK_ESCAPE)
            PostQuitMessage(0);
        if (wParam == 'Q') {
            mouseCaptured = !mouseCaptured;
            if (mouseCaptured) {
                ShowCursor(FALSE);
                // Center the cursor
                RECT rect;
                GetClientRect(hwnd, &rect);
                POINT pt = { (rect.right - rect.left)/2,
                             (rect.bottom - rect.top)/2 };
                ClientToScreen(hwnd, &pt);
                SetCursorPos(pt.x, pt.y);
                lastMouseX = pt.x;
                lastMouseY = pt.y;
            } else {
                ShowCursor(TRUE);
            }
        }
        return 0;
    case WM_KEYUP:
        keys[wParam] = false;
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }


}
   // --- OBJ Loader ---
bool loadOBJ(const char* filename, Vec3D** outVertices, GLuint** outFaces, Vec2D** outTexcoords, GLuint** outFaceTexcoords, int* outNumTexcoords, int* outNumVertices, int* outNumFaces) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;

    Vec3D* tempVertices = malloc(MAX_VERTICES * sizeof(Vec3D));
    Vec2D* tempTexcoords = malloc(MAX_VERTICES * sizeof(Vec2D));
    GLuint* faces = malloc(MAX_FACES * 3 * sizeof(GLuint));
    GLuint* faceTexcoords = malloc(MAX_FACES * 3 * sizeof(GLuint));
    int vcount = 0, vtcount = 0, fcount = 0;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            tempVertices[vcount++] = (Vec3D){x, y, z};
        } else if (line[0] == 'v' && line[1] == 't') {
            float u, v;
            sscanf(line, "vt %f %f", &u, &v);
            tempTexcoords[vtcount++] = (Vec2D){u, v};
        } else if (line[0] == 'f') {
            int v[4] = {0}, vt[4] = {0}, vn[4] = {0};
            int n = 0;
            char* ptr = line + 1;
            while (*ptr && n < 4) {
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                if (*ptr == 0 || *ptr == '\n') break;
                int matches = sscanf(ptr, "%d/%d/%d", &v[n], &vt[n], &vn[n]);
                if (matches == 3) {
                    // v/vt/vn
                } else if ((matches = sscanf(ptr, "%d/%d", &v[n], &vt[n])) == 2) {
                    // v/vt
                } else if ((matches = sscanf(ptr, "%d", &v[n])) == 1) {
                    vt[n] = 0;
                }
                n++;
                // skip to next space
                while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n') ptr++;
            }
            if (n == 3) {
                for (int i = 0; i < 3; i++) {
                    faces[fcount*3 + i] = v[i] - 1;
                    faceTexcoords[fcount*3 + i] = vt[i] - 1;
                }
                fcount++;
            } else if (n == 4) {
                // quad as two triangles
                int idx[6] = {0, 1, 2, 0, 2, 3};
                for (int t = 0; t < 2; t++) {
                    for (int i = 0; i < 3; i++) {
                        int j = idx[t*3 + i];
                        faces[fcount*3 + i] = v[j] - 1;
                        faceTexcoords[fcount*3 + i] = vt[j] - 1;
                    }
                    fcount++;
                }
            }
        }
    }
    fclose(file);

    *outVertices = tempVertices;
    *outTexcoords = tempTexcoords;
    *outFaces = faces;
    *outFaceTexcoords = faceTexcoords;
    *outNumVertices = vcount;
    *outNumTexcoords = vtcount;
    *outNumFaces = fcount;
    return true;
}

Model loadModel(const char* filename) {
    Vec3D* verts = NULL;
    Vec2D* texcoords = NULL;
    GLuint* faces = NULL;
    GLuint* faceTexcoords = NULL;
    int vcount = 0, vtcount = 0,fcount = 0;
    Model model = {0};
    if(loadOBJ(filename, &verts, &faces, &texcoords, &faceTexcoords, &vtcount, &vcount, &fcount)) {
        // Compute center of geometry
        Vec3D center = {0,0,0};
        for(int i=0;i<vcount;i++) {
            center.x += verts[i].x;
            center.y += verts[i].y;
            center.z += verts[i].z;
        }
        if(vcount > 0) {
            center.x /= vcount;
            center.y /= vcount;
            center.z /= vcount;
        }
        model.position = center;

        model.vertices = verts;
        model.faces    = faces;
        model.numVertices = vcount;
        model.numFaces    = fcount;
        model.texcoords   = texcoords;
        model.faceTexcoords = faceTexcoords;
        model.numTexcoords  = vtcount;

        printf("Loaded %d vertices, %d texcoords, %d faces\n", model.numVertices, model.numTexcoords, model.numFaces);
for (int i = 0; i < 5 && i < model.numTexcoords; i++) {
    printf("vt[%d] = (%.3f, %.3f)\n", i, model.texcoords[i].u, model.texcoords[i].v);
}
for (int i = 0; i < 5 && i < model.numFaces*3; i++) {
    printf("faceTexcoords[%d] = %d\n", i, model.faceTexcoords[i]);
}

        // Assign random color per vertex
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



void drawModels(Scene scene, float planes[6][4], float camX, float camY, float camZ) {
    glDisable(GL_CULL_FACE);
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
        if(SphereInFrustum(planes, model.position.x, model.position.y, model.position.z, radius) && distance < 30.0f) {
        // For the model position
        glPushMatrix(); // Save the current matrix
      
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        //printf("Debug: model.normals[%i] = (%.2f, %.2f, %.2f)\n",i,
        //model.normals[i].x, model.normals[i].y, model.normals[i].z);
        glNormalPointer(GL_FLOAT, sizeof(Vec3D), model.normals);
        glVertexPointer(3, GL_FLOAT, sizeof(Vec3D), model.vertices);
        glColorPointer(3, GL_FLOAT, sizeof(Color), model.colors);
        

        glDrawElements(GL_TRIANGLES, model.numFaces*3, GL_UNSIGNED_INT, model.faces);
        
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);


//         glDisable(GL_LIGHTING);
//  glBegin(GL_LINES);
//  for (int v = 0; v < model.numVertices; v++) {
//      glColor3f(1,0,0); // Red for normals
//      glVertex3f(model.vertices[v].x, model.vertices[v].y, model.vertices[v].z);
//      glVertex3f(
//          model.vertices[v].x + model.normals[v].x * 0.2f,
//          model.vertices[v].y + model.normals[v].y * 0.2f,
//          model.vertices[v].z + model.normals[v].z * 0.2f
//      );
//  }
//  glEnd();
//  glEnable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glPopMatrix(); // Restore the previous matrix.
        } else {
            if(distance > 30.0f) printf("\r(F) Model %d: is too far away!\n", i);
            else printf("\r(N) Model, SphereRadius: %d: is not visible!", i); printf("%f, calculated bounding sphere radius of the current model.", calculateBoundingSphereRadius(model.vertices, model.numVertices));
            fflush(stdout);
        }
        
    }
}

// --------------------------------------- TEST (DEMO) -------------------------------------------------------------


// // Draw cube function (like Benny's cube, using immediate mode)
// void DrawCube() {
    
//    // -------------- VERTEX ARRAY MODE ------------------
//    // ----------------- DRAW USING TRIANGLES ------------------
   
   
//    // glEnableClientState makes OpenGL
//    // use vertex arrays instead of
//    // immediate mode, which is more 
//    // efficient for rendering.
//    glEnableClientState(GL_VERTEX_ARRAY);
//    glVertexPointer(3, GL_FLOAT, 0, vertices);
   
//    for(int i=0;i<12;i++) {
//     glColor3fv(tri_colors[i]); // Set color for the current face.
//     // Easily explained:
//     // glVertexPointer specifies the location
//     // and data format of the array of vertex coordinates.
//     // glDrawElements draws the specified primitives
//     // using the vertex array and the indices
//     // meaning it will draw the vertices
//     // specified in the faces array.
//     glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, tri_faces[i]);
//    }

//    // Disable vertex array state after drawing
//    // to avoid affecting other rendering operations.
   
//     glDisableClientState(GL_VERTEX_ARRAY);
    
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
//}
// -------------------------------------------------- ACTUAL RPOGRAM -------------------------------------------------------------


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
        "OpenGL - Press Q to stop/activate Mouse Capturing",
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

    
    // Enable depth test and smooth shading
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    

    // lighing! 
     for (int i = 0; i < scene1.numModels; i++) {
     getVertexNormals(&scene1.models[i]);
     }

    // --------------------------------------- MODELS AND SCENERY - what's here?? --------------------------
    srand((unsigned int)time(NULL));
        
    
//      Model cubeModel, barrelModel;
//      cubeModel.position = (Vec3D){0.0f, 0.0f, 0.0f};
//      cubeModel.vertices = cubeVertices; 
// //   cubeModel.colors = cubeColors;
//      cubeModel.faces = cubeFaces;
//      cubeModel.numVertices = 8;
// //   cubeModel.numColors = 8;
//      cubeModel.numFaces = 12;
//      cubeModel.name = 'C';
  
//     barrelModel.position = (Vec3D){5.0f, 0.0f, 0.0f};
//     barrelModel.numVertices = 32;
// //  barrelModel.numColors = 32;
//     barrelModel.numFaces = 32;
//     barrelModel.name = 'B';
    
// barrelModel.vertices = (Vec3D[]) {
//     // Top circle
//     {0.5, 1, 0},
//     {0.46, 1, 0.19},
//     {0.35, 1, 0.35},
//     {0.19, 1, 0.46},
//     {0, 1, 0.5},
//     {-0.19, 1, 0.46},
//     {-0.35, 1, 0.35},
//     {-0.46, 1, 0.19},
//     {-0.5, 1, 0},
//     {-0.46, 1, -0.19},
//     {-0.35, 1, -0.35},
//     {-0.19, 1, -0.46},
//     {0, 1, -0.5},
//     {0.19, 1, -0.46},
//     {0.35, 1, -0.35},
//     {0.46, 1, -0.19},//after this Bottom circle now
//     {0.5, -1, 0},
//     {0.46, -1, 0.19},
//     {0.35, -1, 0.35},
//     {0.19, -1, 0.46},
//     {0, -1, 0.5},
//     {-0.19, -1, 0.46},
//     {-0.35, -1, 0.35},
//     {-0.46, -1, 0.19},
//     {-0.5, -1, 0},
//     {-0.46, -1, -0.19},
//     {-0.35, -1, -0.35},
//     {-0.19, -1, -0.46},
//     {0, -1, -0.5},
//     {0.19, -1, -0.46},
//     {0.35, -1, -0.35},
//     {0.46, -1, -0.19},
// };

// barrelModel.faces = (GLuint[]) {
//     // Sides
//     0, 16, 1, 
//     1, 16, 17,
//     1, 17, 2,
//     2, 17, 18,
//     2, 18, 3,
//     3, 18, 19,
//     3, 19, 4,
//     4, 19, 20,
//     4, 20, 5,
//     5, 20, 21,
//     5, 21, 6,
//     6, 21, 22,
//     6, 22, 7,
//     7, 22, 23,
//     7, 23, 8,
//     8, 23, 24,
//     8, 24, 9,
//     9, 24, 25,
//     9, 25, 10,
//     10, 25, 26,
//     10, 26, 11,
//     11, 26, 27,
//     11, 27, 12,
//     12, 27, 28,
//     12, 28, 13,
//     13, 28, 29,
//     13, 29, 14,
//     14, 29, 30,
//     14, 30, 15,
//     15, 30, 31,
//     15, 31, 0,
//     0, 31, 16,
// };

// // barrelModel.colors = (Color[]) {
// //     {0.55, 0.27, 0.07}, // Brown
// //     {0.55, 0.27, 0.07},
// //     {0.55, 0.27, 0.07},
// //     {0.55, 0.27, 0.07},
// //     {0.55, 0.27, 0.07},
// //     {0.55, 0.27, 0.07},
// //     {0.55, 0.27, 0.07},
// //     {0.55, 0.27, 0.07},
// //     {0.55, 0.27, 0.07},
// //     {0.55, 0.27, 0.07},
// //     {0.55, 0.27, 0.07},
// //     {0.55, 0.27, 0.07},
// //     {0.55, 0.27, 0.07}
// // };

     Model city_1 = loadModel("models1/cell_4_0_0.obj");
     Model city_2 = loadModel("models1/cell_5_0_0.obj");
     Model city_3 = loadModel("models1/cell_3_0_0.obj");
     Model city_4 = loadModel("models1/cell_2_0_0.obj");
     Model city_5 = loadModel("models1/cell_1_0_0.obj");
     Model city_6 = loadModel("models1/cell_0_0_0.obj");





    scene1.models = malloc(6 * sizeof(Model)); //?
    //**********##### SPECIAL: scene1.models[0] = cubeModel;
   //  scene1.models[1] = barrelModel;
    //  scene1.models[2] = city_1;
    //  scene1.models[3] = city_2;
    //  scene1.models[4] = city_3;
    //  scene1.models[5] = city_4;
    //  scene1.models[6] = city_5;
    //  scene1.models[7] = city_6;

    //   scene1.models[1] = city_1;
    //  scene1.models[2] = city_2;
    //  scene1.models[3] = city_3;
    //  scene1.models[4] = city_4;
    //  scene1.models[5] = city_5;
    //  scene1.models[6] = city_6;
       scene1.models[0] = city_1;
     scene1.models[1] = city_2;
     scene1.models[2] = city_3;
     scene1.models[3] = city_4;
     scene1.models[4] = city_5;
     scene1.models[5] = city_6;
     
     scene1.numModels = 6;

     // After scene1.models[0..7] and scene1.numModels = 8;
int paletteSize = sizeof(vaporPalette) / sizeof(vaporPalette[0]);
for (int i = 0; i < scene1.numModels; i++) {
    Model* m = &scene1.models[i];
    if (m->colors && m->numColors > 0) {
        free(m->colors);
    }
    m->colors = malloc(m->numVertices * sizeof(Color));
    m->numColors = m->numVertices;
    for (int v = 0; v < m->numVertices; v++) {
        m->colors[v] = vaporPalette[v % paletteSize];
    }
}


    for (int i = 0; i < scene1.numModels; i++) {
    getVertexNormals(&scene1.models[i]);
}

    

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

        if (m.normals && m.numVertices > 0) {
            printf("Model %d pos: (%.2f, %.2f, %.2f), dist: %.2f, normal[0]: (%.2f, %.2f, %.2f)\n",
                i, m.position.x, m.position.y, m.position.z, dist,
                m.normals[0].x, m.normals[0].y, m.normals[0].z);
        } else {
            printf("Model %d pos: (%.2f, %.2f, %.2f), dist: %.2f, normals missing!\n",
                i, m.position.x, m.position.y, m.position.z, dist);
        }
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
        if (keys['D']) {
            camX += rightX * moveSpeed;
            camZ += rightZ * moveSpeed;
        }
        if (keys['A']) {
            camX -= rightX * moveSpeed;
            camZ -= rightZ * moveSpeed;
        }

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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


                  /**
                   * 
                   * THIEF 3 PROFILE:
                   * // Thief 3: Moody, blue, stealthy
GLfloat ambientColor[] = {0.13f, 0.15f, 0.22f, 1.0f};
GLfloat lightDir[]     = {0.2f, 1.0f, 0.5f, 0.0f};
GLfloat lightColor[]   = {0.55f, 0.65f, 1.0f, 1.0f};
GLfloat matDiffuse[]   = {0.85f, 0.90f, 1.0f, 1.0f};

glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
glEnable(GL_COLOR_MATERIAL);
glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

glClearColor(0.10f, 0.13f, 0.22f, 1.0f);
glEnable(GL_FOG);
GLfloat fogColor[4] = {0.10f, 0.13f, 0.22f, 1.0f};
glFogfv(GL_FOG_COLOR, fogColor);
glFogf(GL_FOG_START, 12.0f);
glFogf(GL_FOG_END, 32.0f);
glFogi(GL_FOG_MODE, GL_LINEAR);
                   * 
                   * 
                   */
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
         GLfloat ambientColor[] = {0.98f, 0.80f, 0.55f, 1.0f}; // Original warm sunset ambient (for testing)
       //GLfloat ambientColor[] ={0.68f, 0.76f, 0.95f, 1.0f};// Cool, dark blue ambient
       GLfloat lightDir[]     = {0.3f, 1.0f, 0.7f, 1.0f};  // Moonlight from above/side
       GLfloat lightColor[]   ={1.0f, 0.92f, 0.70f, 1.0f};  // Soft blue moonlight
       GLfloat matDiffuse[]   ={1.0f, 0.98f, 0.90f, 1.0f}; // Slightly blue-white for clarity
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
        glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);

        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);

        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

        /*
==============================================================================
  FOG DENSITY & CULLING DISTANCE LOOKUP TABLE (GL_EXP2)
  Use this to match your fog density to a good LOD/culling distance!
==============================================================================
| Fog Density (GL_EXP2) | "Fully Fogged" Distance | Recommended Cull/LOD Distance |
|-----------------------|-------------------------|-------------------------------|
| 0.02                  | ~75                     | 65–70                         |
| 0.03                  | ~50                     | 43–47                         |
| 0.04                  | ~37                     | 32–35                         |
| 0.05                  | ~30                     | 25–28                         |
| 0.06                  | ~25                     | 20–23                         |
| 0.08                  | ~18                     | 14–16                         |
| 0.10                  | ~15                     | 11–13                         |
| 0.12                  | ~12                     | 9–11                          |

- "Fully Fogged" Distance: where objects are basically invisible.
- Cull/LOD Distance: where you should stop drawing objects (just before they're fully hidden).
- For GL_LINEAR, use your GL_FOG_END value minus a little (e.g. GL_FOG_END - 2.0f).
==============================================================================
*/

        // FOG and background
        
        // (0.98f, 0.85f, 0.60f, 1.0f) Sunset Color
        /**
         * NOTE: wow, I SWEAR
         * Profile Color name: Sunset AAA
         * sky color: (1.0f, 0.97f, 0.82f, 1.0f);
         * fog color: {1.0f, 0.97f, 0.82f, 1.0f}; 
         * 
         * with lightings settings
         *     GLfloat ambientColor[] = {0.98f, 0.80f, 0.55f, 1.0f}; // Original warm sunset ambient (for testing)
     GLfloat lightDir[]     = {0.3f, 1.0f, 0.7f, 1.0f};  // Moonlight from above/side
       GLfloat lightColor[]   ={1.0f, 0.92f, 0.70f, 1.0f};  // Soft blue moonlight
       GLfloat matDiffuse[]   ={1.0f, 0.98f, 0.90f, 1.0f}; /


       is sooo good, its like AAA game now
         */
        //(0.78f, 0.88f, 0.98f, 1.0f); Sky blue color, good when paired with the previous setting
        glClearColor(1.0f, 0.98f, 0.86f, 1.0f);
        glEnable(GL_FOG); // 1. Turn on fog

        // {0.38f, 0.32f, 0.45f, 1.0f} is very good AAA styled
        // {0.98f, 0.85f, 0.60f, 1.0f}; sunsetty, orange AAA styled (Sunset Palette) 
        // {1.0f, 0.93f, 0.72f, 1.0f}; (Sunset Palette) still the same just testing (edited a bit)
        // {1.0f, 0.97f, 0.82f, 1.0f}; (Sunset Palette) AAA styled, fit with the Color profile earlier
        //                                              dark-souls 3 styled and orange sungold ultra-bright aesthetic
GLfloat fogColor[4] =  {1.0f, 0.97f, 0.82f, 1.0f};  // 2. Fog color (matches your sky)
glFogi(GL_FOG_MODE, GL_EXP2);
glFogf(GL_FOG_DENSITY, 0.06f);

glFogfv(GL_FOG_COLOR, fogColor);

//10 and 40 original, very good
//glFogf(GL_FOG_START, 10.0f);   // 3. Fog starts at 10 units from camera
//glFogf(GL_FOG_END, 40.0f);    // 4. Fully fogged at 40 units

//glFogi(GL_FOG_MODE, GL_LINEAR); // 5. Linear blend between start and end




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

// gcc opengl8.c -o opengl8.exe -lopengl32 -lglu32 -lgdi32 -mwindows -luser32
// actually working opengl 2.1 program.