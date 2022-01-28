/*
   Student name: Yuguang Meng
   Medical images Registration with sequential execution
   Usage: Reg_openmp.out data_path NI
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main (int argc, char *argv[])
{
  int    i, NI;
  time_t start, end;
  double  duration;
  char char_buffer[256], data_path[256], output_path[256];

  strcpy(data_path,"$HOME/cs555/Project/root");
  strcpy(output_path,"$HOME/cs555/Project/nodes");

  NI = atoi(argv[2]);

  start = time(NULL);

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

  end = time(NULL);
  duration = end - start;
  printf("The processing time is %.1f s.\n", duration);

  return 0;
}
