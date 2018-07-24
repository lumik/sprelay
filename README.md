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

Then go to project folder (`cd sprelay`) and download submodules:
```
git submodule update --init --recursive
```

### Compilation

#### Command line compilation
Open the Qt Console (it should run `qtenv2.bat` to setup environment, for example
`C:\WINDOWS\system32\cmd.exe /A /Q /K C:\Qt\Qt5.4.2_mingw\5.4\mingw491_32\bin\qtenv2.bat`). Then navigate to
the project directory and run
```
mkdir build
cd build
qmake.exe "CONFIG+=release sprelay_build_standalone" sprelay_install_prefix="C:\path\where\you\want\the\application\installed" ../sprelay.pro -r -spec win32-g++
```

You can skip sprelay_install_prefix. Application can't be installed then but it is compiled to the `bin` folder under the project folder.
When CONFIG+=sprelay_build_standalone is omited, the application is built as shared library into `lib` folder and library includes are placed inside
`include` folder.

Then run your `make` command, for example (`-j` flag enables compilation paralelization)
```
mingw32-make -j2
mingw32-make -j1 check TESTARGS="-silent"  # optional if you want to run tests
mingw32-make doc      # optional if you want to make documentation
mingw32-make install  # optional if you want to install the application, see above
```

After make process finishes, go to the bin directory and try to run the program
```
cd C:\path\where\you\want\the\application\installed\bin
sprelay.exe
```

#### Compilation using Qt Creator
Open the sprelay.pro project file in the top directory and cofigure it for appropriate compiler. Setup on the `Projects` tab the right qmake flags in the
`Build Steps` section `Additional arguments`: `CONFIG+=sprelay_build_standalone` (see above the *Command line compilation* section for more details) Then run
either `core_test` (for tests) or `sprelay` (for the application) subprojects.

Then compile the documentation by executing
```
doxygen Doxyfile
```
from command line from the project folder.

[git]: https://git-scm.com/
[qt]: https://www.qt.io/
[msys2]: http://www.msys2.org/
[qtmsys2]: https://wiki.qt.io/MSYS2
[doxygen]: http://www.stack.nl/~dimitri/doxygen/
[graphviz]: http://graphviz.org/
[k8090download]: http://www.vellemanusa.com/downloads/files/downloads/k8090_vm8090_rev1.zip
