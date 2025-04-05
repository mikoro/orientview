# OrientView

OrientView is an orienteering video analyzing program which displays the video and the map side-by-side in real-time. Many image and video file formats are supported. Routes are created and calibrated using [QuickRoute](http://www.matstroeng.se/quickroute/en/). Resulting video can be exported to an MP4 video file.

* Author: [Mikko Ronkainen](http://mikkoronkainen.com)
* Website: [github.com/mikoro/orientview](https://github.com/mikoro/orientview)

![Screenshot](http://mikoro.github.io/images/orientview/readme-screenshot.jpg "Screenshot")

Example videos:

[Kattilajärvi, Finland (stabilised)](https://youtu.be/4jh9KmYjdq8?si=M1UggbHVKInae4A9)  
[Český Ráj, Czech Republic (split centered)](https://youtu.be/KzfDpsgoeck?si=FSmcId5gGnHgNfyn)  
[Prague Zoo, Czech Republic (runner centered, split oriented)](https://youtu.be/wrvxJsXHAtc?si=Hv29LG7lthBRwy39)    
[Český Ráj, Czech Republic (runner centered)](https://youtu.be/iqBbxpQatak?si=TpNw071LAsF-BR9D)

## Download

Download the latest version from the [Releases](https://github.com/mikoro/orientview/releases/latest) page.

## Features

* Opens the most common image file formats (e.g. JPEG, PNG and TIFF).
* Reads route data from [QuickRoute](http://www.matstroeng.se/quickroute/en/). Making it possible to manually adjust the route to be accurate (when gps is not perfect)
* Split times are input manually using simple formatting (absolute or relative).
* Supports video seeking and pausing. Different timing offsets are also adjustable to make the video and route/runner match.
* Supports basic video stabilization using the [OpenCV](http://opencv.org/) library. Stabilization can be done real-time or by using preprocessed data.
* The output video is very customizable. Different colors, sizes, speeds, map orientation and runner following.
* Draws all the graphics using OpenGL. Also utilizes shaders to do custom image resampling (e.g. high quality bicubic).
* Video window and exported video are completely resizable -- original video resolution does not pose any restrictions.
* Resulting video can be exported to an MP4 file with H.264 encoding.
* Program architecture is multithreaded and should allow maximal CPU core usage when for example exporting video.

## Instructions

### How to use

1. You need the video of the run, the map, the gps track, and the split times.
2. If the video is in multiple parts, you need to stitch it together using for example [Avidemux](http://fixounet.free.fr/avidemux/). Make sure to export it to MP4.
3. Scan the map with high resolution (600 dpi TIFF format is preferable). No need to compress it, modern GPUs can handle that.
4. Fix the map (orientation, cropping, levels etc.) and preferably export in TIFF format 
5. Make a smaller copy of the map for use with QuickRoute. **Make sure to only scale it down (change the resolution) do not crop the image, that should have been done in the previous step**. The QuickRoute image data will not be used so its quality doesn't matter. (only data embedded by QuickRoute in the JPEG file is used) QuickRoute needs smaller image, because it is very slow otherwise.
6. Using [QuickRoute](http://www.matstroeng.se/quickroute/en/) cut and align the gps track to the map. You can use as many adjustment points as you want. Then export the map as a JPEG image (File -> Export -> Image...)
7. Format the split times to a single string. Format is "hours:minutes:seconds" with hours and minutes being optional and the separator between splits being "|" or ";". For example: `1:23|1:23:45`. Time separator can also be ".". For example: `1.23;1.23.45`. Split times can be absolute or relative.
8. Open OrientView and browse for the map image file, the QuickRoute JPEG file, and the video file. Input the split times and press Play.
9. Adjust the control time offset to move the controls to correct positions. Then seek the video to the first control and pause. Now adjust the runner time offset to move the runner to the correct position. Press F1 and take note of the adjusted values which you can then input back at the settings window.

#### Tips

- When using QuickRoute, don't forget, that you usually start the gps at the actual start and not tme map start, so it is wrong to align the route start directly with the triangle.
- When using QuickRoute, it is helpful to look at the video to align the route correctly, when you hover over the route, at the bottom of the screen you can see the time of that point. You can use that and the video to better align the route.
- When aligning the offsets in OrientView, really make a use of the shortcuts in the Play window as described in [How to use](#how-to-use).

### Misc

* Most of the UI controls have tooltips explaining what they are for.
* Not all settings are exposed to the UI. You can edit the extra settings by first saving the current settings to a file, opening it with a text editor (the file is in ini format), and then loading the file back.
* The difference between real-time and preprocessed stabilization is that the latter can look at the future when doing the stabilization analysis. This makes the centering faster with sudden large frame movements and also makes the stabilization a little bit more responsive to small movements.
* The rescale shaders are in the *data/shaders* folder. The bicubic shader can be further customized by editing the *rescale_bicubic.frag* file (currently there are five different interpolation functions and some other settings).

### Known issues

* Transparent (not 100% opaque) colors on route and tail are rendered incorrectly (as a rectangle) in the Play preview, however they encode properly in the final video.
* Audio is erased. One solution is to export the audio from the original bodycam footage and then add it back to the Orientview export. I recommend using [Avidemux](http://fixounet.free.fr/avidemux/)

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
