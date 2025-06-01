#include <stdio.h>
#include <stdlib.h>

#define MAX_VERTICES 1000000
#define MAX_FACES 2000000
#define NX 6
#define NY 1
#define NZ 1

/*
TESTED GOOD:
NX 3
NY 1
NZ 1


NX 2
NY 1
NZ 1


this is good, (5 or 6% missing parts), but i think
needs fog LOD like in morrowind so that it hides the missing part
still looks very good from far!
NX 6
NY 1
NZ 1

fix for missing parts:
ohh ok so make it more and more fully inclusive right to all vertices and faces touching no matter what
*/


typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    int v[4];
    int n;
} Face;


Face faces[MAX_FACES];
int numFaces = 0;

Vertex vertices[MAX_VERTICES];
int numVertices = 0;

// Clamp function to keep indices in range
int clamp(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

// Loads only vertex positions from OBJ file
void loadOBJ(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Cannot open file: %s\n", filename);
        exit(1);
    }
    char line[512];
    printf("Reading lines from file...\n");
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            float x, y, z;
            if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
                vertices[numVertices].x = x;
                vertices[numVertices].y = y;
                vertices[numVertices].z = z;
                numVertices++;
            }
        }
        else if(line[0] == 'f' && (line[1] == ' ' || line[1] == 't')) {
            int vi[4], n = 0;
            char* ptr = line + 1;
            while(*ptr && n < 4) {
                while(* ptr == ' ' || *ptr == 't') ptr++;
                if(*ptr == 0 || *ptr == '\n') break;
                int idx;
                if(sscanf(ptr, "%d", &idx) == 1) {
                    faces[numFaces].v[n++] = idx - 1; // OBJ indices start at 1
                }

                // skip to next space or end
                while(*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n') ptr++;

            }
            faces[numFaces].n = n;
            numFaces++;
        }
    }
    fclose(file);
    printf("Loaded %d vertices.\n", numVertices);
}
void export_grid_cells_as_obj(
    float min_x, float max_x, float min_y, float max_y, float min_z, float max_z,
    const char* filename_pattern // e.g., "cell_%d_%d_%d.obj"
) {
    float cellsize_x = (max_x - min_x) / NX;
    float cellsize_y = (max_y - min_y) / NY;
    float cellsize_z = (max_z - min_z) / NZ;

    int* vert_cell = malloc(sizeof(int) * numVertices);
    for (int i = 0; i < numVertices; i++) {
        int cell_x = clamp((int)((vertices[i].x - min_x) / cellsize_x), 0, NX-1);
        int cell_y = clamp((int)((vertices[i].y - min_y) / cellsize_y), 0, NY-1);
        int cell_z = clamp((int)((vertices[i].z - min_z) / cellsize_z), 0, NZ-1);
        vert_cell[i] = (cell_x << 16) | (cell_y << 8) | cell_z;
    }

    for (int cx = 0; cx < NX; cx++) {
        for (int cy = 0; cy < NY; cy++) {
            for (int cz = 0; cz < NZ; cz++) {
                char fname[256];
                snprintf(fname, sizeof(fname), filename_pattern, cx, cy, cz);
                FILE* f = fopen(fname, "w");
                if (!f) continue;

                // int* vmap = malloc(sizeof(int) * numVertices);
                // int vcount = 0;
                // int cell_id = (cx << 16) | (cy << 8) | cz;

                /*FIX BSP missing part:
                  to make fully inclusive,
                  Great question! Here’s what you should change first in your code to make your BSP splitter fully inclusive (no missing parts):

1. Change How You Write Vertices and Faces
Right now:

You only write a vertex if it is assigned to the current cell.
You only write a face if all its vertices are in the current cell.
You need to:

Write a face if any of its vertices are in the current cell.
For each such face, write all its vertices (even if some are outside the cell).
2. How to Do This
A. Build a vertex map for each cell:
For each face, if any vertex is in the cell, mark all its vertices as needed for this cell.
B. Write all needed vertices for the cell.
Assign new indices for the OBJ file.
C. Write all faces that touch the cell, using the new indices.
3. What to Change in Your Code
Replace your current vertex/face writing logic in export_grid_cells_as_obj with this:

                */


                int* vmap = malloc(sizeof(int) * numVertices);
                int vcount = 0;
                int cell_id = (cx << 16) | (cy << 8) | cz;

                /// Step 1: Mark all vertices needed for this cell
                for (int i = 0; i < numVertices; i++) vmap[i] = 0;
                for (int i = 0; i < numFaces; i++) {
                     int used_in_cell = 0;
                     for (int j = 0; j < faces[i].n; j++) {
                           if (vert_cell[faces[i].v[j]] == cell_id) {
                               used_in_cell = 1;
                               break;
                               }
                           }
                           if (used_in_cell) {
                              for (int j = 0; j < faces[i].n; j++) {
                                   int v = faces[i].v[j];
                                   if (vmap[v] == 0) {
                                   vmap[v] = 1; // Mark as needed
                                   }
                                }
                            }
                         }

                         vcount = 0;
                         for (int i = 0; i < numVertices; i++) {
                              if (vmap[i]) {
                                  vmap[i] = ++vcount; // Assign new OBJ index
                                  fprintf(f, "v %f %f %f\n", vertices[i].x, vertices[i].y, vertices[i].z);
                                  }
                        }

                // Step 2: Write faces (any face with at least one vertex in this cell)

        int fcount = 0;
        for (int i = 0; i < numFaces; i++) {
            int used_in_cell = 0;
            for (int j = 0; j < faces[i].n; j++) {
                if (vert_cell[faces[i].v[j]] == cell_id) {
                    used_in_cell = 1;
                    break;
                }
            }
            if (used_in_cell) {
                fprintf(f, "f");
                for (int j = 0; j < faces[i].n; j++)
                     fprintf(f, " %d", vmap[faces[i].v[j]]);
                fprintf(f, "\n");
                fcount++;
            }
        }
                /*
                // Write vertices and build vmap
                for (int i = 0; i < numVertices; i++) {
                    if (vert_cell[i] == cell_id) {
                        vmap[i] = ++vcount;
                        fprintf(f, "v %f %f %f\n", vertices[i].x, vertices[i].y, vertices[i].z);
                    } else {
                        vmap[i] = 0;
                    }
                }

                int fcount = 0;
                // Write faces
                for (int i = 0; i < numFaces; i++) {
                    int in_cell = 1;
                    for (int j = 0; j < faces[i].n; j++) {
                        if (vert_cell[faces[i].v[j]] != cell_id) {
                            in_cell = 0;
                            break;
                        }
                    }
                    if (in_cell) {
                        fprintf(f, "f");
                        for (int j = 0; j < faces[i].n; j++)
                            fprintf(f, " %d", vmap[faces[i].v[j]]);
                        fprintf(f, "\n");
                        fcount++;
                    }
                }*/
                fclose(f);
                if (vcount > 0)
                    printf("Exported %d vertices and %d faces to %s\n", vcount, fcount, fname);
                free(vmap);
            }
        }
    }
    free(vert_cell);
}


