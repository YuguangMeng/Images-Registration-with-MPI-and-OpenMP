/*
   Student name: Yuguang Meng
   Medical images Registration with MPI parallel processing
   Usage: mpirun NI Reg_MPI.out data_path
*/

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define  ROOT	0
#define  NI 64

int main (int argc, char *argv[])

{
  unsigned int    i, numtasks, taskid, block_NI, size, *proc_data_size;
  unsigned char   *SEND_MSG, *RECV_MSG;       //unsigned char or double

  double start, end;
  char char_buffer[1024], root_path[256], node_path[256];
  FILE *fp;

  MPI_Status status;

  strcpy(root_path,"/home/ymeng3/cs555/Project/root");
  strcpy(node_path,"/home/ymeng3/cs555/Project/nodes");

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

  if(taskid == ROOT)
    start = MPI_Wtime();

  if(fp=fopen(argv[1],"rb"))
  {
    fseek( fp, 0L, SEEK_END );
    size=ftell(fp);
    rewind(fp);
    fclose(fp);
  }

  block_NI = NI/(numtasks - 1);
  proc_data_size = (int *)malloc(sizeof(int)*(numtasks));

  if (taskid == ROOT)
  {
    if(fp=fopen(argv[1],"rb"))
    {
      SEND_MSG = (unsigned char *)malloc(sizeof(unsigned char)*size);
      if(fread(SEND_MSG,sizeof(unsigned char), size,fp)!=0)
        MPI_Bcast(SEND_MSG, size, MPI_BYTE, ROOT, MPI_COMM_WORLD);
      fclose(fp);
      free(SEND_MSG);
    }

    MPI_Gather(&size, 1, MPI_INT, proc_data_size, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    for( i=1; i <= numtasks-1; i++)
    {
      RECV_MSG = (unsigned char *)malloc(sizeof(unsigned char)*proc_data_size[i]);
      MPI_Recv(RECV_MSG, proc_data_size[i], MPI_BYTE, i, i, MPI_COMM_WORLD, &status);

      sprintf(char_buffer, "%s/r_%d.nii.gz", root_path,i);
      if(fp=fopen(char_buffer,"wb"))
      {
        fwrite(RECV_MSG, sizeof(unsigned char),proc_data_size[i], fp);
        fclose(fp);
      }
      free(RECV_MSG);
    }
    sprintf(char_buffer, "$HOME/cs555/Project/FSL/fslmerge -t %s/reg %s/r_*.nii.gz", root_path, root_path);
    system(char_buffer);
    sprintf(char_buffer, "rm %s/r_*.nii.gz", root_path);
    system(char_buffer);
  }
  else
  {
    RECV_MSG = (unsigned char *)malloc(sizeof(unsigned char)*size);
    MPI_Bcast(RECV_MSG, size, MPI_BYTE, ROOT, MPI_COMM_WORLD);

    sprintf(char_buffer, "%s/r_%d.nii.gz", node_path,taskid);
    if(fp=fopen(char_buffer,"wb"))
    {
      // local operations start
      fwrite(RECV_MSG, sizeof(unsigned char), size, fp);
      fclose(fp);

      for (i=0; i<block_NI; i++)
      {
        sprintf(char_buffer,
                "$HOME/cs555/Project/FSL/fslroi %s/r_%d %s/r_%d_%d %d 1\n",
                node_path, taskid, node_path, taskid, i, i);
        system(char_buffer);

        sprintf(char_buffer,
                "$HOME/cs555/Project/FSL/flirt -ref %s/Template -in %s/r_%d_%d -out %s/r_%d_%d -cost corratio -dof 12 -searchrx -180 180 -searchry -180 180 -searchrz -180 180 -interp trilinear 1>/dev/null",
                root_path, node_path, taskid, i, node_path, taskid, i);
        system(char_buffer);
      }

      sprintf(char_buffer, "$HOME/cs555/Project/FSL/fslmerge -t %s/r_%d %s/r_%d_*.nii.gz", node_path, taskid, node_path, taskid);
      system(char_buffer);
      sprintf(char_buffer, "rm %s/r_%d_*.nii.gz", node_path, taskid);
      system(char_buffer);

      // local operations end
      MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
      sprintf(char_buffer, "%s/r_%d.nii.gz", node_path,taskid);
      if(fp=fopen(char_buffer,"rb"))
      {
        fseek( fp, 0L, SEEK_END );
        size=ftell(fp);
        rewind(fp);

        MPI_Gather(&size, 1, MPI_INT, proc_data_size, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

        SEND_MSG = (unsigned char *)malloc(sizeof(unsigned char)*size);
        if(fread(SEND_MSG,sizeof(unsigned char), size,fp)!=0)
          MPI_Send(SEND_MSG, size, MPI_BYTE, ROOT, taskid, MPI_COMM_WORLD);
        free(SEND_MSG);
        fclose(fp);
      }
      sprintf(char_buffer, "rm %s/r_%d.nii.gz", node_path, taskid);
      system(char_buffer);
    }
    free(RECV_MSG);
  }

  free(proc_data_size);

  if(taskid == ROOT)
  {
    end = MPI_Wtime();
    printf("The processing time is %.0f s.\n", end-start);
  }

  MPI_Finalize();

}
