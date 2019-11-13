#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>

#define TRIGGER_SMOOTHNESS 1
#define RUNNING_SMOOTHNESS 40
#define VERTICAL_SCROLL_CHECK_DIST 3
#define RELATIVE_ORIGIN -1
#define ACCELARATION 1

bool altDown, isVerticalScroll, isDesktopShown, isDesktopSwitched;
int curSmoothness, curSpeed;
int preX, preY;

char *getEventFileName()
{
        FILE *fp;
        char *eventFile = malloc(sizeof(char) * 10);
        fp = popen("awk 'BEGIN{FS=\"\\n\";RS=\"\\n\\n\"}{if($2 ~ /Touchpad/) {match($6,\"event.\",eventWord); print eventWord[0]}}' /proc/bus/input/devices ", "r");
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
                curSmoothness += curSpeed;
                curSpeed += ACCELARATION;
        }
        else
        {
                system("xdotool keydown Alt");
                if (next)
                        system("xdotool key Tab");
                else
                        system("xdotool key Shift+Tab");
                altDown = true;
        }
}

void reset()
{
        altDown = false;
        isDesktopShown = false;
        isDesktopSwitched = false;
        curSmoothness = TRIGGER_SMOOTHNESS;
        curSpeed = ACCELARATION;
        preX = RELATIVE_ORIGIN;
        preY = RELATIVE_ORIGIN;
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

void handleThreeFingers(bool *threeFingers, int *threeFingersEveCount)
{
        *threeFingers = !*threeFingers;
        *threeFingersEveCount = 0;
        if (!*threeFingers)
        {
                reset();
                system("xdotool keyup Alt");
        }
}

void handleFourFingers(bool *fourFingers, int *fourFingersEveCount)
{
        *fourFingers = !*fourFingers;
        *fourFingersEveCount = 0;
        if (!*fourFingers)
                reset();
}

void handleYChange(int value, bool *threeFingers, int *threeFingersEveCount)
{
        if (value == 0)
                return;
        if (preY != RELATIVE_ORIGIN && abs(value - preY) >= VERTICAL_SCROLL_CHECK_DIST)
        {
                isVerticalScroll = true;
        }
        else
                isVerticalScroll = false;
        if (*threeFingers && *threeFingersEveCount % curSmoothness == 0 && !isDesktopShown && isVerticalScroll)
        {
                isDesktopShown = true;
                curSmoothness = RUNNING_SMOOTHNESS;
                showDesktop();
        }
        preY = value;
}

void handleXChnage(int value, bool *threeFingers, int *threeFingersEveCount, bool *fourFingers, int *fourFingersEveCount)
{
        if (value == 0)
                return;
        if (preX != RELATIVE_ORIGIN && *threeFingers && *threeFingersEveCount % curSmoothness == 0 && !isVerticalScroll)
        {
                curSmoothness = RUNNING_SMOOTHNESS;
                if (value > preX)
                        switchTabs(true);
                else
                        switchTabs(false);
        }
        else if (preX != RELATIVE_ORIGIN && *fourFingers && *fourFingersEveCount % curSmoothness == 0 && !isVerticalScroll && !isDesktopSwitched)
        {
                curSmoothness = RUNNING_SMOOTHNESS;
                isDesktopSwitched = true;
                if (value > preX)
                        switchDesktop(true);
                else
                        switchDesktop(false);
        }
        preX = value;
}

void handleTrackPad()
{
        struct input_event ie;
        int fd, threeFingersEveCount, fourFingersEveCount;
        bool threeFingers = false, fourFingers = false;
        char TOUCHPAD_FILE[] = "/dev/input/";
        curSmoothness = TRIGGER_SMOOTHNESS;
        curSpeed = ACCELARATION;

        //get the touchpad event file
        strcat(TOUCHPAD_FILE, getEventFileName());

        if ((fd = open(TOUCHPAD_FILE, O_RDONLY)) == -1)
        {
                perror("Could not open the touchpad event file");
                exit(EXIT_FAILURE);
        }
        reset();
        while (read(fd, &ie, sizeof(struct input_event)))
        {
                if (threeFingers)
                        threeFingersEveCount++;
                else if (fourFingers)
                        fourFingersEveCount++;
                switch (ie.code)
                {
                case BTN_TOOL_TRIPLETAP:
                        handleThreeFingers(&threeFingers, &threeFingersEveCount);
                        break;
                case BTN_TOOL_QUADTAP:
                        handleFourFingers(&fourFingers, &fourFingersEveCount);
                        break;
                case ABS_Y:
                        handleYChange(ie.value, &threeFingers, &threeFingersEveCount);
                        break;
                case ABS_X:
                        handleXChnage(ie.value, &threeFingers, &threeFingersEveCount, &fourFingers, &fourFingersEveCount);
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
