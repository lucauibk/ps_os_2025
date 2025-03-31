#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

bool** allocateField(int width, int height) {
    //create field and allocate the rows
    bool** field = (bool**)malloc(height * sizeof(bool*));
    if (!field) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    //for each row allocate the columns
    for (int i = 0; i < height; i++) {
        field[i] = (bool*)malloc(width * sizeof(bool));
        if (!field[i]) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    return field;
}
//free the allocated memory
void freeField(bool** field, int height) {
    for (int i = 0; i < height; i++) {
        free(field[i]);
    }
    free(field);
}
//initalize the field with random values based on the density
void initField(bool** field, int width, int height, float density) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            field[i][j] = ((double)rand() / RAND_MAX) < density;
        }
    }
}
//create the pbm file for eacht step
void createPBM(bool** grid, int width, int height, int step) {
    char filename[20];
    snprintf(filename, sizeof(filename), "gol_%05d.pbm", step);
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        exit(1);
    }
    fprintf(file, "P1\n");
    fprintf(file, "# Step %d\n", step);
    fprintf(file, "%d %d\n", width, height);
    
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fprintf(file, "%d ", !grid[i][j]); // 0 = lebend (schwarz), 1 = tot (weiß)
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

//game of life rules
//count the neighbors of a cell
int countNeighbors(bool** field, int width, int height, int x, int y) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue; // Eigene Zelle ignorieren
            
            int x1 = x + i;
            int y1 = y + j;

            // Überprüfen, ob Nachbar innerhalb des Spielfelds liegt
            if (x1 >= 0 && x1 < height && y1 >= 0 && y1 < width) {
                if (field[x1][y1]) count++;
            }
        }
    }
    return count;
}
//if the cell is alive and has 2 or 3 neighbors it stays alive
//if the cell is dead and has 3 neighbors it becomes alive
void stepSimulation(bool** source, bool** destination, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int neighbors = countNeighbors(source, width, height, i, j);
            bool alive = source[i][j];

            if (alive) {
                destination[i][j] = (neighbors == 2 || neighbors == 3);
            } else {
                destination[i][j] = (neighbors == 3);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <width> <height> <density> <steps>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    float density = atof(argv[3]);
    int steps = atoi(argv[4]);

    printf("width:   %4d\n", width);
    printf("height:  %4d\n", height);
    printf("density: %4.0f%%\n", density * 100);
    printf("steps:   %4d\n", steps);

    bool** field1 = allocateField(width, height);
    bool** field2 = allocateField(width, height);

    bool** source = field1;
    bool** destination = field2;
    initField(source, width, height, density);

    for (int i = 0; i <= steps; i++) {
        createPBM(source, width, height, i);
        stepSimulation(source, destination, width, height);

        bool** temp = source;
        source = destination;
        destination = temp;
    }

    freeField(field1, height);
    freeField(field2, height);
    return 0;
}
