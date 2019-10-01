#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>

#define TRIGGER_SMOOTHNESS 1
#define RUNNING_SMOOTHNESS 39
#define VERTICAL_SCROLL_CHECK_DIST 8
#define RELATIVE_ORIGIN -1

bool altDown, isVerticalScroll, desktopShow, desktopSwitched;
int curSmoothness;

char *getEventFile()
{
        FILE *fp;
        char *eventFile = malloc(sizeof(char) * 10);
        fp = popen("awk 'BEGIN{FS=\"\\n\";RS=\"\\n\\n\"}{if($2 ~ /Touchpad/) {  split($6, chars, \" \"); print(chars[3])}}' /proc/bus/input/devices ", "r");
        fscanf(fp, "%s", eventFile);
        return eventFile;
}

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
        }
        printf("Switching to previous tab\n");
}

void showDesktop()
{
        system("xdotool key Alt+Ctrl+d");
}

void switchDesktop(bool next)
{
        if (next)
                system("xdotool key Ctrl+Alt+Down");
        else
                system("xdotool key Ctrl+Alt+Up");
}

void handleTrackPad()
{
        int fd, threeFingersEveCount, fourFingersEveCount;
        bool threeFingers = false, fourFingers = false;
        struct input_event ie;
        char TOUCHPAD_FILE[] = "/dev/input/";
        int preX, preY,
            curSmoothness = TRIGGER_SMOOTHNESS;

        //get the touchpad event file
        strcat(TOUCHPAD_FILE, getEventFile());

        if ((fd = open(TOUCHPAD_FILE, O_RDONLY)) == -1)
        {
                perror("Could not open the touchpad event file");
                exit(EXIT_FAILURE);
        }
        while (read(fd, &ie, sizeof(struct input_event)))
        {
                if (threeFingers)
                        threeFingersEveCount++;
                else if (fourFingers)
                        fourFingersEveCount++;
                switch (ie.code)
                {
                case BTN_TOOL_TRIPLETAP:
                        threeFingers = !threeFingers;
                        threeFingersEveCount = 0;
                        if (!threeFingers)
                        {
                                altDown = false;
                                desktopShow = false;
                                system("xdotool keyup Alt");
                                curSmoothness = TRIGGER_SMOOTHNESS;
                                preX = RELATIVE_ORIGIN;
                                preY = RELATIVE_ORIGIN;
                        }
                        break;
                case BTN_TOOL_QUADTAP:
                        fourFingers = !fourFingers;
                        fourFingersEveCount = 0;
                        printf("four finger\n");
                        if (!fourFingers)
                        {
                                desktopSwitched = false;
                                curSmoothness = TRIGGER_SMOOTHNESS;
                                preX = RELATIVE_ORIGIN;
                                preY = RELATIVE_ORIGIN;
                        }
                        break;
                case ABS_Y:
                        if (preY != RELATIVE_ORIGIN && abs(ie.value - preY) >= VERTICAL_SCROLL_CHECK_DIST)
                        {
                                isVerticalScroll = true;
                        }
                        else
                                isVerticalScroll = false;
                        if (threeFingers && threeFingersEveCount % curSmoothness == 0 && !desktopShow && isVerticalScroll)
                        {
                                desktopShow = true;
                                curSmoothness = RUNNING_SMOOTHNESS;
                                showDesktop();
                        }
                        preY = ie.value;
                        break;
                case ABS_X:
                        if (ie.value == 0)
                                continue;
                        if (preX != RELATIVE_ORIGIN && threeFingers && threeFingersEveCount % curSmoothness == 0 && !isVerticalScroll)
                        {
                                curSmoothness = RUNNING_SMOOTHNESS;
                                if (ie.value > preX)
                                        switchTabs(true);
                                else
                                        switchTabs(false);
                        }
                        else if (preX != RELATIVE_ORIGIN && fourFingers && fourFingersEveCount % curSmoothness == 0 && !isVerticalScroll && !desktopSwitched)
                        {
                                curSmoothness = RUNNING_SMOOTHNESS;
                                desktopSwitched = true;
                                if (ie.value > preX)
                                        switchDesktop(true);
                                else
                                        switchDesktop(false);
                        }
                        preX = ie.value;
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
