#include <mpi.h>
#include<stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
struct result
{
    int counter ;
    char* data;
};

double startTime;
double endTime;

bool Search(char MyChar[], char S[])
{
    int i = 0,j,k;
    int MyCharSize = strlen(MyChar);
    int SearchSize = strlen(S);

    int Yes = 0;

    while(i < MyCharSize)//Line
    {
        char a =(MyChar[i]);
        char b =(S[0]);
        if(a==b)
        {
            j = 0;
            k = i;
            while(j < SearchSize)
            {
                MyChar[k]=(MyChar[k]);
                S[j]=(S[j]);
                if(MyChar[k] == S[j])
                {
                    Yes++;
                }

                j++;
                k++;
            }
            if(Yes == SearchSize)
            {
                return true;
            }
            Yes = 0;
        }
        i++;
    }
    return false;
}


//Line  Query.
bool check(char str[], char Query2[])
{
    int l = strlen(Query2);
    char Query[l];
    strcpy(Query,Query2);

    char delim[] = " ";
    char *ptr = strtok(Query, delim);

    while(ptr != NULL)
    {
        if(Search(str,ptr))
        {
            ptr = strtok(NULL, delim);
        }
        else
            return false;
    }
    return true;
}



struct result readFile(char fileName[50],long offsit,char Query[])
{
    int i = 0;
    char *Result=(char *) malloc(sizeof(char)*5000);
    Result[i]='\0';
    FILE *file ;
    int sizeOfLine;
    char * data = (char *) malloc(sizeof(char)*300);
    file = fopen(fileName, "r") ;

    struct result r;
    r.counter=0;

    fseek(file,offsit,0);
    // printf("%ld", ftell(file));  // print the position of pointer after seek.
    if (file == NULL )
    {
        printf( "file failed to open." ) ;
    }
    else
    {
        for( i = 0; i < 30; i++)
        {
            fgets (data, 400, file);
            if(check(data,Query))
            {
                // printf("%s",data);
                r.counter++;
                strcat(Result, data);
                //printf("%s",Result);
            }
        }
    }
    r.data=Result;
    return r;
}

void writeResultInFile(char fileName[50], char finalResult[], int counter, char* qeury)
{

    FILE *file = fopen(fileName, "w");

    fprintf(file,"Query: %s \n",qeury);

    fprintf(file,"Search Results Found = %d \n",counter);
    fprintf(file,"%s",finalResult);

    fclose(file);
}

struct result searchInFiles(int irecv,int numOfFiles,char arr[])
{
    int i,count=0;
    char *res=(char *) malloc(sizeof(char)*10000);
    //printf("irecv = %d numoffiles = %d\n",irecv,numOfFiles);
    for(i=irecv; i<numOfFiles+irecv; i++)
    {
        char filename[50]="Aristo-Mini-Corpus/Aristo-Mini-Corpus P-";
        char num[3];
        //itoa(i, num, 10);
        snprintf (num, sizeof(num), "%d",i+1);
        strcat(filename,num);
        strcat(filename,".txt");
        //printf("\n%s", filename);
        struct result r= readFile(filename,0,arr);
        count+=r.counter;
        strcat(res, r.data);
    }

    //printf("\n\n%d\n",count);
    struct result finalres;
    finalres.counter=count;
    finalres.data=res;
    return finalres;
}


int main( int argc, char **argv )
{
    int irecv;
    int rank, size, i,numOfFiles,*range,query_length, *arrofcount;
    char query[100];
    char *finalres;
    struct result r;

    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    range=malloc(sizeof(int)*(size-1)-1);
    numOfFiles=50/(size-1);

    if(rank == 0)
    {
        startTime = MPI_Wtime();
        printf("\n\t\t\tSearch Engine Helper\n\n");
        arrofcount=(int *) malloc(sizeof(int)*(size-1));
        finalres=(char *) malloc(sizeof(char)*(size)*10000);
        printf("Enter your Query :\n");
        scanf("%[^\n]%*c", query);
        query_length = strlen(query);
        for(i=0; i<size-1; i++)
        {
            range[i]=i*numOfFiles;
        }

        if(50%(size-1)!=0)
        {
            int temp=(size-1)*numOfFiles;
            MPI_Send(&temp, 1, MPI_INT, (size-1), 0, MPI_COMM_WORLD);
        }


    }
    MPI_Bcast(&query_length, 1, MPI_INT, 0, MPI_COMM_WORLD); //for send the length of the  query
    MPI_Bcast(query, 100, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Scatter(&*range, 1, MPI_INT, &irecv, 1, MPI_INT, 0,MPI_COMM_WORLD);

    if(rank!=size-1)
    {
        // printf("rank = %d: irecv = %d\n", rank,irecv);
        // printf("%s %d\n", quiry,quiry_length);

        /// the code of files and search for rank 0 to size-2 each one started from file i+1 to numOfFiles  i=irecv
        /// if size = 6 the processes 0 ,1 ,2 ,3,4 will run and process 5 will not because the 50%(size-1)==0
        /// the last process will not run if there is no remainder
        r=searchInFiles( irecv, numOfFiles,query);


    }
    else if(50%(size-1)==0&&rank==size-1)
    {
        r.counter=0;
        r.data=(char *) malloc(sizeof(char)*10000);
    }
    else if(50%(size-1)!=0)
    {
        if(rank==size-1)
        {
            MPI_Recv(&irecv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            // printf("rank = %d: irecv = %d\n", rank,irecv);
            // printf("%s %d\n", quiry,quiry_length);




            /// it run only if there is remainder
            /// the code of files and search for rank size-1 only (the last one) from i=irecv to 50
            /// if size = 5 the process 4 will run because the 50%(size-1)!=0 and it search in only 2 files because the 50%(size-1)=2

            r=searchInFiles( irecv, 50%(size-1),query);

        }
    }
    MPI_Gather(&r.counter, 1, MPI_INT, arrofcount, 1, MPI_INT, 0,MPI_COMM_WORLD);
    MPI_Gather(r.data, 10000, MPI_CHAR, finalres, 10000, MPI_CHAR, 0,MPI_COMM_WORLD);

    if(rank==0)
    {
        int count=0;

        for(i=0; i<size; i++)
        {
            count+=arrofcount[i];
        }
        printf("%d\n", count);
        int x=0;
        char*all=(char *) malloc(sizeof(char)*40000);
        for(i=0; i<size*10000; i++)
        {
           if(finalres[i]!=NULL)
           {
               all[x]=finalres[i];
               x++;
           }
        }

        writeResultInFile("file.txt",all,count,query);
        //printf("%s\n", finalres);

        endTime = MPI_Wtime();
        printf("\nRunning Time = %f\n\n", endTime - startTime);
    }



    MPI_Finalize();

}

