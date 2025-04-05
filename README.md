# OrientView

OrientView is an orienteering video analysis program that displays the video and the map side-by-side in real-time. Many image and video file formats are supported. Routes are created and calibrated using [QuickRoute](http://www.matstroeng.se/quickroute/en/). The resulting video can be exported as an MP4 file.

* Author: [Mikko Ronkainen](http://mikkoronkainen.com)
* Website: [github.com/mikoro/orientview](https://github.com/mikoro/orientview)

![Screenshot](http://mikoro.github.io/images/orientview/readme-screenshot.jpg "Screenshot")

Example videos:

[Kattilajärvi, Finland (stabilized)](https://youtu.be/4jh9KmYjdq8?si=M1UggbHVKInae4A9)  
[Český Ráj, Czech Republic (split centered)](https://youtu.be/KzfDpsgoeck?si=FSmcId5gGnHgNfyn)  
[Prague Zoo, Czech Republic (runner-centered, split-oriented)](https://youtu.be/wrvxJsXHAtc?si=Hv29LG7lthBRwy39)  
[Český Ráj, Czech Republic (runner-centered)](https://youtu.be/iqBbxpQatak?si=TpNw071LAsF-BR9D)

## Download

Download the latest version from the [Releases](https://github.com/mikoro/orientview/releases/latest) page.

To test the program, you can also [download test data](https://s3.amazonaws.com/orientview-testdata/orientview-testdata.zip).

## Features

* Opens the most common image file formats (e.g. JPEG, PNG, and TIFF).
* Reads route data from [QuickRoute](http://www.matstroeng.se/quickroute/en/), making it possible to manually adjust the route for accuracy (when GPS is not perfect).
* Split times are input manually using a simple format (absolute or relative).
* Supports video seeking and pausing. Different timing offsets are adjustable to synchronize the video with the route/runner.
* Supports basic video stabilization using the [OpenCV](http://opencv.org/) library. Stabilization can be done in real-time or by using preprocessed data.
* The output video is highly customizable. You can configure different colors, sizes, speeds, map orientations, and runner-following options.
* All graphics are drawn using OpenGL, which utilizes shaders for custom image resampling (e.g., high-quality bicubic).
* Both the video window and exported video are fully resizable, so the original video resolution does not impose restrictions.
* The resulting video can be exported as an MP4 file with H.264 encoding.
* The program architecture is multithreaded and should allow maximum CPU core usage, for example, when exporting video.

## Instructions

### How to use

1. You need the run video, the map, the GPS track, and the split times.
2. If the video is in multiple parts, stitch it together using, for example, [Avidemux](http://fixounet.free.fr/avidemux/). Make sure to export it as MP4.
3. Scan the map at high resolution (600 dpi TIFF is preferable). There is no need to compress it; modern GPUs can handle it.
4. Fix the map (orientation, cropping, levels, etc.) and preferably export to TIFF format.
5. Make a smaller copy of the map for use with QuickRoute. **Make sure to only scale it down (change the resolution); do not crop the image—this should have been done in the previous step.** The QuickRoute image data will not be used, so its quality does not matter (only data embedded by QuickRoute in the JPEG file is used). QuickRoute needs a smaller image because it is very slow otherwise.
6. Using [QuickRoute](http://www.matstroeng.se/quickroute/en/), cut and align the GPS track to the map. You can use as many adjustment points as you want. Then export the map as a JPEG image (File -> Export -> Image...).
7. Format the split times as a single string. The format is "hours:minutes:seconds," with hours and minutes being optional, and the separator between splits being "|" or ";". For example: `1:23|1:23:45`. The time separator can also be ".". For example: `1.23;1.23.45`. Split times can be absolute or relative.
8. Open OrientView and browse for the map image file, the QuickRoute JPEG file, and the video file. Enter the split times and press Play.
9. Adjust the control time offset to move the controls to their correct positions. Then seek the video to the first control and pause. Now adjust the runner time offset to move the runner to the correct position. Press **F1** and note the adjusted values, which you can then re-enter in the settings window.

#### Tips

- When using QuickRoute, remember that the GPS usually starts at the actual start and not the map start, so it is incorrect to align the route start directly with the triangle.
- When using QuickRoute, it helps to watch the video to align the route correctly. When you hover over the route, the time at that point is displayed at the bottom of the screen. You can use this alongside the video to improve alignment.
- When aligning the offsets in OrientView, make use of the shortcuts in the Play window as described in [How to use](#how-to-use).

### Misc

* Most of the UI controls have tooltips explaining their functions.
* Not all settings are exposed in the UI. You can edit additional settings by first saving the current settings to a file, opening it with a text editor (the file is in INI format), and then loading the file back.
* The difference between real-time and preprocessed stabilization is that the latter can analyze future frames. This makes centering faster with sudden large frame movements and also more responsive to small movements.
* The rescale shaders are in the *data/shaders* folder. The bicubic shader can be further customized by editing the *rescale_bicubic.frag* file (currently there are five different interpolation functions and some other settings).

### Known issues

* Transparent (not 100% opaque) colors on the route and tail are rendered incorrectly (as a rectangle) in the Play preview, but they encode properly in the final video.
* Audio is removed. One solution is to export the audio from the original bodycam footage and then add it back to the OrientView export. I recommend using [Avidemux](http://fixounet.free.fr/avidemux/).

### Controls

| Key           | Action                                                                                     |
|---------------|--------------------------------------------------------------------------------------------|
| **F1**        | Toggle info panel on/off                                                                   |
| **F2**        | Select map/video/none for scrolling                                                        |
| **F3**        | Select render mode (map/video/all)                                                         |
| **F4**        | Select route render mode (none/discreet/highlight/pace)                                    |
| **F5**        | Select tail render mode (none/discreet/highlight)                                          |
| **F6**        | Select route view mode (fixed split / runner centered / runner centered fixed orientation) |
| **F7**        | Toggle runner on/off                                                                       |
| **F8**        | Toggle controls on/off                                                                     |
| **F9**        | Toggle video stabilization on/off                                                          |
| **Space**     | Pause or resume video <br> Ctrl + Space advances one frame                                 |
| **Ctrl**      | Slow/small modifier                                                                        |
| **Shift**     | Fast/large modifier                                                                        |
| **Alt**       | Very fast/large modifier                                                                   |
| **Ctrl + 1**  | Reset map modifications                                                                    |
| **Ctrl + 2**  | Reset video modifications                                                                  |
| **Ctrl + 3**  | Reset route modifications                                                                  |
| **Ctrl + 4**  | Reset timing offset modifications                                                          |
| **Left**      | Seek video backwards <br> Scroll map/video left                                            |
| **Right**     | Seek video forwards <br> Scroll map/video right                                            |
| **Up**        | Scroll map/video up                                                                        |
| **Down**      | Scroll map/video down                                                                      |
| **Q**         | Zoom map in                                                                                |
| **A**         | Zoom map out                                                                               |
| **W**         | Rotate map counterclockwise                                                                |
| **S**         | Rotate map clockwise                                                                       |
| **E**         | Increase map width                                                                         |
| **D**         | Decrease map width                                                                         |
| **R**         | Zoom video in                                                                              |
| **F**         | Zoom video out                                                                             |
| **T**         | Rotate video counterclockwise                                                              |
| **G**         | Rotate video clockwise                                                                     |
| **Y**         | Increase route scale                                                                       |
| **H**         | Decrease route scale                                                                       |
| **Page Up**   | Increase runner offset                                                                     |
| **Page Down** | Decrease runner offset                                                                     |
| **Home**      | Increase control offset                                                                    |
| **End**       | Decrease control offset                                                                    |
| **Insert**    | Increase tail length                                                                       |
| **Delete**    | Decrease tail length                                                                       |

## How to build

For build instructions, see [BUILD.md](https://github.com/mikoro/orientview/blob/main/BUILD.md).

## License

    OrientView
    Copyright © 2014-2025 Mikko Ronkainen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
