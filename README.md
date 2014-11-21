TreeCli - a hierarchical, tree structured command line parser/interface
=========================================================================

Configuration structure
-----------------------------

Treecli provides a convenient way to configure various embedded devices using
common serial and network interfaces. It is inspired by command line interfaces
commonly found in network gear (switches, routers, firewalls..).

Configuration interface contains different kinds of configuration nodes placed
in a hierarchical tree structure. Each tree node can contain:

* other nodes (either statically defined or created dynamically at runtime)
* commands (they can be run specifying their name)
* values (configuration variables whose values can be set or read at runtime)

All nodes, commands and variables can be fully defined at compile time as
constants and therefore they can be included in a .text section freeing precious
RAM space in an embedded environment.

However, sometimes it is desirable to have subnodes created at runtime (such as
variable number of network interfaces, variable number of sensors on a single
bus). This can be done with dynamic node constructors which create subnodes
attached to static configuration at runtime.

Configuration tree items are assigned callback functions for various purposes:

* command execution callback
* value get/set callbacks
* dynamic subnode creator callbacks

Additionally, each configuration item can have a short descriptive text attached
which is displayed when help command is issued ("?", question mark).


Configuration parser
-----------------------------

A parser component operates on a selected configuration tree. It has its own context
defining command execution environment which is preserved between calls to the
parser. It is used to define starting point for the parser in the tree hierarchy
and is also passed to callbacks during parsing to let them know where they are
called from.

The parser can be configured to perform various actions like:

* traversing the configuration tree and changing actual working position
* executing commands using their exec callbacks (if requested)
* reading and settings values using get/set callbacks (if requested)
* displaying help strings (if requested)
* calling match/best match callbacks to support autocompletion (if requested)

This component can be used directly to parse individual command strings when
complete editing capabilities are not required (it can be used for startup
configuration loading from nonvolatile memory).


TreeCli shell component
-----------------------------

Line editor library is used internally to offer classical shell-like functioanlity
allowing the user to enter and edit commands and use command history. Command
parser is then used to parse entered commands and optionally offer command
suggestions and do autocompletion. It also manages command prompt and handles
and displays error messages with nice error markers if something goes wrong.


Command format
-----------------------------

A single command consists of multiple **tokens** delimited by one or more
whitespace characters. Whitespaces are ignored and don't influence command
execution. A token is a set of multiple alphanumerical and other special
characters. Tokens can be:

* node specifier
* command
* value name
* value operation
* value literal
* expression (TODO)
* special "?" command (display help)
* special "/" node (tree root node specifier)
* special ".." node (parent node specifier)

Tokens are then concatenated and/or grouped to perform different actions:

* tree traversal - occurs when "/", ".." or node specifier is used
* command execution - command token is used
* value assignment - when value name, operation and literal or expression is used

These actions can be freely combined on a single line. There is one exception -
tree traversal actions behave differently depending on their position:

* if a command ends with a tree traversal action, current working position
  is updated to the actual one. In other words, position in the thee where the
  commands are executed changes after command execution.
* otherwise (if a command ends with command execution or value assignment),
  working position is not changed and stays the same as before the line was parsed


Command line examples
-----------------------------


Lets start with simple commands:

```
/ > interface ethernet print
```

Something more advanced (although not very usable):

```
/ > interface ethernet print .. wireless print .. serial print
```

Or a real world example:

```
/ > interface ethernet eth0 address="192.168.0.20/24" gateway="192.168.0.1"
```

The same example can be entered on multiple lines:

```
/ > interface ethernet eth0
/interface/ethernet/eth0 > address="192.168.0.20/24"
/interface/ethernet/eth0 > gateway="192.168.0.1"

```

Setting IP addresses on multiple interfaces:

```
/ > interface ethernet
/interface/ethernet/ > eth0 address="192.168.0.20/24" gateway="192.168.0.1"
/interface/ethernet/ > eth1 address="192.168.1.20/24" gateway="192.168.1.1"
/interface/ethernet/ > eth2 address="192.168.2.20/24" gateway="192.168.2.1"
/interface/ethernet/ > /
/ >
```


Usage
-----------------------------

See examples/example1.c file. Terminal needs to be in non-canonical mode for
this example to work correctly. You can use run.sh script to set it using
stty utility. Sample configuration tree can be found in conf_tree1.c file.


A note on assertions
-----------------------------

Library uses custom assert macro to check input parameters. There are two kinds
of them - those depending on user input/environment and it is perfectly valid
if they are invalid, and those which depend on programmer and system sanity.
Example - while it is reasonable to expect an init() function to fail (let's say
because of failed memory allocation), it is not so normal to expect a parsing
function to fail "the normal way" (just by returning appropriate error value)
if NULL pointer is given instead of a line of commands. This is kind of parameters
is checked with assert(). Failed assertion is considered a programmer error -
there is either a bug in the library code or you are doing something wrong.
Assertion fail  is not meant to occur at runtime. Despite this fact, custom
assert macro is used to do "soft" assertions - you can define, what should
happen (eg. log it or take appropriate action). A program should never abort at
runtime because of failed assertion.


Current status
-----------------------------

Library is not much usable at this moment, only some main parts are implemented.

* tree traversal is fully implemented
* command line editing, suggestions and autocompletion is fully implemented
* dynamic node creation is partially implemented, cleanup and documentation needed

TODO:

* code and API cleanup
* more examples and documentation
* value assignment is not yet implemented


Contributing
-----------------------------

TODO


License
-----------------------------

To make inclusion in opensource and commercial firmwares and tools easy, code
is published under the simplified BSD license.

Copyright (c) 2014, Marek Koza (qyx@krtko.org)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

