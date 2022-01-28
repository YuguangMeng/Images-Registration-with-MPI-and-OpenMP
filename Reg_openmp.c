/*
   Student name: Yuguang Meng
   Medical images Registration with OpenMP multi-threads parallel processing
   Usage: Reg_openmp.out data_path NI
*/

#include "omp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define NI 64

int main (int argc, char *argv[])
{
  int    i, taskid, nprocs;
  double start, end;
  char char_buffer[256], data_path[256], output_path[256];

  strcpy(data_path,"$HOME/cs555/Project/root");
  strcpy(output_path,"$HOME/cs555/Project/nodes");

  nprocs = atoi(argv[2]);

  start = omp_get_wtime();

  #pragma omp parallel for default(none) private(i, char_buffer) shared(argv,data_path, output_path) num_threads(nprocs)
  for(i=0; i<NI; i++)
  {
    sprintf(char_buffer,
            "$HOME/cs555/Project/FSL/fslroi %s %s/r_%d %d 1\n",
            argv[1], output_path, i, i);
    system(char_buffer);

    sprintf(char_buffer,
            "$HOME/cs555/Project/FSL/flirt -ref %s/Template -in %s/r_%d -out %s/r_%d -cost corratio -dof 12 -searchrx -180 180 -searchry -180 180 -searchrz -180 180 -interp trilinear 1>/dev/null",
            data_path, output_path, i, output_path, i);
    system(char_buffer);
  }

  sprintf(char_buffer, "$HOME/cs555/Project/FSL/fslmerge -t %s/reg %s/r_*.nii.gz", output_path, output_path);
  system(char_buffer);
  sprintf(char_buffer, "rm %s/r_*.nii.gz", output_path);
  system(char_buffer);

  end = omp_get_wtime();
  printf("The processing time is %.0f s.\n", (end-start));

  return 0;
}