// Computes the bounding box of all vertices
void computeBoundingBox(float* min_x, float* max_x, float* min_y, float* max_y, float* min_z, float* max_z) {
    if (numVertices == 0) return;
    *min_x = *max_x = vertices[0].x;
    *min_y = *max_y = vertices[0].y;
    *min_z = *max_z = vertices[0].z;
    for (int i = 1; i < numVertices; i++) {
        if (vertices[i].x < *min_x) *min_x = vertices[i].x;
        if (vertices[i].x > *max_x) *max_x = vertices[i].x;
        if (vertices[i].y < *min_y) *min_y = vertices[i].y;
        if (vertices[i].y > *max_y) *max_y = vertices[i].y;
        if (vertices[i].z < *min_z) *min_z = vertices[i].z;
        if (vertices[i].z > *max_z) *max_z = vertices[i].z;
    }
}

// Assigns each vertex to a grid cell and prints selected vertices for variety
void grid_partition(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z) {
    float cellsize_x = (max_x - min_x) / NX;
    float cellsize_y = (max_y - min_y) / NY;
    float cellsize_z = (max_z - min_z) / NZ;

    printf("Grid cell size: X=%f, Y=%f, Z=%f\n", cellsize_x, cellsize_y, cellsize_z);

    // Print the cell index for the first 10, middle, and last vertex
    int indices[12] = {0, 71774, 143548, 215322, 287096, 358870, 430644, 502418, 574192, numVertices/2, numVertices-1};
    for (int j = 0; j < 12; j++) {
        int i = indices[j];
        if (i < 0 || i >= numVertices) continue;
        int cell_x = clamp((int)((vertices[i].x - min_x) / cellsize_x), 0, NX - 1);
        int cell_y = clamp((int)((vertices[i].y - min_y) / cellsize_y), 0, NY - 1);
        int cell_z = clamp((int)((vertices[i].z - min_z) / cellsize_z), 0, NZ - 1);

        printf("Vertex %d: (%.3f, %.3f, %.3f) -> Cell(%d,%d,%d)\n",
            i, vertices[i].x, vertices[i].y, vertices[i].z, cell_x, cell_y, cell_z);
    }
}

int main() {
    printf("Program started\n");
    loadOBJ("city2.obj"); // Make sure this file exists in the same folder

    float min_x, max_x, min_y, max_y, min_z, max_z;
    computeBoundingBox(&min_x, &max_x, &min_y, &max_y, &min_z, &max_z);

    if (numVertices > 0) {
        printf("Bounding box:\n");
        printf("  X: %f to %f\n", min_x, max_x);
        printf("  Y: %f to %f\n", min_y, max_y);
        printf("  Z: %f to %f\n", min_z, max_z);

        grid_partition(min_x, max_x, min_y, max_y, min_z, max_z);
        export_grid_cells_as_obj(min_x, max_x, min_y, max_y, min_z, max_z, "output1/cell_%d_%d_%d.obj");
printf("Exported grid cells as OBJ files.\n");
    } else {
        printf("No vertices loaded, cannot compute bounding box.\n");
    }

    printf("Press Enter to exit...\n");
    getchar();
    return 0;
}