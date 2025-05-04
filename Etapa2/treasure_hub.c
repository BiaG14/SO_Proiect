#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>


pid_t pid_monitor=-1;
int activ=0; //Verifica daca monitorul este activ sau nu

void start_monitor()
{
    if(activ)
    {
        printf("Monitorul este activ\n");
        return;
    }
    pid_t pid=fork();
    if(pid<0)
    {//failed fork process
        perror("fork\n");
        return;
    }
    if(pid==0)
    {//child
        execl("./monitor","monitor",NULL);
        perror("execl\n");
        exit(1);
    }
    if(pid>0)
    {//parent
        pid_monitor=pid;
        activ=1;
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

void handler_chld(int signum)
{
    int status;
    waitpid(pid_monitor,&status,0);
    printf("Procesul monitor s-a terminat\n");
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
    int fd=open("command.txt",O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
    if(fd<0)
    {
        perror("Fisier\n");
        exit(-2);
    }
    close(fd);
    while(1)
    {  
        printf("Comanda:\n");
        if(fgets(comanda,sizeof(comanda),stdin)==0)
            break;
        comanda[strlen(comanda)-1]='\0'; //Sterg \n de la finalul comenzii citite
        if(strcmp(comanda,"--start_monitor")==0)
        {
            start_monitor();
        } else
        if(strcmp(comanda,"--stop_monitor")==0)
        {
            stop_monitor();
        } else
        if(strncmp(comanda,"--list_hunts",12)==0)
        {
            if(activ==0)
            {
                printf("Monitor inactiv\n");
            }
            else
            {
                int fd=open("command.txt",O_WRONLY);
                if(fd<0)
                {
                    perror("Fisier\n");
                    exit(-2);
                }
                write(fd,comanda,strlen(comanda));
                kill(pid_monitor,SIGUSR1);
                
            }
        } else
        if(strncmp(comanda,"--list_treasures ",17)==0)
        {
            if(activ==0)
            {
                printf("Monitor inactiv\n");
            }
            else
            {
                int fd=open("command.txt",O_WRONLY );
                if(fd<0)
                {
                perror("Fisier\n");
                exit(-2);
                }
                write(fd,comanda,strlen(comanda));
                kill(pid_monitor,SIGUSR1);
               
            }
        } else
        if(strncmp(comanda,"--view_treasure ",16)==0)
        {
            if(activ==0)
            {
                printf("Monitor inactiv\n");
            }
            else
            {
                int fd=open("command.txt",O_WRONLY);
                if(fd<0)
                {
                    perror("Fisier\n");
                    exit(-2);
                }
                write(fd,comanda,strlen(comanda));
                kill(pid_monitor,SIGUSR1);
               
            }
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
        {
            printf("Comanda indisponibila\n");
        }
        close(fd);
        sleep(3);
    }
    
    return 0;
}