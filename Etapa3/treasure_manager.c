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
struct Treasure t;

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
            t.id=atoi(check);
            strcpy(tempBuffer,tempLine);
            strcat(tempBuffer," ");
        }
        if(i==2)
        {
            strcpy(t.username,tempLine);
            strcat(tempBuffer,tempLine);
            strcat(tempBuffer," ");
        }
        if(i==3) //Pe a treia linie latitudinea numar real
        {
            char *endptr;
            strtof(tempLine, &endptr);
            if (*endptr != '\0') break;
            t.latitude=atof(tempLine);
            strcat(tempBuffer,tempLine);
            strcat(tempBuffer," ");
        }
        if(i==4) //Pe a patra linie longitudinea numar real
        {
            char *endptr;
            strtof(tempLine, &endptr);
            if (*endptr != '\0') break;
            t.longitude=atof(tempLine);
            strcat(tempBuffer,tempLine);
            strcat(tempBuffer," ");
        }
        if(i==5)
        {
            strcpy(t.clue,tempLine);
            strcat(tempBuffer,tempLine);
            strcat(tempBuffer," ");
        }
        if(i==6) //Pe a sasea linie valoarea numar intreg
        {
            sprintf(check,"%d",atoi(tempLine));
            if(strcmp(check,tempLine)!=0)
                break;
            t.value=atoi(check);
            strcat(tempBuffer,tempLine);
            strcat(tempBuffer," ");
        }
        
    }
    if(i==6) //Daca s-au dat informatii valide
    {
        write(fileDescriptor,&t,sizeof(t));
    }
    else
    {
        write(1,"Nu s-au dat informatii valide despre treasure\n",strlen("Nu s-au dat informatii valide despre treasure\n"));
        exit(2);
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
    struct stat goodStat, fileStat;
    // 0 on success, -1 on failure
    int checkDir = stat(hunt_id, &goodStat); // Verific dacă hunt-id-ul există deja
    if (checkDir != 0)
    {
        write(1, "Directorul nu exista\n", strlen("Directorul nu exista\n"));
        exit(2);
    }

    char pathFile[50]; // Conține calea pentru fișierele din director
    strcpy(pathFile, hunt_id);
    strcat(pathFile, "/treasures.dat");

    int fileDescriptor = open(pathFile, O_RDONLY); // Deschide fișierul doar pentru citire
    if (fileDescriptor < 0)
    {
        write(1, "Fisierul cu treasures nu exista\n", strlen("Fisierul nu exista\n"));
        exit(2);
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
    struct Treasure t;
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
    char logFile[50]; // Conține calea pentru fișierele din director
    strcpy(logFile, hunt_id);
    strcat(logFile, "/logged_hunt.txt");

    int logDescriptor = open(logFile, O_RDWR | O_APPEND);
    if (logDescriptor < 0)
    {
        if ((logDescriptor = open(logFile, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1)
        {
            exit(2);
        }
        else
        {
            write(1, "Fisierul de log a fost creat", strlen("Fisierul de log a fost creat\n"));
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
            
        }
    }
    if (gasit==0)
    {
        write(1, "Comoara cu ID-ul specificat nu a fost gasita\n", strlen("Comora cu ID-ul specificat nu a fost gasita.\n"));
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
    int fileDescriptor=open(pathFile, O_RDONLY);  
    if(fileDescriptor<0)
    {
        write(1,"Fisierul cu treasures nu exista\n",strlen("Fisierul cu treasures nu exista\n"));
        exit(2);
    }
    char tempPath[100];
    strcpy(tempPath, hunt_id);
    strcat(tempPath, "/temp_treasures.dat");

    int fileTemp = open(tempPath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fileTemp < 0)
    {
        write(1, "Nu s-a putut crea fisierul temporar\n", strlen("Nu s-a putut crea fisierul temporar\n"));
        close(fileDescriptor);
        exit(2);
    }
    int treasure_id=atoi(id);
    int gasit=0;
    char num[50];
    while(read(fileDescriptor,&t,sizeof(t))>0)
    {
        if(treasure_id==t.id)
        {
            write(1,"Acest treasure va fi sters\n",strlen("Acest treasure va fi sters\n"));
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
        }
        else
        {
            write(fileTemp,&t,sizeof(t));
        }
    }
    close(fileDescriptor);
    close(fileTemp);
    if(gasit==0)
    {
        write(1, "Nu s-a gasit treasure-ul cu ID-ul specificat\n", strlen("Nu s-a gasit treasure-ul cu ID-ul specificat\n"));
        unlink(tempPath);
    }
    else
    {
        fileDescriptor = open(pathFile, O_WRONLY);
        if (fileDescriptor < 0)
        {
            write(1, "Fisierul cu treasures nu exista\n", strlen("Fisierul cu treasures nu exista\n"));
            exit(2);
        }

        fileTemp = open(tempPath, O_RDONLY);
        if (fileTemp < 0)
        {
            write(1, "Nu s-a putut deschide fisierul temporar\n", strlen("Nu s-a putut deschide fisierul temporar\n"));
            close(fileDescriptor);
            exit(2);
        }

        // Golește fișierul original
        ftruncate(fileDescriptor, 0);
        lseek(fileDescriptor, 0, SEEK_SET);

        // Copiază datele din fișierul temporar în cel original
        while (read(fileTemp, &t, sizeof(t)) > 0)
        {
            write(fileDescriptor, &t, sizeof(t));
        }
        close(fileTemp);
        unlink(tempPath); // șterge fișierul temporar
        close(fileDescriptor);
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