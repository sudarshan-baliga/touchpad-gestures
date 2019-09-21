#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define TOUCHPAD_FILE "/dev/input/event11"

void switchTabs(bool right)
{

}


void handleTrackPad()
{
        int fd;
        bool twoFingers = false;
        struct input_event ie;
        if ((fd = open(TOUCHPAD_FILE, O_RDONLY)) == -1)
        {
                perror("Could not open the touchpad event file");
                exit(EXIT_FAILURE);
        }
        while (read(fd, &ie, sizeof(struct input_event)))
        {

                switch (ie.code)
                {
                case BTN_TOOL_FINGER:
                        /* code */
                        printf("Finger\n");
                        break;
                case BTN_TOOL_DOUBLETAP:
                        /* code */
                        printf("Double\n");
                        twoFingers = !twoFingers;
                        break;
                case ABS_X:
                        if(twoFingers)
                                printf("Switch Tabs Here %d\n ", ie.value);
                        break;
                default:
                        break;
                }
        }
}

int main()
{
        handleTrackPad();
        return 0;
}
