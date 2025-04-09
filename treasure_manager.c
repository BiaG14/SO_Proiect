#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

struct Treasure {
    int id;               
    char username[50];
    float latitude, longitude;
    char clue[256];
    int value; 
};
//De structura s-a folosit doar creierul meu cand apela functia add pentru argumente

void add(const char *hunt_id)
{
    struct stat goodStat,linkStat;
    //0 on success, -1 on failure
    int checkDir=stat(hunt_id,&goodStat); //Verific daca hunt-id-ul exista deja
    if(checkDir==0)
    {
        write(1,"Directorul exista\n",strlen("Directorul exista\n"));
    }
    else
    {
        if(mkdir(hunt_id,0777)==0)
        {
            write(1,"Directorul a fost creat\n",strlen("Directorul a fost creat\n"));
        }
        else
        {
            write(1,"Directorul nu poate fi creat\n",strlen("Directorul nu poate fi creat\n"));
            exit(2);
        }
    }
    char pathFile[50]; //Contine calea pentru fisierele din director
    strcpy(pathFile,hunt_id);
    strcat(pathFile,"/treasures.dat");
    int fileDescriptor=open(pathFile, O_RDWR | O_APPEND);   
    if(fileDescriptor<0)
    {
        if((fileDescriptor = open(pathFile, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
        {
            write(1,"Fisierul nu poate fi creat\n",strlen("Fisierul nu poate fi creat\n"));
            exit(2);
        }
        else
        {
            write(1, "Fisierul a fost creat\n", strlen("Fisierul a fost creat\n"));
        }
    }
    write(1,"Informatiile despre treasure date pe linii separate\n",strlen("Informatiile despre treasure date pe linii separate\n"));
    // Daca trebuie citite datele de la stdin
    char tempLine[100]; //Pentru o linie citita
    char tempBuffer[512]; //Pentru toate informatiile despre treasure
    int i=0; //Numara liniile ca sa verifice sa se dea exact 6
    int size; //Numarul de bytes cititi
    char check[100]; //Pentru verificarea validitatii unei informatii (daca e numar)
    while((size=read(0,tempLine,100))>1)
    {
        i++;
        if(i>6) //S-au dat prea multe linii
        {
            break;
        }
        tempLine[size-1]='\0'; //Sterge '\n'
        if(i==1) //Verific ca pe prima linie sa am un id nr intreg
        {
            sprintf(check,"%d",atoi(tempLine));
            if(strcmp(check,tempLine)!=0)
                break;
        }
        if(i==3) //Pe a treia linie latitudinea numar real
        {
            char *endptr;
            strtof(tempLine, &endptr);
            if (*endptr != '\0') break;
            
        }
        if(i==4) //Pe a patra linie longitudinea numar real
        {
            char *endptr;
            strtof(tempLine, &endptr);
            if (*endptr != '\0') break;


        }
        if(i==6) //Pe a sasea linie valoarea numar intreg
        {
            sprintf(check,"%d",atoi(tempLine));
            if(strcmp(check,tempLine)!=0)
                break;
        }
        if(i==1)
        {
            strcpy(tempBuffer,tempLine);
        }
        else
        {
            strcat(tempBuffer," ");
            strcat(tempBuffer,tempLine);
        }
    }
    if(i==6) //Daca s-au dat informatii valide
    {
        write(fileDescriptor,tempBuffer,strlen(tempBuffer));
    }
    else
    {
        write(1,"Nu s-au dat informatii valide despre treasure\n",strlen("Nu s-au dat informatii valide despre treasure\n"));
        exit(2);
    }
    write(fileDescriptor,"\n",strlen("\n"));
    char logFile[50]; //Contine calea pentru fisierele din director
    strcpy(logFile,hunt_id);
    strcat(logFile,"/logged_hunt.txt");
    int logDescriptor=open(logFile, O_RDWR | O_APPEND);   
    if(logDescriptor<0)
    {
        //write(1,"Fisierul nu exista\n",strlen("Fisierul nu exista\n"));
        if((logDescriptor = open(logFile, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
        {
            //write(1,"Fisierul nu poate fi creat\n",strlen("Fisierul nu poate fi creat\n"));
            exit(2);
        }
        else
        {
            write(1, "Fisierul de log a fost creat", strlen("Fisierul de log a fost creat\n"));
        }
    }
    //In caz de eroare adaug in logged_hunt totusi ce s-a incercat?
    char message[512];
    strcpy(message,"--add ");
    strcat(message,tempBuffer);
    strcat(message,"\n");
    write(logDescriptor,message,strlen(message));
    char pathLink[50]; //Calea pentru legatura simbolica
    strcpy(pathLink,"logged_hunt-");
    strcat(pathLink,hunt_id);
    int checkLink=lstat(pathLink,&linkStat);
    if(checkLink!=0) //Verific daca exista
    {
        checkLink=symlink(pathFile,pathLink); //Daca nu exista creez una
        if(checkLink!=0)
        {
            write(1,"Legatura simbolica nu s-a putut crea\n",strlen("Legatura simbolica nu s-a putut crea\n"));
            exit(2);
        }
    }
    else
    {
        write(1,"Legatura simbolica exista deja\n",strlen("Legatura simbolica exista deja\n"));
    }
    close(logDescriptor);
    close(fileDescriptor);
}

void list(const char *hunt_id)
{
    struct stat goodStat,fileStat;
    //0 on success, -1 on failure
    int checkDir=stat(hunt_id,&goodStat); //Verific daca hunt-id-ul exista deja
    if(checkDir!=0)
    {
        write(1,"Directorul nu exista\n",strlen("Directorul nu exista\n"));
        exit(2);
    }
    char pathFile[50]; //Contine calea pentru fisierele din director
    strcpy(pathFile,hunt_id);
    strcat(pathFile,"/treasures.dat");
    int fileDescriptor=open(pathFile, O_RDWR | O_APPEND);   
    if(fileDescriptor<0)
    {
        write(1,"Fisierul cu treasures nu exista\n",strlen("Fisierul nu exista\n"));
        exit(2);
    }
    char buffer[4096];
    write(1,hunt_id,strlen(hunt_id));
    write(1,"\n",strlen("\n"));
    if(stat(pathFile,&fileStat)==0)
    {
        char buffer[100],buffer2[100];
        sprintf(buffer, "Dimensiunea fisierului %s este: %lld bytes\n", pathFile, fileStat.st_size);
        write(1,buffer,strlen(buffer));
        time_t timestamp = fileStat.st_mtimespec.tv_sec;
        struct tm *tmInfo=localtime(&timestamp);
        strftime(buffer2,80,"%Y-%m-%d %H:%M:%S",tmInfo);
        write(1,"Ultima modificare a fisierului a avut loc la: ",strlen("\nUltima modificare a fisierului a avut loc la: "));
        write(1,buffer2,strlen(buffer2));

        write(1,"\n",1);
    }

    int bytes;
    while ((bytes = read(fileDescriptor, buffer, sizeof(buffer))) > 0)
    {
        write(1, buffer, bytes);
    }

    char logFile[50]; //Contine calea pentru fisierele din director
    strcpy(logFile,hunt_id);
    strcat(logFile,"/logged_hunt.txt");
    int logDescriptor=open(logFile, O_RDWR | O_APPEND);   
    if(logDescriptor<0)
    {
        if((logDescriptor = open(logFile, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
        {
            //write(1,"Fisierul nu poate fi creat\n",strlen("Fisierul nu poate fi creat\n"));
            exit(2);
        }
        else
        {
            write(1, "Fisierul de log a fost creat", strlen("Fisierul de log a fost creat\n"));
        }
    }
    char message[512];
    strcpy(message,"--list ");
    strcat(message, hunt_id);
    strcat(message,"\n");
    write(logDescriptor,message,strlen(message));
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
        write(1,"Directorul nu exista\n",strlen("Directorul nu exista\n"));
        exit(2);
    }
    char pathFile[50]; //Contine calea pentru fisierele din director
    strcpy(pathFile,hunt_id);
    strcat(pathFile,"/treasures.dat");
    int fileDescriptor=open(pathFile, O_RDWR | O_APPEND);   
    if(fileDescriptor<0)
    {
        write(1,"Fisierul cu treasures nu exista\n",strlen("Fisierul nu exista\n"));
        exit(2);
    }
    char c;
    int poz=0;
    char buffer[1024];
    while(read(fileDescriptor,&c,1)==1)
    {
        if(c=='\n')
        {
            buffer[poz]='\0';
            int i=0;
            while(buffer[i]!=' ')
            {
                if(buffer[i]!=id[i])
                break;
                i++;
            }
            if(buffer[i]==' ') //Daca s-a potrivit id-ul de pe acea linie
            {
                write(1,buffer,strlen(buffer));
                write(1,"\n",1);
            }
            poz=0;
        }
        else
        {
            buffer[poz++]=c;
        }
    }
    char logFile[50]; //Contine calea pentru fisierele din director
    strcpy(logFile,hunt_id);
    strcat(logFile,"/logged_hunt.txt");
    int logDescriptor=open(logFile, O_RDWR | O_APPEND);   
    if(logDescriptor<0)
    {
        //write(1,"Fisierul nu exista\n",strlen("Fisierul nu exista\n"));
        if((logDescriptor = open(logFile, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
        {
            //write(1,"Fisierul nu poate fi creat\n",strlen("Fisierul nu poate fi creat\n"));
            exit(2);
        }
        else
        {
            write(1, "Fisierul de log a fost creat", strlen("Fisierul de log a fost creat\n"));
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

void remove_treasure(const char *hunt_id, const char *id)
{
    struct stat goodStat;
    //0 on success, -1 on failure
    int checkDir=stat(hunt_id,&goodStat); //Verific daca hunt-id-ul exista deja
    if(checkDir!=0)
    {
        write(1,"Directorul nu exista\n",strlen("Directorul nu exista\n"));
        exit(2);
    }
    char pathFile[50]; //Contine calea pentru fisierele din director
    strcpy(pathFile,hunt_id);
    strcat(pathFile,"/treasures.dat");
    int fileDescriptor=open(pathFile, O_RDWR | O_APPEND);  
    if(fileDescriptor<0)
    {
        write(1,"Fisierul cu treasures nu exista\n",strlen("Fisierul cu treasures nu exista\n"));
        exit(2);
    }
    char buffer[1024];
    int bytes; //Numarul de bytes cititi
    off_t fileSize=lseek(fileDescriptor,0,SEEK_END); //Pentru dimensiunea buffer-ului in care se citeste continutul fisierului
    char *fileContent = malloc(fileSize + 1);
    if (!fileContent) {
        write(1, "Eroare la alocarea memoriei\n", strlen("Eroare la alocarea memoriei\n"));
        exit(2);
    }
    int contentPosition=0; //Folosit pentru buffer-ul in care am continutul
    lseek(fileDescriptor,0,SEEK_SET);
    while((bytes=read(fileDescriptor,buffer,512))>0)
    {
        for(int i=0;i<strlen(buffer);i++)
        {
            fileContent[contentPosition++]=buffer[i];
        }
    }
    fileContent[contentPosition]='\0';
    close(fileDescriptor);
    fileDescriptor=open(pathFile,O_WRONLY | O_TRUNC); //deschis pentru scriere
    if(fileDescriptor<0)
    {
        write(1,"Fisierul nu s-a putut redeschide\n",strlen("Fisierul nu s-a putut redeschide\n"));
        exit(2);
    }
    char line[1024];
    int linePosition=0;
    int i=0;
    while(i<contentPosition)
    {
        if(fileContent[i]=='\n')
        {
            if(strncmp(line,id,strlen(id))==0 && line[strlen(id)] == ' ' ) //Daca nu s-a potrivit id-ul
            {
                write(1,"Acest treasure va fi sters\n",strlen("Acest treasure va fi sters\n"));
                write(1,line,strlen(line));
                write(1,"\n",1);
            }
            else
            {
                write(fileDescriptor,line,linePosition);
                write(fileDescriptor,"\n",1);
            }
            linePosition=0;
        }
        else
        {
            line[linePosition++]=fileContent[i];
        }
        i++;
    }
    char logFile[50]; //Contine calea pentru fisierele din director
    strcpy(logFile,hunt_id);
    strcat(logFile,"/logged_hunt.txt");
    int logDescriptor=open(logFile, O_RDWR | O_APPEND);   
    if(logDescriptor<0)
    {
        //write(1,"Fisierul nu exista\n",strlen("Fisierul nu exista\n"));
        if((logDescriptor = open(logFile, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
        {
            //write(1,"Fisierul nu poate fi creat\n",strlen("Fisierul nu poate fi creat\n"));
            exit(2);
        }
        else
        {
            write(1, "Fisierul de log a fost creat", strlen("Fisierul de log a fost creat\n"));
        }
    }
    char message[512];
    strcpy(message,"--remove_treasure ");
    strcat(message, hunt_id);
    strcat(message," ");
    strcat(message,id);
    strcat(message,"\n");
    write(logDescriptor,message,strlen(message));
    free(fileContent); 
    close(logDescriptor);
    close(fileDescriptor);
}

void remove_hunt(const char *hunt_id)
{
    struct stat goodStat;
    //0 on success, -1 on failure
    int checkDir=stat(hunt_id,&goodStat); //Verific daca hunt-id-ul exista deja
    if(checkDir!=0)
    {
        write(1,"Directorul nu exista\n",strlen("Directorul nu exista\n"));
        exit(2);
    }
    char pathFile[50]; //Contine calea pentru fisierele din director
    strcpy(pathFile,hunt_id);
    strcat(pathFile,"/treasures.dat");
    if(remove(pathFile)!=0)
    {
        write(1,"Fisierul nu s-a putut sterge\n",strlen("Fisierul nu s-a putut sterge\n"));
        exit(2);
    }
    char logFile[50]; //Contine calea pentru fisierele din director
    strcpy(logFile,hunt_id);
    strcat(logFile,"/logged_hunt.txt");
    if(remove(logFile)!=0)
    {
        write(1,"Fisierul nu s-a putut sterge\n",strlen("Fisierul nu s-a putut sterge\n"));
        exit(2);
    }
    char pathLink[50];
    strcpy(pathLink,"logged_hunt-");
    strcat(pathLink,hunt_id);
    if(remove(pathLink)!=0)
    {
        write(1,"Legatura nu s-a putut sterge\n",strlen("Legatura nu s-a putut sterge\n"));
        exit(2);
    }
    if(remove(hunt_id)!=0)
    {
        write(1,"Nu s-a putut sterge directorul\n",strlen("Nu s-a putut sterge directorul\n"));
        exit(2);
    }
    else
    {
        write(1,"Directorul dat a fost sters\n",strlen("Directorul dat a fost sters\n"));
    }
    
}

int main(int argc,char **argv)
{ char nrArg[50];
    strcpy(nrArg,"Numar invalid de argumente\n");
    if(strcmp(argv[1],"--add")==0)
    {
        if(argc!=3)
        {
            write(1,nrArg,strlen(nrArg));
        }
        else
        {
            const char* hunt_id=argv[2];
            add(hunt_id);
        }
    }
    else
    if(strcmp(argv[1],"--list")==0)
    {
        if(argc!=3)
        {
            write(1,nrArg,strlen(nrArg));
        }
        else
        {
            const char* hunt_id=argv[2];
            list(hunt_id);
        }
    }
    else
    if(strcmp(argv[1],"--view")==0)
    {
        if(argc!=4)
        {
            write(1,nrArg,strlen(nrArg));
        }
        else
        {
            const char* hunt_id=argv[2];
            const char* id=argv[3];
            view(hunt_id,id);
        }
    }
    else
    if(strcmp(argv[1],"--remove_treasure")==0)
    {
        if(argc!=4)
        {
            write(1,nrArg,strlen(nrArg));
        }
        else
        {
            const char* hunt_id=argv[2];
            const char* id=argv[3];
            remove_treasure(hunt_id,id);
        }
    }
    else
    if(strcmp(argv[1],"--remove_hunt")==0)
    {
        if(argc!=3)
        {
            write(1,nrArg,strlen(nrArg));
        }
        else
        {
            const char* hunt_id=argv[2];
            remove_hunt(hunt_id);
        }
    }
    else
        write(1,"Operatie invalida\n",strlen("Operatie invalida\n"));
    return 0;
}