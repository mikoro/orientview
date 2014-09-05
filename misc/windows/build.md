# Building on Windows

1. Install [Visual Studio 2013 Professional](http://www.visualstudio.com/).
2. Install [Qt 5.3](http://qt-project.org/downloads) (64-bit, VS 2013, OpenGL).
3. Install [Visual Studio Add-in for Qt5](http://qt-project.org/downloads).
4. Configure Qt Visual Studio settings to point to the correct Qt installation.
5. Add Qt bin folder to your path.
6. Clone [https://github.com/mikoro/orientview.git](https://github.com/mikoro/orientview.git).
7. Clone [https://github.com/mikoro/orientview-binaries.git](https://github.com/mikoro/orientview-binaries.git).
8. Copy the files from *orientview-binaries/windows* to the orientview root folder.
9. Build using either Visual Studio or QtCreator.
10. If launching the debug build from Visual Studio fails with warning about missing dlls, edit the project working directory to point to the debug build output folder.
