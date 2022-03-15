
/*
 * Conway's Game of Life
 *
 * A. Mucherino
 *
 * PPAR
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

int N = 32;
int itMax = 20;

// allocation only
unsigned int* allocate()
{
   return (unsigned int*)calloc(N*N,sizeof(unsigned int));
};

// conversion cell location : 2d --> 1d
// (row by row)
int code(int x,int y,int dx,int dy)
{
   int i = (x + dx)%N;
   int j = (y + dy)%N;
   if (i < 0)  i = N + i;
   if (j < 0)  j = N + j;
   return i*N + j;
};

// writing into a cell location 
void write_cell(int x,int y,unsigned int value,unsigned int *world)
{
   int k;
   k = code(x,y,0,0);
   world[k] = value;
};

// random generation
unsigned int* initialize_random()
{
   int x,y;
   unsigned int cell;
   unsigned int *world;

   world = allocate();
   for (x = 0; x < N; x++)
   {
      for (y = 0; y < N; y++)
      {
         if (rand()%5 != 0)
         {
            cell = 0;
         }
         else if (rand()%2 == 0)
         {
            cell = 1;
         }
         else
         {
            cell = 2;
         };
         write_cell(x,y,cell,world);
      };
   };
   return world;
};

// dummy generation
unsigned int* initialize_dummy()
{
   int x,y;
   unsigned int *world;

   world = allocate();
   for (x = 0; x < N; x++)
   {
      for (y = 0; y < N; y++)
      {
         write_cell(x,y,x%3,world);
      };
   };
   return world;
};

// "glider" generation
unsigned int* initialize_glider()
{
   int x,y,mx,my;
   unsigned int *world;

   world = allocate();
   for (x = 0; x < N; x++)
   {
      for (y = 0; y < N; y++)
      {
         write_cell(x,y,0,world);
      };
   };

   mx = N/2 - 1;  my = N/2 - 1;
   x = mx;      y = my + 1;  write_cell(x,y,1,world);
   x = mx + 1;  y = my + 2;  write_cell(x,y,1,world);
   x = mx + 2;  y = my;      write_cell(x,y,1,world);
                y = my + 1;  write_cell(x,y,1,world);
                y = my + 2;  write_cell(x,y,1,world);

   return world;
};

// "small exploder" generation
unsigned int* initialize_small_exploder()
{
   int x,y,mx,my;
   unsigned int *world;

   world = allocate();
   for (x = 0; x < N; x++)
   {
      for (y = 0; y < N; y++)
      {
         write_cell(x,y,0,world);
      };
   };

   mx = N/2 - 2;  my = N/2 - 2;
   x = mx;      y = my + 1;  write_cell(x,y,2,world);
   x = mx + 1;  y = my;      write_cell(x,y,2,world);
                y = my + 1;  write_cell(x,y,2,world);
                y = my + 2;  write_cell(x,y,2,world);
   x = mx + 2;  y = my;      write_cell(x,y,2,world);
                y = my + 2;  write_cell(x,y,2,world);
   x = mx + 3;  y = my + 1;  write_cell(x,y,2,world);

   return world;
};


// reading a cell
int read_cell(int x,int y,int dx,int dy,unsigned int *world)
{
   int k = code(x,y,dx,dy);
   return world[k];
};

// updating counters
void update(int x,int y,int dx,int dy,unsigned int *world,int *nn,int *n1,int *n2)
{
   unsigned int cell = read_cell(x,y,dx,dy,world);
   if (cell != 0)
   {
      (*nn)++;
      if (cell == 1)
      {
         (*n1)++;
      }
      else
      {
         (*n2)++;
      };
   };
};

// looking around the cell
void neighbors(int x,int y,unsigned int *world,int *nn,int *n1,int *n2)
{
   int dx,dy;

   (*nn) = 0;  (*n1) = 0;  (*n2) = 0;

   // same line
   dx = -1;  dy = 0;   update(x,y,dx,dy,world,nn,n1,n2);
   dx = +1;  dy = 0;   update(x,y,dx,dy,world,nn,n1,n2);

   // one line down
   dx = -1;  dy = +1;  update(x,y,dx,dy,world,nn,n1,n2);
   dx =  0;  dy = +1;  update(x,y,dx,dy,world,nn,n1,n2);
   dx = +1;  dy = +1;  update(x,y,dx,dy,world,nn,n1,n2);   

   // one line up
   dx = -1;  dy = -1;  update(x,y,dx,dy,world,nn,n1,n2);
   dx =  0;  dy = -1;  update(x,y,dx,dy,world,nn,n1,n2);
   dx = +1;  dy = -1;  update(x,y,dx,dy,world,nn,n1,n2);
};

// computing a new generation
short newgeneration(unsigned int *world1,unsigned int *world2,int xstart,int xend)
{
   int x,y;
   int nn,n1,n2;
   unsigned int cell;
   short change = 0;

   // cleaning destination world
   for (x = 0; x < N; x++)
   {
      for (y = 0; y < N; y++)
      {
         write_cell(x,y,0,world2);
      };
   };

   // generating the new world
   for (x = xstart; x < xend; x++)
   {
      for (y = 0; y < N; y++)
      {
         neighbors(x,y,world1,&nn,&n1,&n2);
         cell = read_cell(x,y,0,0,world1);

         if (cell != 0){

            // if alive cell has less than 2 or more than 3 neighbors it dies 
            if ( (n1+n2)<2 || (n1+n2)>3 ) {
               write_cell(x,y,0,world2);

            //if alive cell has 2 or 3 neighbors it lives on 
            } else if ( 1<(n1+n2)<4 ) {

               if (cell == 1){
                  write_cell(x,y,1,world2);
               } else {
                  write_cell(x,y,2,world2);
               }

            }
         } else { //if cell not alive has 3 alive neighbors it becomes alive
            if ((n1+n2) == 3 ){
               // becomes mayoritarian type of neighbor
               if (n1>n2) {
                  write_cell(x,y,1,world2);
               } else {
                  write_cell(x,y,2,world2);
               }

            }
         }

      };
   };
   change = 1;
   return change;
};

// cleaning the screen
void cls()
{
    int i;
    for (i = 0; i < 10; i++)
    {
        fprintf(stdout,"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    }
}

// diplaying the world
void print(unsigned int *world)
{
   int i;
   cls();
   for (i = 0; i < N; i++)  fprintf(stdout,"-");  

   for (i = 0; i < N*N; i++)
   {
      if (i%N == 0)  fprintf(stdout,"\n");
      if (world[i] == 0)  fprintf(stdout," ");
      if (world[i] == 1)  fprintf(stdout,"o");
      if (world[i] == 2)  fprintf(stdout,"x");
   };
   fprintf(stdout,"\n");

   for (i = 0; i < N; i++)  fprintf(stdout,"-");  
   fprintf(stdout,"\n");
   sleep(1);
};

// main
int main(int argc,char *argv[])
{

    int it,change;
    unsigned int *world1,*world2;
    unsigned int *worldaux;

    // n number of processes
    int my_rank, n;
    char hostname[128];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n);

    gethostname(hostname,128);

    if (N%n != 0){ // all processes check that N is divisible by p
        MPI_Finalize();
    }

    if (my_rank == 0) { // process 0

        //world1 = initialize_dummy();
        //world1 = initialize_random();
        //world1 = initialize_glider();
        world1 = initialize_small_exploder();
        world2 = allocate(); 


    } else { // rest of the processes 
         
         world2 = allocate(); 
         world1 = allocate();      


    }

   // send world1 to all other processes
   MPI_Bcast(world1, N*N, MPI_INT, 0, MPI_COMM_WORLD);

   // inicial print 
   if (my_rank == 0) {
    print(world1);

   }

   int lineUp = my_rank * (N / nbprocs); 
   int lineDown = (my_rank + 1) * (N / nbprocs) - 1;

   int rankUp = (my_rank - 1 < 0) ? nbprocs - 1 : my_rank - 1;
   int rankDown = (my_rank + 1 >= nbprocs) ? 0 : my_rank + 1;

   int neigh_LDown = (lineDown + 1 >= N) ? 0 : lineDown + 1;
   int neigh_LUp = (lineUp - 1 < 0) ? N - 1 : lineUp - 1;  


 
   it = 0;  change = 1; 
   while (change && it < itMax)
   {

      int xstart = N/n * my_rank;
      int xend = (my_rank + 1) * (N/n);

      change = newgeneration(world1,world2,xstart,xend);
      worldaux = world1;  world1 = world2;  world2 = worldaux;

      // exchange neighboring rows
      if (nbprocs > 1)
      {
         for (int i = 0; i < N; i++)
         {
            neigh_row[i] = read_cell(lineUp, i, 0, 0, world1);
         }

         MPI_Send(neigh_row, N, MPI_INT, rankUp, 1, MPI_COMM_WORLD);
         MPI_Recv(neigh_row, N, MPI_INT, rankDown, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         for (int i = 0; i < N; i++)
         {
            write_cell(neigh_LDown, i, neigh_row[i], world1);
         }

         for (int i = 0; i < N; i++)
         {
            neigh_row[i] = read_cell(lineDown, i, 0, 0, world1);
         }

         MPI_Send(neigh_row, N, MPI_INT, rankDown, 2, MPI_COMM_WORLD);
         MPI_Recv(neigh_row, N, MPI_INT, rankUp, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

         for (int i = 0; i < N; i++)
         {
            write_cell(neigh_LUp, i, neigh_row[i], world1);
         }

         MPI_Gather((world1 + N * lineUp), N * (N / nbprocs), MPI_INT, world2, N * (N / nbprocs), MPI_INT, 0, MPI_COMM_WORLD);
         if (my_rank == 0)
         {
            print(world2);
         }
      }
      
      it++;
   }; 

   if (my_rank == 0){
     print(world2);
   }


   // ending 
   free(world1);   free(world2);
   MPI_Finalize();
   return 0;
};

