# Sprelay overivew {#sprelay_overview}

Sperelay application provides means to controll Velleman %K8090 relay card. It can be compiled as a standalone
application or as a library (see @ref index "sprelay index page" for more details about compilation).

If you want to contribute, please follow the @subpage sprelay_style_guide "coding style guide".


## Standalone application

If you want to explore the stand alone application you can start with @ref main() "main()" function but the most
important parts of the application is commented in the chapter describing [dynamic library](#dynamic-library).


## Dynamic library @anchor dynamic-library

Sprelay consists of two dynamic libraries:
- `sprelay_core` library which provides programatic control over the relay card and is summarized in the
  @ref group_biomolecules_sprelay_core_public module. Most of the functionality is provided by the
  @ref biomolecules::sprelay::core::k8090::K8090 class.
- `sprelay` library which contains dialog window which can be used to construct GUI and is summarized in the
@ref group_biomolecules_sprelay_gui_public module.

The public interface of the library is gathered in @ref group_biomolecules_sprelay_public module. Usage example could
be seen in the @ref biomolecules::sprelay::gui::CentralWidget class documentation.

@warning If you want to use sprelay library you have to enable `c++11` standard inside your project.
