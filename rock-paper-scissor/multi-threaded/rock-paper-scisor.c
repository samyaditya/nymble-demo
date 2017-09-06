#include "header.h"
#include<gtk/gtk.h>  

int p1[2], p2[2];
pthread_mutex_t t1_mutex, t2_mutex;
uint8_t t1_sendRandNum = 0, t2_sendRandNum = 0;

GtkTextBuffer *buffer;
GtkTextIter iter;

void hello(GtkWidget* widget, gpointer data){  
    //g_print("Hello World\n");  
    //gtk_text_buffer_insert(buffer, &iter, "================== long ========== sample ============== text ==================\n", -1);
    pthread_mutex_unlock(&t1_mutex);
    t1_sendRandNum = 1;
    pthread_mutex_lock(&t1_mutex);
    pthread_mutex_unlock(&t2_mutex);
    t2_sendRandNum = 1;
    pthread_mutex_lock(&t2_mutex);
}  
gint delete_event_handler(GtkWidget* widget, GdkEvent* event, gpointer data){  
    g_print("delete event occured\n");  
    return FALSE;  
}  
void destroy(GtkWidget* widget, gpointer data){  
    gtk_main_quit();  
}

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

void *thread1(void *arg)
{
    printf("In thread1 , pid = %d, threadID = %d\n", getpid(), pthread_self());
    while(1) {
        pthread_mutex_unlock(&t1_mutex);
        if (t1_sendRandNum) {
            int randNum = genRandNum();
            write(p1[1], &randNum, sizeof(randNum));
            t1_sendRandNum = 0;
        }
        pthread_mutex_lock(&t1_mutex);
        usleep(10000);
    }
}

void *thread2(void *arg)
{
    printf("In thread2 , pid = %d, threadID = %d\n", getpid(), pthread_self());
    while(1) {
        pthread_mutex_unlock(&t2_mutex);
        if (t2_sendRandNum) {
            int randNum = genRandNum();
            write(p2[1], &randNum, sizeof(randNum));
            t2_sendRandNum = 0;
        }
        pthread_mutex_lock(&t2_mutex);
        usleep(10000);
    }
}

void *threadGtk(void *arg)
{
    printf("In threadGtk , pid = %d, threadID = %d\n", getpid(), pthread_self());
    gtk_main();  
}

int main(int argc, char **argv)
{
    pid_t c1_pid, c2_pid;
    int c1_res, c2_res, round = 0;
    float c1_total = 0, c2_total = 0;
    char tmpStr[1024] = {0};
    if ((pipe(p1) < 0) || (pipe(p2) < 0)) {
        perror("pipe");
        return -1;
    }

    pthread_mutex_init(&t1_mutex, NULL);
    pthread_mutex_init(&t2_mutex, NULL);
    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);






    /////////////////////////////////////////
    GtkWidget *window;
    GtkWidget *button;  
    GtkWidget *win2;
    GtkWidget *view;
    GtkWidget *vbox;
    GtkTextIter start, end;
    gtk_init(&argc, &argv);  
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
    
    
    
    win2 = gtk_window_new(GTK_WINDOW_TOPLEVEL);  

    gtk_window_set_position(GTK_WINDOW(win2), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(win2), 300, 200);
    gtk_window_set_title(GTK_WINDOW(win2), "GtkTextView");
    

    vbox = gtk_vbox_new(FALSE, 0);
    view = gtk_text_view_new();
    gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 0);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
    //gtk_text_buffer_insert(buffer, &iter, "Plain text\n", -1);

    gtk_container_add(GTK_CONTAINER(win2), vbox);




    g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(delete_event_handler), NULL);  
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);  
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);  
    button = gtk_button_new_with_label("Hello World");  
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(hello), NULL);  
    gtk_container_add(GTK_CONTAINER(window), button);  
    gtk_widget_show(button);  
    gtk_widget_show(window);  
    gtk_widget_show_all(win2);
    pthread_create(&t3, NULL, threadGtk, NULL);
    /////////////////////////////////////////







    printf("In Parent , pid = %d, threadID = %d\n", getpid(), pthread_self());
    usleep(100000);
    printf("============================================================\n");
    while(c1_total <= 10.0f && c2_total <= 10.0f) {
        read(p1[0], &c1_res, sizeof(c1_res));
        read(p2[0], &c2_res, sizeof(c2_res));
        printf("Round: %02d,\tc1: %s,\tc2: %s,\t", ++round, get_enum_string(c1_res), get_enum_string(c2_res));
        sprintf(tmpStr, "Round: %02d,\tc1: %s,\tc2: %s,\t", round, get_enum_string(c1_res), get_enum_string(c2_res));
        gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
        if (c1_res == PAPER && c2_res == SCISOR) {
            c2_total+=1.0f;
            printf("c2 wins\n");
            sprintf(tmpStr, "c2 wins\n");
            gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
        } else if (c1_res == SCISOR && c2_res == PAPER) {
            c1_total+=1.0f;
            printf("c1 wins\n");
            sprintf(tmpStr, "c1 wins\n");
            gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
        } else if (c1_res == SCISOR && c2_res == ROCK) {
            c2_total+=1.0f;
            printf("c2 wins\n");
            sprintf(tmpStr, "c2 wins\n");
            gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
        } else if (c1_res == ROCK && c2_res == SCISOR) {
            c1_total+=1.0f;
            printf("c1 wins\n");
            sprintf(tmpStr, "c1 wins\n");
            gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
        } else if (c1_res == PAPER && c2_res == ROCK) {
            c1_total+=1.0f;
            printf("c1 wins\n");
            sprintf(tmpStr, "c1 wins\n");
            gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
        } else if (c1_res == ROCK && c2_res == PAPER) {
            c2_total+=1.0f;
            printf("c2 wins\n");
            sprintf(tmpStr, "c2 wins\n");
            gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
        } else if (c1_res == c2_res) {
            c1_total+=0.5f;
            c2_total+=0.5f;
            printf("it's a tie\n");
            sprintf(tmpStr, "it's a tie\n");
            gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
        }
    }
    printf("------------------------------------------------------------\n");
    sprintf(tmpStr, "------------------------------------------------------------\n");
    gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
    printf("Final Score:\tc1(%d): %g,\tc2(%d): %g,\t", c1_pid, c1_total, c2_pid, c2_total);
    sprintf(tmpStr, "Final Score:\tc1(%d): %g,\tc2(%d): %g,\t", c1_pid, c1_total, c2_pid, c2_total);
    gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
    if (c1_total > c2_total) {
        printf("c1 wins\n");
        sprintf(tmpStr, "c1 wins\n");
        gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
    } else if (c1_total < c2_total) {
        printf("c2 wins\n");
        sprintf(tmpStr, "c2 wins\n");
        gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
    } else if (c1_total == c2_total) {
        printf("equal score\n");
        sprintf(tmpStr, "equal score\n");
        gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
        srand(getpid());
        if (rand() > rand()) {
            printf("master's chice c1 wins\n");
            sprintf(tmpStr, "master's chice c1 wins\n");
            gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
        } else {    // Very unlikely to get same random number b/w 0 and RAND_MAX
            printf("master's choice c2 wins\n");
            sprintf(tmpStr, "master's choice c2 wins\n");
            gtk_text_buffer_insert(buffer, &iter, tmpStr, -1);
        }
    }
    printf("============================================================\n");

    return 0;
}
