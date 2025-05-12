#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>



pid_t pid_monitor=-1,pid2=-1;
int activ=0; //Verifica daca monitorul este activ sau nu
int read_d=-1; //pentru capatul de citire din pipe

void start_monitor()
{
    if(activ)
    {
        printf("Monitorul este activ\n");
        return;
    }
    int pfd[2];
    if(pipe(pfd) == -1)
    {
        perror("Pipe: ");
        exit(-2);
    }
    pid_t pid=fork();
    if(pid<0)
    {//failed fork process
        perror("Fork: ");
        return;
    }
    if(pid==0)
    {//child
        close(pfd[0]);
        dup2(pfd[1],1);
        close(pfd[1]);
        execl("./monitor","monitor",NULL);
        perror("Execl");
        exit(1);
    }
    else
    {//parent
        close(pfd[1]);
        pid_monitor=pid;
        activ=1;
        read_d=pfd[0];
        
        if (read_d < 0) {
            perror("Descriptor");
        }
        printf("Monitorul a fost activat cu pid-ul %d\n", pid);
    }
}

int expirat=0;

void handler_alrm(int sig)
{
    printf("Monitorul nu s-a oprit in cele 5 secunde\n");
    expirat=1;
}

void stop_monitor()
{
    if(activ==0)
    {
        printf("Monitorul este inactiv\n");
    }
    else
    {
        kill(pid_monitor,SIGUSR2);
        struct sigaction sigact1;
        sigact1.sa_handler=handler_alrm;
        sigemptyset(&sigact1.sa_mask);
        sigact1.sa_flags = 0;
        sigaction(SIGALRM,&sigact1,NULL);
        alarm(5);
        while(1)
        {
            if(activ==0)
            {
                printf("Monitorul s-a oprit\n");
                alarm(0);
                return;
            }
            if(expirat)
            {
                return;
            }
            usleep(100000);
        }
    }

}

struct Treasure {
    int id;               
    char username[50];
    float latitude, longitude;
    char clue[256];
    int value; 
};

void calculate_score(const char* cale)
{
    DIR *dir=opendir(cale);
    if(!dir)
    {
       return;
    }
    struct dirent *entry;
    while((entry=readdir(dir)) != NULL)
    {
        if(strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0)
            continue; //ne-ar trimite in acelasi director sau in directorul parinte
        char directory_path[512];
        strcpy(directory_path,cale);
        strcat(directory_path,"/");
        strcat(directory_path,entry->d_name);
        struct stat statbuf;
        if(stat(directory_path,&statbuf)==-1)
            continue;
        if(!S_ISDIR(statbuf.st_mode))
        {
           continue;
        }
        int pfd2[2];
        if(pipe(pfd2)==-1)
        {
            perror("Pipe");
            exit(-2);
        }
        pid2=fork();
        if(pid2<0)
        {
            perror("Fork");
            exit(-2);
        }
        else if(pid2==0)
        {
            close(pfd2[0]);
            dup2(pfd2[1],1);
            close(pfd2[1]);
            execl("./calculate_score","./calculate_score",entry->d_name,NULL);
            perror("Execl");
            exit(1);
         }
        else
        {
            close(pfd2[1]);
            char buffer[512]={0};
            printf("Scoruri pentru %s:\n", entry->d_name);
            ssize_t bytes_read=0;
            while ((bytes_read = read(pfd2[0], buffer, sizeof(buffer)-1)) > 0) {
                buffer[bytes_read] = '\0';
                printf("%s", buffer);
            }
            close(pfd2[0]);
            waitpid(pid2, NULL, 0);
        }
    }
    closedir(dir);
}

void handler_chld(int signum)
{
    int status;
    waitpid(pid_monitor,&status,0);
    activ=0;
    pid_monitor=-1;
}

int main(void)
{
    struct sigaction sa;
    sa.sa_handler=handler_chld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD,&sa,NULL);
    char comanda[256];
    int fd_comanda=open("command.txt",O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
    if(fd_comanda<0)
    {
        perror("Fisier\n");
        exit(-2);
    }
    close(fd_comanda);
    while(1)
    {  
        printf("Comanda:\n");
        if(fgets(comanda,sizeof(comanda),stdin)==0)
            break;
        comanda[strlen(comanda)-1]='\0'; //Sterg \n de la finalul comenzii citite
        if(strcmp(comanda,"--start_monitor")==0)
        {
            start_monitor();
            usleep(50000);
        } else
        if(strcmp(comanda,"--stop_monitor")==0)
        {
            stop_monitor();
            usleep(50000);
        } else
        if(strncmp(comanda,"--list_hunts",12)==0)
        { 
            if(activ==0)
            {
                printf("Monitor inactiv\n");
            }
            else
            {
                int fd_local=open("command.txt",O_WRONLY);
                if(fd_local<0)
                {
                    perror("Fisier\n");
                    exit(-2);
                }
                write(fd_local,comanda,strlen(comanda));
                close(fd_local);
                usleep(10000);
                kill(pid_monitor,SIGUSR1);

                sleep(2);
                char buffer[512];
               
                ssize_t bytes_read = read(read_d, buffer, sizeof(buffer) - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0'; // terminator de string
                    printf("%s", buffer);
                } else {
                    perror("Eroare la citirea din pipe");
                }
            }
            sleep(2);
        } else
        if(strncmp(comanda,"--list_treasures ",17)==0)
        {
            if(activ==0)
            {
                printf("Monitor inactiv\n");
            }
            else
            {
                int fd_local=open("command.txt",O_WRONLY);
                if(fd_local<0)
                {
                    perror("Fisier\n");
                    exit(-2);
                }
                write(fd_local,comanda,strlen(comanda));
                close(fd_local);
                usleep(10000);
                kill(pid_monitor,SIGUSR1);

                sleep(2);
                char buffer[512];
                ssize_t bytes_read = read(read_d, buffer, sizeof(buffer) - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0'; // terminator de string
                    printf("%s", buffer);
                } else {
                    perror("Eroare la citirea din pipe");
                }
            }
            sleep(2);
        } else
        if(strncmp(comanda,"--view_treasure ",16)==0)
        {
            if(activ==0)
            {
                printf("Monitor inactiv\n");
            }
            else
            {
                int fd_local=open("command.txt",O_WRONLY);
                if(fd_local<0)
                {
                    perror("Fisier\n");
                    exit(-2);
                }
                write(fd_local,comanda,strlen(comanda));
                close(fd_local);
                usleep(10000);
                kill(pid_monitor,SIGUSR1);
                sleep(2);
                char buffer[512];
                ssize_t bytes_read = read(read_d, buffer, sizeof(buffer) - 1);
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0'; // terminator de string
                    printf("%s", buffer);
                } else {
                    perror("Eroare la citirea din pipe");
                }
            }
            sleep(2);
        } else
        if(strcmp(comanda,"--exit")==0)
        {
            if(activ==1)
            {
                printf("Monitorul e inca activ\n");
            }
            else
                break; //Iesire din while dupa ce monitorul a devenit inactiv
        }
        else
        if(strncmp(comanda,"--calculate_score",17)==0)
        {
            if(activ==1)
            {
                printf("Opreste intai monitorul!\n");
            }
            else
            {
                calculate_score(".");
                usleep(10000);
            }
        }
        
        else
        {
            printf("Comanda indisponibila\n");
        }
    }
    
    return 0;
}