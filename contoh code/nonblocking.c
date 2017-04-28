#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "math.h"
#include "mpi.h"

int main(int argc,char** argv)
{
   int          taskid, ntasks;
   MPI_Status   status;
   MPI_Request	send_request,recv_request;
   int          ierr,i,j,itask,recvtaskid;
   int	        buffsize;
   double       *sendbuff,*recvbuff;
   double       sendbuffsum,recvbuffsum;
   double       sendbuffsums[1024],recvbuffsums[1024];
   double       inittime,totaltime,recvtime,recvtimes[1024];

   MPI_Init(&argc, &argv);

   MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
   MPI_Comm_size(MPI_COMM_WORLD,&ntasks);

   buffsize=atoi(argv[1]);

   if ( taskid == 0 ){
     printf("\n\n\n");
     printf("#####\n\n");
     printf(" Example 6 \n\n");
     printf(" Point-to-point Communication: MPI_Isend MPI_Irecv \n\n");
     printf(" Vector size: %d\n",buffsize);
     printf(" Number of tasks: %d\n\n",ntasks);
     printf("#####\n\n");
     printf("                --> BEFORE COMMUNICATION <--\n\n");
   }

   sendbuff=(double *)malloc(sizeof(double)*buffsize);
   recvbuff=(double *)malloc(sizeof(double)*buffsize);

   srand((unsigned)time( NULL ) + taskid);
   for(i=0;i<buffsize;i++){
     sendbuff[i]=(double)rand()/RAND_MAX;
   }

   sendbuffsum=0.0;
   for(i=0;i<buffsize;i++){
     sendbuffsum += sendbuff[i];
   }   
   ierr=MPI_Gather(&sendbuffsum,1,MPI_DOUBLE,
                   sendbuffsums,1, MPI_DOUBLE,
                   0,MPI_COMM_WORLD);                  
   if(taskid==0){
     for(itask=0;itask<ntasks;itask++){
       recvtaskid=itask+1;
       if(itask==(ntasks-1))recvtaskid=0;
       printf("Task %d : Sum of vector sent to %d = %e\n",
               itask,recvtaskid,sendbuffsums[itask]);
     }  
   }

   inittime = MPI_Wtime();

   if ( taskid == 0 ){
     ierr=MPI_Isend(sendbuff,buffsize,MPI_DOUBLE,
	           taskid+1,0,MPI_COMM_WORLD,&send_request);   
     ierr=MPI_Irecv(recvbuff,buffsize,MPI_DOUBLE,
	           ntasks-1,MPI_ANY_TAG,MPI_COMM_WORLD,&recv_request);
     recvtime = MPI_Wtime();
   }
   else if( taskid == ntasks-1 ){
     ierr=MPI_Isend(sendbuff,buffsize,MPI_DOUBLE,
	           0,0,MPI_COMM_WORLD,&send_request);   
     ierr=MPI_Irecv(recvbuff,buffsize,MPI_DOUBLE,
	           taskid-1,MPI_ANY_TAG,MPI_COMM_WORLD,&recv_request);
     recvtime = MPI_Wtime();
   }
   else{
     ierr=MPI_Isend(sendbuff,buffsize,MPI_DOUBLE,
	           taskid+1,0,MPI_COMM_WORLD,&send_request);
     ierr=MPI_Irecv(recvbuff,buffsize,MPI_DOUBLE,
	           taskid-1,MPI_ANY_TAG,MPI_COMM_WORLD,&recv_request);
     recvtime = MPI_Wtime();
   }
   ierr=MPI_Wait(&send_request,&status);
   ierr=MPI_Wait(&recv_request,&status);

   totaltime = MPI_Wtime() - inittime;

   recvbuffsum=0.0;
   for(i=0;i<buffsize;i++){
     recvbuffsum += recvbuff[i];
   }   

   ierr=MPI_Gather(&recvbuffsum,1,MPI_DOUBLE,
                   recvbuffsums,1, MPI_DOUBLE,
                   0,MPI_COMM_WORLD);
                   
   ierr=MPI_Gather(&recvtime,1,MPI_DOUBLE,
                   recvtimes,1, MPI_DOUBLE,
                   0,MPI_COMM_WORLD);

   if(taskid==0){
     printf("#####\n\n");
     printf(" -> AFTER COMMUNICATION <- \n\n");
     for(itask=0;itask<ntasks;itask++){
       printf("Task %d : Sum of received vector= %e : Time=%f seconds\n",
               itask,recvbuffsums[itask],recvtimes[itask]);
     }  
     printf("\n");
     printf("#####\n\n");
     printf(" Communication time : %f seconds\n\n",totaltime);  
     printf("#####\n\n");
   }

   free(recvbuff);
   free(sendbuff);

   MPI_Finalize();

}