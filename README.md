<!--- eat my shorts doxygen!! -->
SVN-External-Disposer
=======================

A QT based tool to manage svn:externals.

The svn:externals are a property type used by the subversion version control system
to include different subdirectories from different location into a source code tree.

If a software is build by different libraries, modules or software parts within a 
source-code tree, the configuration of this software can be done with svn:externals.
In this situations, specially of the configuration gets more and more complex,
this tool can help to get an overview which folder is pulled in by a svn:external entry, 
from where it is coming from and which revision is used.

The software can be seen as a file-browser, which is extended by some information about the
svn:externals. It consists a revision-log browser which then helps to select the correct revision.

Further information
   - SVN Documentation about svn:externals: http://svnbook.red-bean.com/en/1.7/svn.advanced.externals.html

and some discussions about svn:externals and why it is maybe not good to use this feature:
   - http://stackoverflow.com/a/345404
   - http://stackoverflow.com/a/304036
   - http://www.christoph-jahn.com/?p=675

The latest stable version for windows can be downloaded from [here][win32_v1.0.0] and the latest stable sources can be downloaded as zip from [here][source_v1.0.0].



Get the source-code
=======================

```
git clone https://github.com/feddischson/svn-externals-disposer.git
```



Dependencies
==============

Build
--------------
This tool depends on QT-5 and a compiler which supports C++11.

Runtime
--------------
The SVN command line tool available in your system environment.
Try to run (on windows via cmd.exe)
```
svn --version
```
to figure out if you have the SVN command line tool with the correct version.
This tool is tested with SVN version 1.8.x.


Build and install this tool
=======================

``` 
qmake svn_externals_disposer.pro
make
make install
```

Create the Documentation
=======================

```
doxygen doxy.cfg
```
The result is placed in `doxydoc`.


Run some tests
=======================

```
qmake test.pro
./test_svn_externals_disposer
```

Version log
=================


v1.2.0
-------
This version can be found [here][v1.2.0]:

  - Adds a repository browser
  - A windows build can be downloaded from [here][win32_v1.2.0].
  - The sources code can be downloaded as zip from [here][source_v1.2.0].


v1.1.0
-------
This version can be found [here][v1.1.0]:

  - Adds performance improvements
  - Adds a filter functionality to show only external-related files/folders.
  - A windows build can be downloaded from [here][win32_v1.1.0].
  - The sources code can be downloaded as zip from [here][source_v1.1.0].



v1.0.1
-------
This version can be found [here][v1.0.1]:

  - Adds performance improvments
  - Supports svn:external files
  - Cleans paths ( e.g. some/./path is converted to some/path )
  - Prints a message when an external is defined without an existing folder (e.g. svn update was not done)
  - A windows build can be downloaded from [here][win32_v1.0.1].
  - The sources code can be downloaded as zip from [here][source_v1.0.1].



v1.0.0
--------
This version can be found [here][v1.0.0]:

  - Shows svn:externals information in a file-browser
  - Allows editing svn:externals properties via file-browser or dialog
  - Includes a revision history browser
  - Highlights changes
  - Allows to revert changes
  - Allows to save changes
  - A windows build can be downloaded from [here][win32_v1.0.0].
  - The sources code can be downloaded as zip from [here][source_v1.0.0].


License
=======================

```
Copyright (C) 2016  Christian Haettich [feddischson]

This program is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation; 
either version 3 of the License, or (at your option) 
any later version.

This program is distributed in the hope that it will 
be useful, but WITHOUT ANY WARRANTY; without even the 
implied warranty of MERCHANTABILITY or FITNESS FOR A 
PARTICULAR PURPOSE. See the GNU General Public License 
for more details.

You should have received a copy of the GNU General 
Public License along with this program; if not, see 
<http://www.gnu.org/licenses/>. 
```


[v1.2.0]: https://github.com/feddischson/svn-externals-disposer/releases/tag/v1.2.0
[win32_v1.2.0]: https://github.com/feddischson/svn-externals-disposer/releases/download/v1.2.0/svn_externals_disposer_v1.2.0_win32.zip
[source_v1.2.0]: https://github.com/feddischson/svn-externals-disposer/archive/v1.2.0.zip


[v1.1.0]: https://github.com/feddischson/svn-externals-disposer/releases/tag/v1.1.0
[win32_v1.1.0]: https://github.com/feddischson/svn-externals-disposer/releases/download/v1.1.0/svn_externals_disposer_v1.1.0_win32.zip
[source_v1.1.0]: https://github.com/feddischson/svn-externals-disposer/archive/v1.1.0.zip


[v1.0.1]: https://github.com/feddischson/svn-externals-disposer/releases/tag/v1.0.1
[win32_v1.0.1]: https://github.com/feddischson/svn-externals-disposer/releases/download/v1.0.1/svn_externals_disposer_v1.0.1_win32.zip
[source_v1.0.1]: https://github.com/feddischson/svn-externals-disposer/archive/v1.0.1.zip


[v1.0.0]: https://github.com/feddischson/svn-externals-disposer/releases/tag/v1.0.0
[win32_v1.0.0]: https://github.com/feddischson/svn-externals-disposer/releases/download/v1.0.0/svn_externals_disposer_v1.0.0_win32.zip
[source_v1.0.0]: https://github.com/feddischson/svn-externals-disposer/archive/v1.0.0.zip



