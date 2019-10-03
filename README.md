
# TouchPad gestures  

This program tries to emulate windows 10 Touchpad Gestures.  
This program reads the touchpad event file and simulates the actions.  
Each touchpad gestures triggers the keypress which is given below. You can change the default behaviour of the action in the distro keyboard shortcut settings.  

## Dependencies  

* xdotool (can be installed using the command```sudo apt install xdotool``` in ubuntu)  

## How to run  

For now this proram has to be compiled using gcc(```gcc main.c```)and run with admin privilage.
As it matures I will convert it into a kernel module.

## Gestures  

| Gesture                       | KeyPress          | Default gnome Action       |
| ----------------------------- | ----------------- | -------------------------- |
| Three finger Swipe left       | ALT + TAB         | Switch to next Tab         |
| Three finger Swipe left       | ALT + TAB + SHIFT | Switch to previous Tab     |
| Three finger Swipe Up or Down | CTRL + ALT + d    | ShowDesktop                |
| Four finger Swipe left        | CTRL + ALT + UP   | Switch to next Desktop     |
| Four finger Swipe Right       | CTRL + ALT + DOWN | Switch to previous Desktop |
