
// ------------------------------------------------- IMPORT LIBS -----------------------------------------------------

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")


GLuint fbo, fboTex, fboDepth;

// ------------------------------------------------- CAMERA POSITION AND ROTATION ----------------------------------



// Camera position and rotation
float camX = 0.0f, camY = 0.0f, camZ = 0.0f;
float camPitch = 0.0f, camYaw = 0.0f;

float objX = 0.0f, objY = 0.0f, objZ = 0.0f;



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
       if(planes[i][0]*x + planes[i][0]*y + planes[i][2]*z + planes[i][3] <= -radius)
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

    // Create window
    g_hWnd = CreateWindow(
        wc.lpszClassName,
        "OpenGL",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        700, 500,
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

        glViewport(0, 0, 700, 500);

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

        // LOD if 20 or 25 meters away, dont draw at all
        float dx = objX - camX;
        float dy = objY - camY;
        float dz = objZ - camZ;
        float distance = sqrtf(dx * dx + dy * dy + dz * dz);
        


        // Check if cube is visible in the frustum
        float radius = 1.732f; // Radius of the sphere we want to check
        if(SphereInFrustum(planes, objX, objY, objZ, radius) && distance < 20.0f) {
        glPushMatrix();
        glRotatef(cubeAngle, 1, 1, 0);
        DrawCube();
        glPopMatrix();
        } else {
            if(distance > 20.0f) printf("Cube is too far away!");
            else printf("Cube is not visible!");
        }
        





        SwapBuffers(g_hDC);
    }

    // Cleanup
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    DestroyWindow(g_hWnd);

    return 0;
}

// gcc opengl3.c -o opengl3.exe -lopengl32 -lglu32 -lgdi32 -mwindows -luser32
// actually working opengl 2.1 program.