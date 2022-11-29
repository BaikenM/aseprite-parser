# aseprite-parser
A library for parsing .aseprite files. Aseprite allows exporting saved files to JSON or PNG and other graphical formats. This library is aimed to provide the user with an interface to work directly with .aseprite files without the need of storing additional exported files.

NOTE: the tests have covered only a few chunk types (0x0004, 0x2004, 0x2005, 0x2007, 0x2018, 0x2019).
A detailed .aseprite file format specification can be found at https://github.com/aseprite/aseprite/blob/main/docs/ase-file-specs.md#frames
