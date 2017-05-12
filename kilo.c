#include<ctype.h>
#include<stdio.h>
#include<unistd.h>
#include<termios.h>
#include<stdlib.h>
#include<errno.h>

/*** defines ***/

#define CTRL_KEY(k) ((k) & 0x1f)

struct termios original_termios;

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

    // in stdlib.h
    // Called when program exits from main() or exit()
    // restore where the original terminal settings
    atexit(disableRawMode);

    // Copy of original structure to prevent changes in original
    struct termios raw = original_termios;
    
    // lflag is miscellaneous flag 
    // Echo is a bit flag with 1000 in the end
    // Anded with 0111 to make it 0000
    // ICANON - turn off canonical mode - we will read byte by byte
    // ICANON instead of line by line.
    // ICANON is a local flag not input flag
    // generally flags in c_iflag start with I.
    // Turn off ctrl-c send SIGINT, ctrl-z sends SIGSTP signals resp.
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);

    // ctrl + s : stop data transfer to terminal
    // ctrl + q : resume data flow to terminal
    // IEXTEN Flag : disable ctrl + v 
    // ICRNL : Disable ctrl + m  as 10
    // I : Input, CR: Carriage Return, NL : New Line
    raw.c_iflag &= ~(IXON | IEXTEN | ICRNL);

    // Flags for older terminal emulators.
    // Turned off in new ones
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // Turn off all output processing
    raw.c_oflag &= ~(OPOST);

    // Minimum bytes to read before it returns
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    // Write the new struct 
    // TCSAFLUSH - Clears any input left and not fed into shell
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}



int main(){
    enableRawMode();
    char c;
    while(1)
    {
        c = '\0';
        // read() 1 byte from standard input stream
        if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        // iscntrl() in ctype.h
        // iscntrl checks if  char is control char
        // ASCII 0-31 are control and 127 too.
        // page up sends 4 byte to terminal starting with esc
        // ctrl - s : freeze ctrl - q : resume
        // printf() in stdio.h
        if(iscntrl(c)){
            printf("%d\r\n", c);
        } else { 
            printf("%d ('%c'\r)\n", c, c);
        }
        if(c == CTRL_KEY('q')) break;
    }


    return 0;
}
