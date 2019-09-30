#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

char TOUCHPAD_FILE[] = "/dev/input/";
bool altDown, isVerticalScroll, desktopShow, desktopSwitched;
const int TRIGGER_SMOOTHNESS = 15, RUNNING_SMOOTHNESS = 28, VERTICAL_SCROLLL_CHECK_DIST = 3;
int preX, preY, curSmoothness;

char *getEventFile()
{
        FILE *fp;
        char *eventFile = malloc(sizeof(char) * 10);
        fp = popen("awk 'BEGIN{FS=\"\\n\";RS=\"\\n\\n\"}{if($2 ~ /Touchpad/) {  split($6, chars, \" \"); print(chars[3])}}' /proc/bus/input/devices ", "r");
        fscanf(fp, "%s", eventFile);
        printf("%s \n", eventFile);
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
                curSmoothness = RUNNING_SMOOTHNESS;
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
                        if (!threeFingers)
                        {
                                threeFingersEveCount = 0;
                                altDown = false;
                                desktopShow = false;
                                system("xdotool keyup Alt");
                                curSmoothness = TRIGGER_SMOOTHNESS;
                        }
                        break;
                case BTN_TOOL_QUADTAP:
                        fourFingers = !fourFingers;
                        printf("four finger\n");
                        if (!fourFingers)
                        {
                                fourFingersEveCount = 0;
                                desktopSwitched = false;
                                curSmoothness = TRIGGER_SMOOTHNESS;
                        }
                        break;
                case ABS_Y:
                        if (abs(ie.value - preY) >= VERTICAL_SCROLLL_CHECK_DIST)
                        {
                                isVerticalScroll = true;
                        }
                        else
                                isVerticalScroll = false;
                        if (threeFingers && threeFingersEveCount % curSmoothness == 0 && !desktopShow && isVerticalScroll)
                        {
                                desktopShow = true;
                                showDesktop();
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
                        else if (fourFingers && fourFingersEveCount % curSmoothness == 0 && !isVerticalScroll && !desktopSwitched)
                        {
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
