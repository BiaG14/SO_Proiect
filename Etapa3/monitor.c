#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>


struct Treasure {
    int id;               
    char username[50];
    float latitude, longitude;
    char clue[256];
    int value; 
};

struct Treasure t;

void list_hunts(const char* cale)
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
        char treasure_path[512];
        strcpy(treasure_path,directory_path);
        strcat(treasure_path,"/");
        strcat(treasure_path,"treasures.dat");
        int fd=open(treasure_path,O_RDONLY);
        if(fd<0)
        {
            continue;
        }
        off_t size = lseek(fd,0,SEEK_END);
        close(fd);
        int nr_comori=size/sizeof(struct Treasure);
        printf("Fisier: %s, size: %lld, struct: %ld\n", treasure_path, size, sizeof(struct Treasure));
        printf("HUNT: %s; Comori: %d\n",entry->d_name, nr_comori);
       

    }
    closedir(dir);
}

void list(const char *hunt_id)
{ 
    struct stat goodStat, fileStat;
    // 0 on success, -1 on failure
    int checkDir=stat(hunt_id,&goodStat); //Verific daca hunt-id-ul exista
    if(checkDir!=0)
    {
        perror("Director");
        exit(2);
    }

    char pathFile[100]; // Conține calea pentru fișierele din director
    strcpy(pathFile, hunt_id);
    strcat(pathFile, "/treasures.dat");

    int fileDescriptor = open(pathFile, O_RDONLY); // Deschide fișierul doar pentru citire
    if (fileDescriptor < 0)
    {
        perror("Fisier");
        exit(-2);
    }

    // Afișează informații despre director și fișier
    char buffer[100], buffer2[100];
    write(1, hunt_id, strlen(hunt_id));
    write(1, "\n", 1);

    if (stat(pathFile, &fileStat) == 0)
    {
        sprintf(buffer, "Dimensiunea fisierului %s este: %lld bytes\n", pathFile, fileStat.st_size);
        write(1, buffer, strlen(buffer));
        
        time_t timestamp = fileStat.st_mtimespec.tv_sec;
        struct tm *tmInfo = localtime(&timestamp);
        strftime(buffer2, 80, "%Y-%m-%d %H:%M:%S", tmInfo);
        
        write(1, "Ultima modificare a fisierului a avut loc la: ", strlen("Ultima modificare a fisierului a avut loc la: "));
        write(1, buffer2, strlen(buffer2));
        write(1, "\n", 1);
    }

    // Citește fișierul binar și afișează datele
    char num[50];
    while (read(fileDescriptor, &t, sizeof(t)) > 0)
    {
        // Afișează informațiile despre fiecare treasure pe care l-am citit in t
        write(1, "ID: ", strlen("ID: "));
        sprintf(num, "%d",t.id);
        write(1, num, strlen(num));
        write(1, "\n", 1);

        write(1, "Username: ", strlen("Username: "));
        write(1, t.username, strlen(t.username));
        write(1, "\n", 1);

        write(1, "Latitude: ", strlen("Latitude: "));
        sprintf(num, "%.2f", t.latitude);
        write(1, num, strlen(num));
        write(1, "\n", 1);

        write(1, "Longitude: ", strlen("Longitude: "));
        sprintf(num, "%.2f", t.longitude);
        write(1, num, strlen(num));
        write(1, "\n", 1);

        write(1, "Clue: ", strlen("Clue: "));
        write(1, t.clue, strlen(t.clue));
        write(1, "\n", 1);

        write(1, "Value: ", strlen("Value: "));
        sprintf(num,"%d",t.value);
        write(1, num, strlen(num));
        write(1, "\n\n", 2);
    }

    // Deschide fișierul de log
    char logFile[100]; // Conține calea pentru fișierele din director
    strcpy(logFile, hunt_id);
    strcat(logFile, "/logged_hunt.txt");

    int logDescriptor = open(logFile, O_RDWR | O_APPEND);
    if (logDescriptor < 0)
    {
        if ((logDescriptor = open(logFile, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
        {
            perror("Log");
            exit(-2);
        }
        
    }

    char message[512];
    strcpy(message, "--list ");
    strcat(message, hunt_id);
    strcat(message, "\n");
    write(logDescriptor, message, strlen(message));
    close(logDescriptor);
    close(fileDescriptor);
}

void view(const char *hunt_id,const char *id)
{ 
    struct stat goodStat;
    //0 on success, -1 on failure
    int checkDir=stat(hunt_id,&goodStat); //Verific daca hunt-id-ul exista deja
    if(checkDir!=0)
    {
        perror("Director");
        return;
    }
    char pathFile[100]; //Contine calea pentru fisierele din director
    strcpy(pathFile,hunt_id);
    strcat(pathFile,"/treasures.dat");
    int fileDescriptor=open(pathFile, O_RDONLY);   
    if(fileDescriptor<0)
    {
        perror("Fisier");
        return;
    }
    int treasure_id=atoi(id); //Pentru id-ul pe care il cautam
    int gasit=0; //Ca sa verificam daca a fost gasiit acel id
    char num[50];
    while(read(fileDescriptor,&t,sizeof(t))>0)
    {
        if(t.id==treasure_id)
        {
            write(1, "ID: ", strlen("ID: "));
            sprintf(num, "%d",t.id);
            write(1, num, strlen(num));
            write(1, "\n", 1);

            write(1, "Username: ", strlen("Username: "));
            write(1, t.username, strlen(t.username));
            write(1, "\n", 1);

            write(1, "Latitude: ", strlen("Latitude: "));
            sprintf(num,"%.2f",t.latitude);
            write(1, num, strlen(num));
            write(1, "\n", 1);

            write(1, "Longitude: ", strlen("Longitude: "));
            sprintf(num,"%.2f",t.longitude);
            write(1, num, strlen(num));
            write(1, "\n", 1);

            write(1, "Clue: ", strlen("Clue: "));
            write(1, t.clue, strlen(t.clue));
            write(1, "\n", 1);

            write(1, "Value: ", strlen("Value: "));
            sprintf(num,"%d",t.value);
            write(1, num, strlen(num));
            write(1, "\n", 1);
            gasit=1;
            break;
        }
    }
    if (gasit==0)
    {
        write(1, "Comoara cu ID-ul specificat nu a fost gasita\n", strlen("Comoara cu ID-ul specificat nu a fost gasita\n"));
        close(fileDescriptor);
    }
    char logFile[100]; //Contine calea pentru fisierele din director
    strcpy(logFile,hunt_id);
    strcat(logFile,"/logged_hunt.txt");
    int logDescriptor=open(logFile, O_RDWR | O_APPEND);   
    if(logDescriptor<0)
    {
        //write(1,"Fisierul nu exista\n",strlen("Fisierul nu exista\n"));
        if((logDescriptor = open(logFile, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
        {
            perror("Log");
            return;
        }
        
    }
    char message[512];
    strcpy(message,"--view ");
    strcat(message, hunt_id);
    strcat(message," ");
    strcat(message,id);
    strcat(message,"\n");
    write(logDescriptor,message,strlen(message));
    close(logDescriptor);
    close(fileDescriptor);

}

void handler_USR1(int signum)
{
    sleep(1);
    int fd=open("command.txt",O_RDONLY);
    if(fd<0)
    {
        perror("Fisier");
        exit(-1);
    }
    char comanda[256]={0};
    char c;
    int i = 0;
    while(read(fd, &c, 1) > 0) {
    if (c == '\n') 
    {
        comanda[i] = '\0';
        i = 0;
    } 
    else 
    {
        comanda[i++] = c;
    }
}
    char copie[256];
    strcpy(copie,comanda);
    if(strncmp(comanda,"--list_hunts",13)==0)
    {
        list_hunts(".");
    }
    if(strncmp(comanda,"--list_treasures ",17)==0)
    {
        char hunt_id[10];
        int i=0;
        char *arg=strtok(copie, " ");
        while(arg!=NULL)
        {
            if(i==1)
            strcpy(hunt_id,arg);
            i++;
            arg=strtok(NULL," ");
        }
        list(hunt_id);
    }
    if(strncmp(comanda,"--view_treasure ",16)==0)
    {  
        char hunt_id[10],id[10];
        int i=-1;
        char* arg=strtok(copie, " ");
        while(arg!=NULL)
        {
            i++;
            if(i==1)
            strcpy(hunt_id,arg);
            if(i==2)
            strcpy(id,arg);
            arg=strtok(NULL," ");
        }
        view(hunt_id,id);
    }
    close(fd);
    fd = open("command.txt", O_WRONLY | O_TRUNC);
    if (fd >= 0) {
        close(fd); 
    }
}

void handler_USR2(int signum)
{
    exit(0);
}

int main(void)
{   setbuf(stdout,NULL);
    struct sigaction sa1,sa2;
    sa1.sa_handler=handler_USR1;
    sigaction(SIGUSR1,&sa1,NULL);
    sa2.sa_handler=handler_USR2;
    sigaction(SIGUSR2,&sa2,NULL);
    while(1)
    {
        pause();
    }
    return 0;
}