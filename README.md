# SpRelay – program for controlling K8090 relay card.

## Requirements

* `Windows` operating system
* `git` – to clone the `SpRelay` repository. You can get git from [here][git].
* `Qt` > 5 – to compile the program. You can get it from [here][qt].
* `MinGW` – it can be installed with Qt (32bit compiler) or downloaded for example with `MSYS2` suite from
  [here][msys2]. From MSYS2 terminal, you can also download and install Qt library for 64 mingw 
  ([description][qtmsys2]).  
* `Doxygen` – for documentation compilation, you can get it from [here][doxygen].
* `dot` – for graphs in documentation, it is part of the GraphViz library, you can get it from [here][graphviz].
* K8090 relay card drivers – [download][k8090download]

Make sure that the Doxygen and GraphViz binaries are in the `PATH` variable.

In our case, the relay card has to be connected to 32bit Windows XP, so you can't use the last versions of the
dependencies, the last version of `git` supporting Windows XP is `2.10.0`, `Qt` can't use ANGLE so, the last compiled
officialy suplied version with OpenGL is `5.4.2`.

## Getting Started

To get you started you can clone the `SpRelay` repository and compile program with `Qt`. The documentation is created by
`Doxygen` and `dot` program from `GraphViz` library.

### Obtaining `SpRelay` application

Clone the `sprelay` repository using git:

```
git clone https://github.com/biomolecules/sprelay.git
```

If you just want to use `SpRelay` without the commit history then you can do:

```
git clone --depth=1 https://github.com/biomolecules/sprelay.git
```

The `depth=1` tells git to only pull down one commit worth of historical data.

### Compilation

#### Command line compilation
Open the Qt Console (it should run `qtenv2.bat` to setup environment, for example
`C:\WINDOWS\system32\cmd.exe /A /Q /K C:\Qt\Qt5.4.2_mingw\5.4\mingw491_32\bin\qtenv2.bat`). Then navigate to
the project directory and run
```
mkdir build
cd build
qmake.exe "CONFIG+=release" ../sprelay.pro -r -spec win32-g++
```

Then run your `make` command, for example (`-j` flag enables compilation paralelization)
```
mingw32-make -j2
mingw32-make doc
```

After make process finishes, go to the bin directory and try to run tests and the program
```
cd ../bin
sprelayunittests.exe
sprelay.exe
```

#### Compilation using Qt Creator
Open the sprelay.pro project file in the top directory, cofigure it for appropriate compiler and run both, tests and
src subprojects.

Then compile the documentation by executing
```
doxygen Doxyfile
```
from `sprelay` directory.

[git]: https://git-scm.com/
[qt]: https://www.qt.io/
[msys2]: http://www.msys2.org/
[qtmsys2]: https://wiki.qt.io/MSYS2
[doxygen]: http://www.stack.nl/~dimitri/doxygen/
[graphviz]: http://graphviz.org/
[k8090download]: http://www.vellemanusa.com/downloads/files/downloads/k8090_vm8090_rev1.zip