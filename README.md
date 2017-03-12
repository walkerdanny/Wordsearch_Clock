# Wordsearch_Clock

### Arduino based clock that animates a wordsearch to display the time


Youtube video:

<a href="http://www.youtube.com/watch?feature=player_embedded&v=63ALiYsWzBI
" target="_blank"><img src="http://img.youtube.com/vi/63ALiYsWzBI/0.jpg" 
alt="Video Emo" width="240" height="180" border="10" /></a>

This is the code for my wordsearch clock!

[More info](http://danny.makesthings.work/wordsearch.html)

The main part is "wordsearchClock.ino", used for programming the Arduino.

It relies on the [Time library](https://www.pjrc.com/teensy/td_libs_Time.html), [DS3232RTC Library](https://github.com/JChristensen/DS3232RTC) and [FastLED](http://fastled.io).

The file "gridGenerator.pde" generates a PDF of the letter grid, given an array of letters. The file "Letter_Grid.pdf" is the front face of the clock that I used.
