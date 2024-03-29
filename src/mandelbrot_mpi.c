#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "mpi.h"

#define MASTER 0


double c_x_min;
double c_x_max;
double c_y_min;
double c_y_max;

double pixel_width;
double pixel_height;

int iteration_max = 200;

int image_size;
unsigned char **image_buffer;
unsigned char **image_buffer2;

int i_x_max;
int i_y_max;
int image_buffer_size;
int n_cores;

struct sub_image {
    int init_x;
    int final_x;
    int init_y;
    int final_y;
};

int gradient_size = 16;
int colors[17][3] = {
                        {66, 30, 15},
                        {25, 7, 26},
                        {9, 1, 47},
                        {4, 4, 73},
                        {0, 7, 100},
                        {12, 44, 138},
                        {24, 82, 177},
                        {57, 125, 209},
                        {134, 181, 229},
                        {211, 236, 248},
                        {241, 233, 191},
                        {248, 201, 95},
                        {255, 170, 0},
                        {204, 128, 0},
                        {153, 87, 0},
                        {106, 52, 3},
                        {16, 16, 16},
                    };

void allocate_image_buffer(){
    int rgb_size = 3;
    image_buffer = (unsigned char **) malloc(sizeof(unsigned char *) * image_buffer_size);
    image_buffer2 = (unsigned char **) malloc(sizeof(unsigned char *) * image_buffer_size * 2);

    for(int i = 0; i < image_buffer_size; i++){
        image_buffer[i] = (unsigned char *) malloc(sizeof(unsigned char) * rgb_size);
    };

    for(int i = 0; i < image_buffer_size * 2; i++){
        image_buffer2[i] = (unsigned char *) malloc(sizeof(unsigned char) * rgb_size);
    };
};

void init(int argc, char *argv[]){
    if(argc < 6){
        printf("usage: ./mandelbrot_mpi c_x_min c_x_max c_y_min c_y_max image_size\n");
        printf("examples with image_size = 11500:\n");
        printf("    Full Picture:         ./mandelbrot_mpi -2.5 1.5 -2.0 2.0 11500\n");
        printf("    Seahorse Valley:      ./mandelbrot_mpi -0.8 -0.7 0.05 0.15 11500\n");
        printf("    Elephant Valley:      ./mandelbrot_mpi 0.175 0.375 -0.1 0.1 11500\n");
        printf("    Triple Spiral Valley: ./mandelbrot_mpi -0.188 -0.012 0.554 0.754 11500\n");
        exit(0);
    }
    else{        
        sscanf(argv[1], "%lf", &c_x_min);
        sscanf(argv[2], "%lf", &c_x_max);
        sscanf(argv[3], "%lf", &c_y_min);
        sscanf(argv[4], "%lf", &c_y_max);
        sscanf(argv[5], "%d", &image_size);

        i_x_max           = image_size;
        i_y_max           = image_size;
        image_buffer_size = image_size * image_size;

        pixel_width       = (c_x_max - c_x_min) / i_x_max;
        pixel_height      = (c_y_max - c_y_min) / i_y_max;
    };
};

void update_rgb_buffer(int iteration, int x, int y){
    int color;

    if(iteration == iteration_max){
        image_buffer[(i_y_max * y) + x][0] = colors[gradient_size][0];
        image_buffer[(i_y_max * y) + x][1] = colors[gradient_size][0];
        image_buffer[(i_y_max * y) + x][2] = colors[gradient_size][0];
    }
    else{
        color = iteration % gradient_size;

        image_buffer[(i_y_max * y) + x][0] = colors[color][0];
        image_buffer[(i_y_max * y) + x][1] = colors[color][1];
        image_buffer[(i_y_max * y) + x][2] = colors[color][2];
    };
};

void write_to_file(int taskid){
    FILE * file;
    char * filename = "output.ppm";
    char header[50];
    char * comment = "# ";
    int max_color_component_value = 255;
    MPI_File fh;


    if( access(filename, F_OK ) != -1 ) {
        // file exists
        printf("deletou o arquivo.\n");
        remove(filename);
    } 
    

    MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    sprintf(header, "P6\n %s\n %d\n %d\n %d\n",
            comment, i_x_max, i_y_max, max_color_component_value);

    //fprintf(file, "P6\n %s\n %d\n %d\n %d\n",
    //        comment, i_x_max, i_y_max, max_color_component_value);
    
    if (taskid == MASTER) {
        MPI_File_write(fh, header, 25, MPI_BYTE, MPI_STATUS_IGNORE);
        
        printf("task: %d | header: %s\n", taskid, header);
        MPI_Barrier(MPI_COMM_WORLD);
    }

    else {
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_File_close(&fh);

    //sprintf(filename, "output.ppm");
    file = fopen(filename,"a");

    for(int i = 0; i < image_buffer_size; i++){
        if (image_buffer[i][0] != 0 || image_buffer[i][1] != 0 || image_buffer[i][2] != 0) {
            fwrite(image_buffer[i], 1 , 3, file);
            //MPI_File_write_ordered(fh, image_buffer[i], 3, MPI_BYTE, MPI_STATUS_IGNORE);
        }
        else {
            fseek(file, 1, SEEK_CUR);
            //MPI_File_seek(fh, 3, MPI_SEEK_CUR);
        }
        //MPI_File_write(fh, image_buffer[i], 3, MPI_BYTE, MPI_STATUS_IGNORE);
        
    };

    printf("task %d done.\n", taskid);
    
    fclose(file);   
};

void compute_mandelbrot(int init_x, int final_x, int init_y, int final_y){
    double z_x;
    double z_y;
    double z_x_squared;
    double z_y_squared;
    double escape_radius_squared = 4;

    int iteration;
    int i_x;
    int i_y;

    double c_x;
    double c_y;

    for(i_y = init_y; i_y < final_y; i_y++){
        c_y = c_y_min + i_y * pixel_height;

        if(fabs(c_y) < pixel_height / 2){
            c_y = 0.0;
        };

        for(i_x = init_x; i_x < final_x; i_x++){
            c_x         = c_x_min + i_x * pixel_width;

            z_x         = 0.0;
            z_y         = 0.0;

            z_x_squared = 0.0;
            z_y_squared = 0.0;

            for(iteration = 0;
                iteration < iteration_max && \
                ((z_x_squared + z_y_squared) < escape_radius_squared);
                iteration++){
                z_y         = 2 * z_x * z_y + c_y;
                z_x         = z_x_squared - z_y_squared + c_x;

                z_x_squared = z_x * z_x;
                z_y_squared = z_y * z_y;
            };

            update_rgb_buffer(iteration, i_x, i_y);
        };
    };
};

struct sub_image calculate_sub_image(int id) {
    struct sub_image si;

    si.init_x = 0; 
    si.init_y = (i_y_max/n_cores) * id;
    si.final_x = i_x_max;
    si.final_y = si.init_y + (i_y_max/n_cores);

    return si;
}

void mandelbrot_th(int taskid) {
    struct sub_image si;

    si = calculate_sub_image(taskid);

    compute_mandelbrot(si.init_x, si.final_x, si.init_y, si.final_y);
}

int main(int argc, char *argv[]){
    int taskid;

    init(argc, argv);
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &n_cores);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

    
    

    allocate_image_buffer();
    mandelbrot_th(taskid);
    write_to_file(taskid);

    
    MPI_Finalize();
    return 0;
}
