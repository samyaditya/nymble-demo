#include "header.h"

int p1[2], p2[2];

/*
 * Converts enum value to string
 */
char * get_enum_string(int x)
{
    if (x == 1) return "PAPER";
    else if (x == 2) return "SCISOR";
    else if (x == 3) return "ROCK";
    else return "UNKNOWN";
}

/*
 * Generates random num from 1 to 3
 */
int genRandNum()
{
   int randNum = 0;
   randNum = ((rand() % 3) + 1);
   return(randNum);
}

/*
 * Gets triggered in child 1 when master sends 'READY' signal
 */
void sig_alrm_c1(int sig)
{
    int randNum = genRandNum();
    write(p1[1], &randNum, sizeof(randNum));
}

/*
 * Gets triggered in child 2 when master sends 'READY' signal
 */
void sig_alrm_c2(int sig)
{
    int randNum = genRandNum();
    write(p2[1], &randNum, sizeof(randNum));
}

/*
 * Gets triggered in child 1 & child 2 when master says game is over
 */
void sig_usr(int sig)
{
    char *res;
    if (sig == SIGUSR1) {
        res = "wins";
    } else if (sig == SIGUSR2) {
        res = "lose";
    }

    printf("In sig handler, child %d %s.\n", getpid(), res);
    exit(0);
}

int main(int argc, char **argv)
{
    pid_t c1_pid, c2_pid;
    int c1_res, c2_res, round = 0;
    float c1_total = 0, c2_total = 0;
    if ((pipe(p1) < 0) || (pipe(p2) < 0)) {
        perror("pipe");
        return -1;
    }

    c1_pid = fork();
    if (c1_pid == 0) {   // child 1
        printf("In Child1 , pid = %d\n", getpid());
        srand(getpid());
        signal(SIGALRM, sig_alrm_c1);
        signal(SIGUSR1, sig_usr);
        signal(SIGUSR2, sig_usr);
        getchar();
    } else {
        c2_pid = fork();
        if (c2_pid == 0) {   // child 2
            printf("In Child2 , pid = %d\n", getpid());
            srand(getpid());
            signal(SIGALRM, sig_alrm_c2);
            signal(SIGUSR1, sig_usr);
            signal(SIGUSR2, sig_usr);
            getchar();
        } else {    // parent
            printf("In Parent , pid = %d\n", getpid());
            usleep(100000);
            printf("============================================================\n");
            while(c1_total <= 10.0f && c2_total <= 10.0f) {
                kill(c1_pid, SIGALRM);
                kill(c2_pid, SIGALRM);
                read(p1[0], &c1_res, sizeof(c1_res));
                read(p2[0], &c2_res, sizeof(c2_res));
                printf("Round: %02d,\tc1: %s,\tc2: %s,\t", ++round, get_enum_string(c1_res), get_enum_string(c2_res));
                if (c1_res == PAPER && c2_res == SCISOR) {
                    c2_total+=1.0f;
                    printf("c2 wins,\tScore: c1: %g,\t\tc2: %g\n", c1_total, c2_total);
                } else if (c1_res == SCISOR && c2_res == PAPER) {
                    c1_total+=1.0f;
                    printf("c1 wins,\tScore: c1: %g,\t\tc2: %g\n", c1_total, c2_total);
                } else if (c1_res == SCISOR && c2_res == ROCK) {
                    c2_total+=1.0f;
                    printf("c2 wins,\tScore: c1: %g,\t\tc2: %g\n", c1_total, c2_total);
                } else if (c1_res == ROCK && c2_res == SCISOR) {
                    c1_total+=1.0f;
                    printf("c1 wins,\tScore: c1: %g,\t\tc2: %g\n", c1_total, c2_total);
                } else if (c1_res == PAPER && c2_res == ROCK) {
                    c1_total+=1.0f;
                    printf("c1 wins,\tScore: c1: %g,\t\tc2: %g\n", c1_total, c2_total);
                } else if (c1_res == ROCK && c2_res == PAPER) {
                    c2_total+=1.0f;
                    printf("c2 wins,\tScore: c1: %g,\t\tc2: %g\n", c1_total, c2_total);
                } else if (c1_res == c2_res) {
                    c1_total+=0.5f;
                    c2_total+=0.5f;
                    printf("it's a tie,\tScore: c1: %g,\t\tc2: %g\n", c1_total, c2_total);
                }
            }
            printf("------------------------------------------------------------\n");
            printf("Final Score:\tc1(%d): %g,\tc2(%d): %g,\t", c1_pid, c1_total, c2_pid, c2_total);
            if (c1_total > c2_total) {
                printf("c1 wins\n");
                kill(c1_pid, SIGUSR1);
                kill(c2_pid, SIGUSR2);
            } else if (c1_total < c2_total) {
                printf("c2 wins\n");
                kill(c2_pid, SIGUSR1);
                kill(c1_pid, SIGUSR2);
            } else if (c1_total == c2_total) {
                printf("equal score\n");
                srand(getpid());
                if (rand() > rand()) {
                    printf("master's chice c1 wins\n");
                    kill(c1_pid, SIGUSR1);
                    kill(c2_pid, SIGUSR2);
                } else {    // Very unlikely to get same random number b/w 0 and RAND_MAX
                    printf("master's choice c2 wins\n");
                    kill(c2_pid, SIGUSR1);
                    kill(c1_pid, SIGUSR2);
                }
            }
            printf("============================================================\n");
            waitpid(c1_pid, NULL, 0);
            waitpid(c2_pid, NULL, 0);
        }
    }

    return 0;
}
