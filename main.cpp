#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define TOUCHPAD_FILE "/dev/input/event11"

void switchTabs(bool next)
{
        printf("Switching to previous tab\n");
        system("xdotool key Alt+Tab");
}

void handleTrackPad()
{
        int fd, smoothNess = 30, twoFingerEveCount;
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
                        printf("single Finger\n");
                        break;
                case BTN_TOOL_DOUBLETAP:
                        /* code */
                        twoFingers = !twoFingers;
                        if (!twoFingers)
                                twoFingerEveCount = 0;
                        break;
                case ABS_X:
                        if (twoFingers && twoFingerEveCount % smoothNess == 0)
                                switchTabs(true);
                        twoFingerEveCount++;
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
