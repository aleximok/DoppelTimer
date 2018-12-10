# DoppelTimer
### Time tracking app

Actually, this timer application was intended as QT-development training project. As long as it still a kind of programming playground, nevertheless, DoppelTimer is a useful featured application with a decent personal time-tracking functionality.

Main floating panel looks like:

![alt text](https://github.com/aleximok/DoppelTimer/raw/master/art/ToolWinShot.png "")

Panel with options revealed:

![alt text](https://github.com/aleximok/DoppelTimer/raw/master/art/ToolWinOpenShot.png "")

Temporarily, win32setup available here:

<http://gradency.com/files/DTWin32Setup-1.3.exe>

## Quick facts about DoppelTimer

1. The main idea of DoppelTimer is keeping regular breaks during a computer-related job. The first timer is for doing work, and the second one is for short relax times. This helps prevent eyestrain, lower back pain, and other unpleasant desk job drawbacks. That's why by default work/relax timers are set to 40/10 minutes. Though, you may change it to any suitable values.

2. DoppelTimer has two built-in sound schemes, "Kitchen timer" and "Grandfather clock". When the alarm is about to start ringing, you'll be warned beforehand with slowly raised "ticking" sound. Personally, I believe that each unexpected abrupt signals shorten your life by a couple of minutes. In addition, it is still possible to set sound off.

3. Added OpenGL screensaver imitating a kind of skydiving. It helps immediately switch off from working and gives a rest to your eyes. Pops on relax timer activation, deactivates on 'Esc' key or mouse click.

4. To pick the current activity you can simply double click on related row header. Only today activities can be picked! If your activity is the same as the day before, you can utilize the "Clone" button.

5. Double click on tray icon to show/hide application window.

6. The application was developed/tested primarily on the Windows platform. Though, thanks to QT cross-platform compatibility it should work as well on Linux and Mac OS.

7. DoppelTimer demonstrates "sticky window" behavior implemented exclusively with QT provided tools and facilities. QT documentation states that calling window' move() method inside moveEvent() can lead to infinite recursion. However, I'd invented workaround for the problem, so timer window is able to "stick" to upper/lower desktop bounds during its dragging without eternal freezes. For implementation details refer to "CWindowSticker.h/cpp". The class is nothing but event filter and can be used with any window simply like this:

```
    CMyWindow win;

    // Sets itself as an object child, so it destroyed automatically with window.
    new CWindowSticker (&win, kStickVerticalSides | kDragFramelessWindow);
```

8. Optional table columns 'Estimate' and 'Difference' are measured in time units and can be shown/hidden with "Estimation" checkbox. Each time unit is a completely finalized period of working time.

9. Sound files partially recorded by me, partially taken from freesound.org:

   - S: Vienna Fluteclock 1810.aif by Ohrwurm -- https://freesound.org/s/64923/ -- License: Creative Commons 0
   - S: Grandfather Clock.wav by FreqMan -- https://freesound.org/s/88525/ -- License: Attribution
