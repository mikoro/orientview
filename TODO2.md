# Todo

## Less work
* Add support for slowing/speeding up the video during playback. Maybe remove the divisor settings.
* Add support for reading split times straight from the QuickRoute JPEG file data. The split times are coded as the "lap times".
* Add sound playback support. Extract the sound data from the video file with ffmpeg and output with Qt Multimedia.

## More work
* Add the ability to load multiple routes at the same time for "ghost runners". The program architecture doesn't need much refactoring to support that.
* Add split time importing to SplitsManager. It should be a flexible regex based implementation that could read all the runners, positions and split times of a single route from a text file. Text file format is whatever is published at the results website.
* Add real-time statistics of the runner's performance (+ other runners too).
* Make route rendering prettier. It now uses default drawing commands available in QPainter which are quite basic looking. A more subtle base route appearance + smooth gradients for the pace route appearance.
