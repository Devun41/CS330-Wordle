#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <strings.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#define MAX_PENDING 5 
#define MAX_SIZE 2048 

typedef enum { F, T } boolean;

int 
main(int argc, char** argv) 
{ 
    FILE *fp;
    // variables for streak txt file
    FILE* streakP;
    
    struct sockaddr_in sin;
    struct sockaddr_in cliAddr; 
    char buf[MAX_SIZE]; 
    int buf_len; 
    socklen_t addr_len; 
    int s, new_s; 
    char *port,*init_dir,*end_dir; 
    if(argc==2) { 
        port = argv[1]; 
    } 
    else { 
        fprintf(stderr, "usage: %s port\n", argv[0]); 
        exit(1); 
    }
    /* build address data structure */ 
    memset(&sin,'0',sizeof(sin));
    bzero((char *)&sin, sizeof(sin)); 
    sin.sin_family = AF_INET; 
    sin.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    sin.sin_port = htons(atoi(port)); 
    /* setup passive open */ 
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {  perror("simplex-talk: socket"); 
        exit(1); 
    } 
    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {  perror("simplex-talk: bind"); 
        exit(1); 
    } 
    listen(s, 10); 
    /* wait for connection, then receive and print text */  
    while(1) { 
        if ((new_s = accept(s, (struct sockaddr *)&cliAddr, &addr_len)) < 0) {  
            fprintf(stderr, "error: %s accept\n", argv[0]);
            exit(1); 
        }
        if(fork() == 0)
        {
            boolean win;
            int numLetters, correctLetters;
            char wordToGuess[20];
            char returnString[20];
            //placeholder for words
            char wordguy[MAX_SIZE];

            char numToString[MAX_SIZE];

            int streakHold[8]; //temporary array to hold high score steaks
            int currentStreak = 0; //players current game streak

            if(recv(new_s, buf, sizeof(buf), 0)<0)
            {
                close(new_s);
                break;
            }
            buf[strlen(buf)-1] = '\0';
            numLetters = atoi(buf);
            while(1)
            {
                for(int k=0;k<20;k++)
                    wordToGuess[k] = '\0';
                fprintf(stdout, "letters: %sL.txt \n", buf);
                //select word from word database
                //opens txt file based on number of letters wanted, can and will read and write file
                switch(numLetters)
                {
                    case 3:
                        fp=fopen("WordDatabase/3L.txt","r+");
                        break;
                    case 4:
                        fp=fopen("WordDatabase/4L.txt","r+");
                        break;
                    case 5:
                        fp=fopen("WordDatabase/5L.txt","r+");
                        break;
                    case 6:
                        fp=fopen("WordDatabase/6L.txt","r+");
                        break;
                    case 7:
                        fp=fopen("WordDatabase/7L.txt","r+");
                        break;
                    case 8:
                        fp=fopen("WordDatabase/8L.txt","r+");
                        break;
                    case 9:
                        fp=fopen("WordDatabase/9L.txt","r+");
                        break;
                    case 10:
                        fp=fopen("WordDatabase/10L.txt","r+");
                        break;
                    default:
                        close(new_s); //default case in case of error
                        return 0;
                }
                //if file isnt found, print an error and exit the program
                if(!fp){
                    //printf(strcat("ERROR: Could not find ", buf, "letter database file."));
                    fflush(stdout);
                    close(new_s);
                    break;
                }
                //gets the word to guess
                fgets(wordToGuess, 20, fp);
                //below puts wordToGuess at bottom of file/database for the next usable word next time.
                int z=0;
                while(fgets(wordguy, MAX_SIZE, fp))
                {
                    fseek(fp,(z*(numLetters+1)),SEEK_SET);
                    fprintf(fp,"%s",wordguy);
                    fseek(fp,numLetters+1,SEEK_CUR);
                    z++;
                }
                fseek(fp,(z*(numLetters+1)),SEEK_SET);
                fprintf(fp,"%s",wordToGuess);
                fclose(fp);

                for(int k=MAX_SIZE-1;k>=0;k--)
                    wordguy[k] = '\0';
                //remove the newline character from string for sake of guessing.
                wordToGuess[strlen(wordToGuess)-1] = '\0';
                fprintf(stdout, "Word: %s... Waiting for Guess\n", wordToGuess); //only seen by server
                //set boolean to false
                win = F;
                //Give player 6 guesses
                for(int i=0;i<6;i++)
                {
                    recv(new_s, buf, sizeof(buf), 0);
                    //remove newline from buffer for guessing
                    buf[strlen(buf)-1] = '\0';
                    //make all of the return string empty
                    for(int k=20;k>=0;k--)
                        returnString[k] = '\0';
                    //track correct letters. If all correct, then game must end.
                    correctLetters = 0;
                    for(int j=0;j<numLetters;j++)
                    {
                        if(tolower(buf[j]) == wordToGuess[j])
                        {
                            correctLetters++;
                            returnString[j] = '*';
                        }
                        else
                        {
                            returnString[j] = '_';
                            for(int l=0;l<numLetters;l++)
                            {
                                if(tolower(buf[j])==wordToGuess[l])
                                {
                                    returnString[j] = '~';
                                    break;
                                }
                            }
                        }
                    }
                    fprintf(stdout, "Result: %s\n", returnString);
                    if(send(new_s, returnString, strlen(returnString), 0) < 0) { 
                        perror("client: send"); 
                    }
                    if(correctLetters >= numLetters)
                    {
                        win = T;
                        currentStreak++;//if they won add to streak
                        break;
                    }
                }
                recv(new_s, buf, sizeof(buf), 0);
                wordToGuess[strlen(wordToGuess)] = '\n';
                if(win == F)
                {
                    strcat(wordguy, "Game Over! You Didn't Guess the word!\nThe Word was: ");
                    strcat(wordguy, wordToGuess);
                    if(send(new_s, wordguy, strlen(wordguy),0) < 0)
                    {
                        perror("client: send");
                    }
                }
                else
                {

                    streakP = fopen("StreakHighScore.txt", "r+");
                    int num;
                    int i = 0;
                    //puts the current records into an array
                    while (fscanf(streakP, "%i", &num) > 0) {
                        if (!streakP) {
                            perror("Could not open StreakHighScore.txt.");
                            break;
                        }
                        streakHold[i] = num;
                        i++;
                    }
                    fclose(streakP);
                    streakP = fopen("StreakHighScore.txt", "w+");
                    //checks if the current record for the players chosen letters in word is more than the one stored in the database and if it is, change it
                    if (streakHold[numLetters - 3] < currentStreak) {
                        streakHold[numLetters - 3] = currentStreak;
                    }
                    for(int i=0;i<8;i++)
                    {
                        fprintf(streakP, "%i\n", streakHold[i]);
                    }
                    fclose(streakP);

                    strcat(wordguy, "Game Over! You Guessed Correctly!\nThe Word was: ");
                    strcat(wordguy, wordToGuess);
                    if (streakHold[numLetters - 3] == currentStreak) {
                        strcpy(numToString, "");
                        strcat(wordguy, "\nYou hold the new record for ");
                        sprintf(numToString,"%d", numLetters);
                        strcat(wordguy, numToString);
                        strcpy(numToString, "");
                        strcat(wordguy, " letters at ");
                        sprintf(numToString,"%d", streakHold[numLetters-3]);
                        strcat(wordguy, numToString);
                    }
                    else {
                        strcpy(numToString, "");
                        strcat(wordguy, "\nThe streak for ");
                        sprintf(numToString,"%d", numLetters);
                        strcat(wordguy, numToString);
                        strcpy(numToString, "");
                        strcat(wordguy, " letters is ");
                        sprintf(numToString,"%d", streakHold[numLetters-3]);
                        strcat(wordguy, numToString);
                        //strcat(streakguy, "The streak for " + numLetters + " letters is " + streakHold[numLetters - 3]);
                    }
                    wordguy[strlen(wordguy)] = '\n';

                    if(send(new_s, wordguy, strlen(wordguy),0) < 0)
                    {
                        perror("client: send");
                    }
                }
                printf("hello working?");
                fflush(stdout);
                recv(new_s, buf, sizeof(buf), 0);
                if(buf[0] == 'n' || buf[0] == 'N')
                {
                    fflush(stdout);
                    close(new_s);
                    return 0;
                }
            }
            fflush(stdout);
            close(new_s); //closing the socket in the child so it is not remained open
            break;
        }
        else
        {
            close(new_s); //closing the socket in the parent so a new client can connect
            continue;
        }
    } 
    close(s); 
    return 0; 
}
/* Code that didnt survive your commit
    FILE *fp;
    // variables for streak txt file
    FILE *streakP;
    int streakHold[8]; //temporary array to hold high score steaks
    int currentStreak=0;

    struct sockaddr_in sin;
    struct sockaddr_in cliAddr; 
    char buf[MAX_SIZE]; 
    int buf_len; 
    socklen_t addr_len; 
    int s, new_s; 
    char *port,*init_dir,*end_dir; 
    if(argc>1) { 
        fprintf(stderr, "usage: %s \n", argv[0]); 
        exit(1); 
    }
    port = "12000";
     //build address data structure 
memset(&sin, '0', sizeof(sin));
bzero((char*)&sin, sizeof(sin));
sin.sin_family = AF_INET;
sin.sin_addr.s_addr = inet_addr("127.0.0.1");
sin.sin_port = htons(atoi(port));
//setup passive open 
if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("simplex-talk: socket");
    exit(1);
}
if ((bind(s, (struct sockaddr*)&sin, sizeof(sin))) < 0) {
    perror("simplex-talk: bind");
    exit(1);
}
listen(s, 10);
/* wait for connection, then receive and print text 
while (1) {
    if ((new_s = accept(s, (struct sockaddr*)&cliAddr, &addr_len)) < 0) {
        fprintf(stderr, "error: %s accept\n", argv[0]);
        exit(1);
    }
    if (fork() == 0)
    {
        boolean win;
        int numLetters, correctLetters;
        char wordToGuess[20];
        char returnString[20];
        //placeholder for words
        char wordguy[100];
        while (1)
        {
            for (int k = 0; k < 20; k++)
                wordToGuess[k] = '\0';
            recv(new_s, buf, sizeof(buf), 0);
            buf[strlen(buf) - 1] = '\0';
            numLetters = atoi(buf);
            fprintf(stdout, "letters: %sL.txt \n", buf);
            //select word from word database
            //opens txt file based on number of letters wanted, can and will read and write file
            switch (numLetters)
            {
            case 3:
                fp = fopen("WordDatabase/3L.txt", "r+");
                break;
            case 4:
                fp = fopen("WordDatabase/4L.txt", "r+");
                break;
            case 5:
                fp = fopen("WordDatabase/5L.txt", "r+");
                break;
            case 6:
                fp = fopen("WordDatabase/6L.txt", "r+");
                break;
            case 7:
                fp = fopen("WordDatabase/7L.txt", "r+");
                break;
            case 8:
                fp = fopen("WordDatabase/8L.txt", "r+");
                break;
            case 9:
                fp = fopen("WordDatabase/9L.txt", "r+");
                break;
            case 10:
                fp = fopen("WordDatabase/10L.txt", "r+");
                break;
            default:
                fp = fopen("WordDatabase/3L.txt", "r+"); //default case in case of error
                break;
            }
            //if file isnt found, print an error and exit the program
            if (!fp) {
                //printf(strcat("ERROR: Could not find ", buf, "letter database file."));
                fflush(stdout);
                close(new_s);
                break;
            }
            //gets the word to guess
            fgets(wordToGuess, 20, fp);
            //below puts wordToGuess at bottom of file/database for the next usable word next time.
            int z = 0;
            while (fgets(wordguy, 100, fp))
            {
                fseek(fp, (z * (numLetters + 1)), SEEK_SET);
                fprintf(fp, "%s", wordguy);
                fseek(fp, numLetters + 1, SEEK_CUR);
                z++;
            }
            fseek(fp, (z * (numLetters + 1)), SEEK_SET);
            fprintf(fp, "%s", wordToGuess);
            fclose(fp);

            for (int k = 0; k < 100; k++)
                wordguy[k] = '\0';
            //remove the newline character from string for sake of guessing.
            wordToGuess[strlen(wordToGuess) - 1] = '\0';
            fprintf(stdout, "Word: %s... Waiting for Guess\n", wordToGuess); //only seen by server
            //set boolean to false
            win = F;
            //Give player 6 guesses
            for (int i = 0; i < 6; i++)
            {
                recv(new_s, buf, sizeof(buf), 0);
                //remove newline from buffer for guessing
                buf[strlen(buf) - 1] = '\0';
                //make all of the return string empty
                for (int k = 0; k < 20; k++)
                    returnString[k] = '\0';
                //track correct letters. If all correct, then game must end.
                correctLetters = 0;
                for (int j = 0; j < numLetters; j++)
                {
                    if (buf[j] == wordToGuess[j])
                    {
                        correctLetters++;
                        returnString[j] = '*';
                    }
                    else
                    {
                        returnString[j] = '_';
                        for (int l = 0; l < numLetters; l++)
                        {
                            if (buf[j] == wordToGuess[l])
                            {
                                returnString[j] = '~';
                                break;
                            }
                        }
                    }
                }
                fprintf(stdout, "Result: %s\n", returnString);
                if (send(new_s, returnString, strlen(returnString), 0) < 0) {
                    perror("client: send");
                }
                if (correctLetters >= numLetters)
                {
                    win = T;
                    currentStreak++;//if they won add to streak
                    break;
                }
            }
            wordToGuess[strlen(wordToGuess)] = '\n';
            //GAME OVER check
            if (win == F)
            {
                strcat(wordguy, "Game Over! You Didn't Guess the word!\nThe Word was: ");
                strcat(wordguy, wordToGuess);

                6

                    if (send(new_s, wordguy, strlen(wordguy), 0) < 0)
                    {
                        perror("client: send");
                    }
            }
            else
            {
                strcat(wordguy, "Game Over! You Guessed Correctly!\nThe Word was: ");
                strcat(wordguy, wordToGuess);

                streakP = fopen("StreakHighScore.txt", "r+");
                int num;
                int i = 0;
                //puts the current records into an array
                while (fscanf(streakP, "%i", &num) > 0) {
                    if (!streakP) {
                        perror("Could not open SterakHighScore.txt.");
                        break;
                    }
                    streakHold[i] = num;
                    i++;
                }
                //checks if the current record for the players chosen letters in word is more than the one stored in the database and if it is, change it
                if (streakHold[numLetters - 3] < currentStreak) {
                    streakHold[numLetters - 3] = currentStreak;
                }
                fwrite(streakHold, sizeof(int), sizeof(streakHold), streakP); //rewrites the StreakHighScore text to the current records
                fclose(streakP);
                if (send(new_s, wordguy, strlen(wordguy), 0) < 0)
                {
                    perror("client: send");
                }
            }
            fflush(stdout);
            recv(new_s, buf, sizeof(buf), 0);
            if (buf[0] == 'n' || buf[0] == 'N')
            {
                fflush(stdout);
                close(new_s);
                return 0;
            }
        }
        fflush(stdout);
        close(new_s); //closing the socket in the child so it is not remained open
        break;
    }
    else
    {
        close(new_s); //closing the socket in the parent so a new client can connect
        continue;
    }
}
close(s);
return 0;
}
*/