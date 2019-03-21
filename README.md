![icon](http://software.rochus-keller.info/LolaCreator_100.png)
## Welcome to LolaCreator 

LolaCreator is a [QtCreator](https://download.qt.io/official_releases/qtcreator/) plugin. It turns QtCreator into a Lola-2 IDE. 

The plugin is still work in progress, but it already has enough functionality to analyze existing code bases or to develop new code. The current version supports Lola-2 syntax checking/coloring and semantic code navigation/highlighting. Projects can be configured using a file format similar to qmake.

![screenshot](http://software.rochus-keller.info/lolacreator_screenshot_3.png "LolaCreator Screenshot")


### Implemented Features

- Syntax highlighting 
- Context sensitive code completer, see [screenshot](http://software.rochus-keller.info/vlcreator_completer_screenshot.png)
- Inline code warnings and errors 
- Parentheses and begin/end block matching/navigation
- Hover tooltips
- Follow symbol under cursor (multi-file support)
- Show where a symbol is used 
- Drop-down menu to jump to modules in current file  
- Module locator (global) and symbol locator (current document); see [screenshot 1](http://software.rochus-keller.info/vlcreator_module_locator_screenshot.png) and [screenshot 2](http://software.rochus-keller.info/vlcreator_symbol_locator_screenshot.png)
- Syntax based code folding and some other features supported by QtCreator, see http://doc.qt.io/qtcreator/
- Project file to configure source files and libraries

### Project file format

LolaCreator can either open single Lola-2 files or project configuration files with the suffix ".llpro".

The llpro file can be empty; in that case all Lola-2 files (default suffix ".Lola") located in the llpro directory including subdirectories are assumed to be the source files of the project. 

llpro files follow the same syntax as QtCreator pro files (see http://doc.qt.io/qt-5/qmake-project-files.html), but uses other variable names. The following variables are supported:

`SRCFILES += /path/filename.Lola`

`SRCFILES += "/path with whitespace/filename.Lola"`

```
SRCFILES += /firstpath/filename.Lola /secondpath/filename.Lola \
		/thirdpath_on_new_line/filename.Lola
```
		
`SRCFILES += /directory_path1/ filename1.Lola filename2.Lola /directory_path2/ filename3.Lola filename4.Lola`

The variable `SRCFILES` lets you add source files explicitly to your project. There are different syntax versions. You can add filenames with absolute or relative paths; if you add a directory path only then the following filenames are assumed to belong to the directory with the given path.
Note that path/filenames including whitespace have to be quoted by " ". Also note that you can add more than one element to a line separated by whitespace and that a \ lets you continue on the next line; this applies to all variables.

`SRCEXT = .Lola .Mod .lola`

The `SRCEXT` variable can be used to override the expected suffix of Lola-2 files. Default is ".Lola".


`SRCDIRS += dirpath1 dirpath2 dirpath3*`

`SRCDIRS += dirpath4 -filename1 -filename2 dirpath5 -filename3`

With the `SRCDIRS` variable it is possible to add all Lola-2 files in a given directory. All files with a suffix specified in SRCEXT are considered. There are different syntax versions. You can add directories with absolute or relative path. If you postfix the path with an "*" then also all subdirectories are implicitly added. If you want to leave out certain files add the filename to the line prefixed with "-".

`LIBFILES += `

`LIBDIRS += `

`LIBEXT = `

These variables are used the same way as their SRC counterparts. Use them to add Lola-2 files which are used but not modified by the present project.


`TOPMOD = RISC5Top`

Use TOPMOD to explicitly set the top-level module; this is useful in case there is more than one top-level module in the code base. LolaCreator does not use this information.


### Download Binaries

Pre-compiled versions of the LolaCreator plugin can be downloaded for Windows, Linux and Macintosh as part of the QtcVerilog application.

See here to learn more: https://github.com/rochus-keller/QtcVerilog/blob/master/README.md

### Compatibility

Unfortunately the plugin API of QtCreator is not stable. LolaCreator is compatible with QtCreator 3.6.1. But in QtCreator 4.x they not only have changed some function signatures, but also the architecture to a considerable extent. According to my current state of investigation, managing compatibility with a few ifdefs is no longer feasible; instead it seems necessary to maintain different codebases in parallel to be compatible with both QtCreator 3.x and 4.x and cope with the different architectures at the same time.

Since I consider the effort to reverse engineer each QtCreator version anew and to develop different variants of the code in parallel to be too high, I do it the other way round: instead of adapting the plugin to each major and minor QtCreator version, I offer a simple binary deployment for Linux, Windows and Mac of a lean QtCreator version including LolaCreator and some other plugins. See here to learn more: https://github.com/rochus-keller/QtcVerilog.

### Build Steps
Follow these steps if you want to build LolaCreator yourself:

1. Make sure that QtCreator and the Qt base development package are installed on your system. LolaCreator was developed and tested with QtCreator 3.6 and Qt 5.4 on Linux. It is not compatible (yet) with QtCreator 4.x. Instead of using QtCreator, you can also use QtcVerilog (see https://github.com/rochus-keller/QtcVerilog).
1. Create a build directory; let's call it BUILD_DIR
1. Download the LolaCreator source code from https://github.com/rochus-keller/LolaCreator/archive/master.zip and unpack it to the BUILD_DIR; rename the subdirectory to "LolaCreator".
1. Download the Lola-2 parser source code from https://github.com/rochus-keller/Lola/archive/master.zip and unpack it to the BUILD_DIR; rename "Lola-Master" to "Lola". 
1. Download the appropriate version of the QtCreator source code from https://download.qt.io/official_releases/qtcreator/ and unpack it to the BUILD_DIR. Instead of the QtCreator source code you can also use the QtcVerilog source code (see https://github.com/rochus-keller/QtcVerilog).
1. Either set the QTC_SOURCE (path to the QtCreator/QtcVerilog source code directory) and QTC_BUILD (path to the QtCreator/QtcVerilog installation directory) environment variables or directly modify the QTCREATOR_SOURCES and IDE_BUILD_TREE variables in LolaCreator.pro. 
1. Goto the BUILD_DIR/LolaCreator subdirectory and execute `QTDIR/bin/qmake LolaCreator.pro` (see the Qt documentation concerning QTDIR).
1. Run make; after a couple of minutes the plugin is compiled and automatically deployed to the local QtCreator plugin directory (on Linux to ~/.local/share/data/QtProject/qtcreator/plugins/x.y.z/libLolaCreator.so) or directly to QtcVerilog. 
1. Start QtCreator/QtcLola-2 and check in the "Help/About Plugins..." dialog (section "Other Languages") that the plugin is activated.

Instead of using qmake and make you can open LolaCreator.pro using QtCreator and build it there.

Note that on Windows you also have to compile QtCreator/QtcVerilog itself because you also need the lib files for the plugin dll's (i.e. Core.lib, TextEditor.lib and ProjectExplorer.lib). Compiling QtCreator is not an easy task; compiling QtcVerilog is much easier.

### To do's

- Documentation
- Semantic indenter (the current one simply adjusts to the previous line)
- Build configurations, automatic translation of Lola-2 to Verilog and/or Yosys RTLIR
- Implement wizzard to add or import files, generate stub modules and automatically update the llpro file
- Improve hover text
- Implement options dialog (format settings, paths, etc.)
- Integrate wave viewer

### Support
If you need support or would like to post issues or feature requests please use the Github issue list at https://github.com/rochus-keller/LolaCreator/issues or send an email to the author.



