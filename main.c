#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define TOUCHPAD_FILE "/dev/input/event11"
bool altDown;
int TRIGGER_SMOOTHNESS = 30, RUNNING_SMOOTHNESS = 40;
int preX, preY, curSmoothness;

void switchTabs(bool next)
{
        if (altDown)
        {
                if (next)
                        system("xdotool key Tab");
                else
                        system("xdotool key Shift+Tab");
        }
        else
        {
                system("xdotool keydown Alt");
                altDown = true;
                curSmoothness = RUNNING_SMOOTHNESS;
        }
        printf("Switching to previous tab\n");
}

void handleTrackPad()
{
        int fd, twoFingerEveCount;
        bool twoFingers = false;
        struct input_event ie;
        curSmoothness = TRIGGER_SMOOTHNESS;
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
                        printf("single Finger\n");
                        break;
                case BTN_TOOL_DOUBLETAP:
                        twoFingers = !twoFingers;
                        if (!twoFingers)
                        {
                                twoFingerEveCount = 0;
                                altDown = false;
                                system("xdotool keyup Alt");
                                curSmoothness = TRIGGER_SMOOTHNESS;
                        }
                        break;
                case ABS_X:
                        if (ie.value == 0)
                                continue;
                        if (twoFingers && twoFingerEveCount % curSmoothness == 0)
                        {
                                if (ie.value > preX)
                                        switchTabs(true);
                                else
                                        switchTabs(false);
                        }
                        preX = ie.value;
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
