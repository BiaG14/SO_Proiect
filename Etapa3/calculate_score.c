#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
    int id;
    char username[50];
    float latitude;
    float longitude;
    char clue[256];
    int value;
} Treasure;

typedef struct {
    char name[50];
    int score;
} userScore;

int main(int argc,char** argv)
{ 
    if(argc!=2)
    {
        fprintf(stderr,"Numar invalid de argumente\n");
        return 0;
    }
    char path[100];
    strcpy(path,argv[1]);
    strcat(path,"/");
    strcat(path,"treasures.dat");
    int fd=open(path,O_RDONLY);
    if(fd<0)
    {
        perror("Fisier");
        exit(-2);
    }
    userScore users[100];
    Treasure t;
    int count=0;
    while(read(fd,&t,sizeof(Treasure))>0)
    { 
        int check=0; //verifica daca am sau nu deja numele in vector
        for(int i=0;i<count;i++)
        {
            if(strcmp(t.username,users[i].name)==0)
            {
                users[i].score+=t.value;
                check++;
            }
        }
        if(check==0)
        {
            
            strcpy(users[count].name,t.username);
            users[count].score=t.value;
            count++;
        }
    }
    for(int i=0;i<count;i++)
    {
        printf("%s %d\n",users[i].name,users[i].score);
    }
    close(fd);
    return 0;
}