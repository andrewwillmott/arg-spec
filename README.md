**ArgSpec** provides a mechanism for parsing command-line arguments via a simple
printf/scanf-style specification, which defines the arguments, the variables
they should bind to, and their description. The same specification can also be
used to generate documentation, either as a help string, or even in html format.

It is available either as a classic single-header library, in `ArgSpec.h`, or
more classic library files, `ArgSpec.hpp` and `ArgSpec.cpp`. The single header
is just the two library files concatenated together. Because typical use is to
only be included in main.cpp, the default is to include the implementation, and
you should #define `ARG_SPEC_NO_IMPL` if you don't want this.

The `ArgSpec` class handles bools, ints, floats, strings, enums, vectors of any
of these, and repeated arguments. It also handles optional arguments, error
detection, and help.

Here's a simple example:

    #include "ArgSpec.h"

    ...

    enum { kSlotPresent, kFlagPresent };

    const char* name = "unknown";
    int slot;

    argSpec.ConstructSpec
    (
         "Test command for ArgSpec",
         "<name:string> [<slot:int>^]", &name, &slot, kSlotPresent,
            "Purpose of name and slot",
         "-flag^", kFlagPresent,
             "What the flag does",
         nullptr
    );

    tArgError err = argSpec.Parse(argc, argv);

    if (err != kArgNoError)
    {
        printf("%s\n", argSpec.ResultString());
        return -1;
    }

    if (argSpec.Flag(kFlagPresent))
        DoFlag();
    if (argSpec.Flag(kSlotPresent))
        AddSlot(slot);
    ShowName(name);     // guaranteed to have been set


When you call `Parse()`, the name, slot, and the flags `kSlotPresent` and
`kFlagPresent` are set appropriately, or an error returned if the arguments
given don't properly match the spec.


Format
======

The top-level specification consists of:

  * A string containing a description of the command.
  * A specification of the main arguments
  * Zero or more **option** specifications, each consisting of the option
    followed by a specification of its arguments, if any.
  * Zero or more **enum** specifications -- see the corresponding section below.
  * A `nullptr` indicating the end of the full specification.

An argument specification consists of a string with a list of arguments to be
parsed, followed by pointers to the corresponding variables to be set, followed
by a documentation string describing the effect of the arguments.

    argSpec.ConstructSpec
    (
        "Description",
        "<type> ...", typePtr, ...,
            "description of main arguments",
        "-option <type> ...", typePtr, ...,
            "description of this option",
        "=enum", "name", value, ..., nullptr,
        ...
        nullptr
    };


Basic Types
===========

Supported scalar types are:

    bool        // Format: true, false, yes, no, or integer. Shortcut: %b
    int         // Shortcut: %d
    float       // Shortcut: %f
    double      // Shortcut: %F
    cstring     // C++ type: const char*. Shortcut: %s
    string      // C++ type: std::string
    vec2        // C++ type: float v[2].
    vec3        // C++ type: float v[3].
    vec4        // C++ type: float v[4].

Types are specified either using printf-style '%' arguments as a shortcut, or
more fully within "<>" brackets, with an optional label used in the
documentation, for instance:

    "-time %f", &time,
    "-time <float>", &time,
    "-time <timeOfDay:float>", &time,


Flags
=====

A flag can be specified by appending "^" to the option or argument whose
presence should cause it to be set. Hence a simple flag can be specified by

    "-flag^", kOptionFlag,

but you can also set a flag if an argument, or even optional argument, is
specified, for example

    "-flag <int>^ [<string>^]", kOptionFlagHasInt, kOptionFlagHasString,



Optional Arguments
==================

You can mark an argument as optional by surrounding it by square brackets, in
which case it does not have to be specified. Without this, the parser will
report too few or too many arguments as an error.

You can have multiple optional arguments, in either the main argument list or
an option, but they must be nested, otherwise the parser can't tell which
comes when. So this is invalid:

    "<int> [<string>][<float> <float> <float>][<string>]"

and should instead be written as:

    "<int> [<string> [<float> <float> <float> [<string>]]]"


Array Types
===========

An array of a given type may be specified by appending "...", to indicate that
the initial argument and any further arguments are to be added to a vector<> of
the corresponding type. For example,

    vector<int> values;
    ...
        "-values <int> ...", &values,

And

    command -values 1 2 3 4 5

The array is terminated by either the end of the argument list, or another
option being specified. Alternatively you can use "--", which will be ignored,
which is useful if you want to end the array and then specify some main
arguments. E.g.,

    command -values 1 2 3 4 -- "main argument"

Alternatively, an array can be specified by appending "[]" to the type. For
example:

    vector<int> values;
    ...
        "-values <int[]>", &values,

With this style, the array contents are specified wholly by the next argument,
which must be quoted accordingly, e.g.,

    command -values "1 2 3 4 5"

This can be useful in situations where the command is being scripted, or you
want to avoid ambiguity between option names and array contents.


Enums
=====

You can define enums within the specification at any point by using
`"=enumName"`, after which `enumName` can used as a type name with an integer
argument. For example,

    "=colour", "red", 0, "blue", 1, "green", 2, nullptr,
    "-fg <colour>", &fgColour,
        "Set foreground colour",
    "-bg <colour>", &bgColour,
        "Set background colour",

The generated help will then include a definition of the enum.

You can also define enums out of line by using ":enumName", in conjunction with
`cArgEnumInfo`. (This can be useful if you want to reuse the enum name list in
other code.) For example,

    enum tColour { kRed, kGreen, kBlue };

    cArgEnumInfo kColourEnum[] = { "red", kRed, "green",  kGreen, "blue", kBlue, nullptr };

    ...
    ":colour", kColourEnum,
    ...


Help
====

By default the parser will look for the option `-h`, and return
`kArgHelpRequested` if found, with full text help present in `ErrorString()`.
You can override this behaviour by specifying `-h` as an option yourself.

You can also fetch the generated help manually, using one of the settings in
`tHelpType`. In addition to plain text, both html and markdown formats are
supported.


Example
=======

The included file `ArgSpecExample.cpp` contains a working example of usage.
See also [example-help.md](example-help.md), and [example-help.html](example-help.html).
