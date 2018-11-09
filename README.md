# Sprelay – program for controlling K8090 relay card.

[![Build Status](https://travis-ci.org/biomolecules/sprelay.svg?branch=master)](https://travis-ci.org/biomolecules/sprelay)
[![Build status](https://ci.appveyor.com/api/projects/status/ppywbasppr32qnad/branch/master?svg=true)](https://ci.appveyor.com/project/lumik/sprelay/branch/master)
[![codecov](https://codecov.io/gh/biomolecules/sprelay/branch/master/graph/badge.svg)](https://codecov.io/gh/biomolecules/sprelay)


## Requirements

* `Windows` operating system
* `git` – to clone the `Sprelay` repository. You can get git from [here][git].
* `Qt` > 5 – to compile the program. You can get it from [here][qt].
* `MinGW` – it can be installed with Qt (32bit compiler) or downloaded for example with `MSYS2` suite from
  [here][msys2]. From MSYS2 terminal, you can also download and install Qt library for 64 mingw 
  ([description][qtmsys2]).
* `CMake` build system, you can get it from [here][cmake].
* `Doxygen` – for documentation compilation, you can get it from [here][doxygen].
* `dot` – for graphs in documentation, it is part of the GraphViz library, you can get it from [here][graphviz].
* `python` – for running tests, you can get it from [here][python].
* K8090 relay card drivers – [download][k8090download]

Make sure that the CMake, Doxygen, GraphViz and Python binaries are in the `PATH` variable.

In our case, the relay card has to be connected to 32bit Windows XP, so you can't use the last versions of the
dependencies, the last version of `git` supporting Windows XP is `2.10.0`, `Qt` can't use ANGLE so, the last compiled
officialy suplied version with OpenGL is `5.4.2` and last supported `python` version is `3.4`.


## Getting Started

To get you started you can clone the `Sprelay` repository and compile program with `Qt`. The documentation is created by
`Doxygen` and `dot` program from `GraphViz` library.


### Obtaining `Sprelay` application

Clone the `sprelay` repository using git:

```
git clone https://github.com/biomolecules/sprelay.git
```

If you just want to use `Sprelay` without the commit history then you can do:

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
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ^
-DCMAKE_INSTALL_PREFIX=.. -DBUILD_STANDALONE=ON -DMAKE_TESTS=ON
```

You can skip `CMAKE_INSTALL_PREFIX`. Application is then installed to default destination ("C:\Program Files" on
Windows, "/usr/locaL" on Linux).
the application is built as shared library and library includes are placed inside `include` folder if
`BUILD_STANDALONE` is omited or set to `OFF`,  The tests are not built if `MAKE_TESTS` is omited or set to `OFF`.

If you want to build only shared library without gui, you can use SKIP_GUI define:
```
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug ^
-DCMAKE_INSTALL_PREFIX=.. -DBUILD_STANDALONE=OFF -DMAKE_TESTS=ON -DSKIP_GUI=OFF
```
`Sprelay` application depends on `enum_flags` library. The library is searched in system path first and if not found,
the internal `enum_flags` copy is used. If you want to specify different location of `enum_flags` you can set
`enum_flags_ROOT_DIR` variable. If you want to force usage of `enum_flags` distributed with the application you can
specify `MAKE_ENUM_FLAGS=ON`. For example if you have installed `enum_flags` at `c:\libraries\enum_flags`, you can
invoke
```
cmake .. -G "MinGW Makefiles" -DMAKE_ENUM_FLAGS=OFF ^
-Denum_flags_ROOT_DIR:PATH=c:\libraries\enum_flags
```

Then run your `make` command, for example (`-j` flag enables compilation paralelization)
```
mingw32-make -j2
mingw32-make test      # optional if you built tests and want to run them
ctest -V               # to run tests with detail output
mingw32-make test ARGS="-V" # the same as above
mingw32-make doc       # optional if you want to make documentation
mingw32-make install   # optional if you want to install the application, see
# above
```

After make process finishes, go to the bin directory and try to run the program
```
cd C:\path\where\you\want\the\application\installed\bin
sprelay.exe
```


#### Compilation using Qt Creator

Open the CMakeFiles project file in the top directory and cofigure it for appropriate compiler. Add arguments as
discussed above.

Then compile the documentation by executing
```
doxygen Doxyfile
```
from command line from the build project folder.


[git]: https://git-scm.com/
[qt]: https://www.qt.io/
[msys2]: http://www.msys2.org/
[qtmsys2]: https://wiki.qt.io/MSYS2
[cmake]: https://cmake.org/download/
[doxygen]: http://www.stack.nl/~dimitri/doxygen/
[graphviz]: http://graphviz.org/
[python]: https://www.python.org/downloads/windows/
[k8090download]: http://www.vellemanusa.com/downloads/files/downloads/k8090_vm8090_rev1.zip
