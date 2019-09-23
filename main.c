#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define TOUCHPAD_FILE "/dev/input/event11"
bool altDown, isVerticalScroll, desktopShow;
int TRIGGER_SMOOTHNESS = 25, RUNNING_SMOOTHNESS = 30, VERTICAL_SCROLLL_CHECK_DIST = 3;
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

void showDesktop()
{
        system("xdotool key Alt+Ctrl+d");
}

void handleTrackPad()
{
        int fd, threeFingersEveCount, fourFingersEveCount;
        bool threeFingers = false, fourFingers = false;
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
                case BTN_TOOL_TRIPLETAP:
                        threeFingers = !threeFingers;
                        if (!threeFingers)
                        {
                                threeFingersEveCount = 0;
                                altDown = false;
                                system("xdotool keyup Alt");
                                curSmoothness = TRIGGER_SMOOTHNESS;
                        }
                case BTN_TOOL_QUADTAP:
                        fourFingers = !fourFingers;
                        if(!fourFingers)
                        {
                                fourFingersEveCount = 0;
                                desktopShow = false;
                                curSmoothness = TRIGGER_SMOOTHNESS;
                        }
                case ABS_Y:
                        if (threeFingers && threeFingersEveCount % curSmoothness == 0)
                        {
                                if (abs(ie.value - preY) >= VERTICAL_SCROLLL_CHECK_DIST && !desktopShow)
                                {
                                        isVerticalScroll = true;
                                        desktopShow = true;
                                        showDesktop();
                                }
                                else
                                        isVerticalScroll = false;
                        }
                        preY = ie.value;
                        break;
                case ABS_X:
                        if (ie.value == 0)
                                continue;
                        if (threeFingers && threeFingersEveCount % curSmoothness == 0 && !isVerticalScroll)
                        {
                                if (ie.value > preX)
                                        switchTabs(true);
                                else
                                        switchTabs(false);
                        }
                        preX = ie.value;
                        threeFingersEveCount++;
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
