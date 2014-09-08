# Todo

* Add support for reading split times straight from the QuickRoute JPEG file data.
* Make route rendering prettier. It now uses default drawing commands available in QPainter which are quite basic looking. A more subtle base route appearance + smooth gradients for the pace route appearance.
* Add the ability to load multiple routes at the same time for "ghost runners". The program architecture doesn't need much refactoring to support that.
* Add split time importing to SplitsManager. It should be a flexible regex based implementation that could read all the runners, positions and split times of a single route from a text file. Text file format is whatever is published at the results website.
* Add real-time statistics of the runner's performance (+ other runners too).
