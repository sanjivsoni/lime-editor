/*** includes ***/
#include<ctype.h>
#include<stdio.h>
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<errno.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** data ***/
struct termios original_termios;

/*** terminal ***/
void die(const char *s)
{
    perror(s);
    exit(1);
}
void disableRawMode(){
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1)
        die("tcsetattr");
}

void enableRawMode(){

    // Read terminal attributes into raw struct
    if(tcgetattr(STDIN_FILENO, &original_termios) == -1)
        die("tcsetattr");

    // Called when program exits from main() or exit()
    atexit(disableRawMode);

    struct termios raw = original_termios;
    
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);

    raw.c_iflag &= ~(IXON | IEXTEN | ICRNL);

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    raw.c_oflag &= ~(OPOST);

    // Minimum bytes to read before it returns
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    // Write the new struct 
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

char editorReadKey(){
    int nread;
    char c;
    while((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if(nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

void editorProcessKeypress()
{
    char c = editorReadKey();

    switch(c)
    {
        case CTRL_KEY('q');
        exit(0);
        break;
    }
}

/*** init ***/

int main(){
    enableRawMode();
    while(1)
    {
        editorProcessKeypress();
    }

    return 0;
}
