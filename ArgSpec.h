//
//  File:       ArgSpec.h
//
//  Function:   Command-line argument parsing package
//
//  Copyright:  Andrew Willmott
//

#ifndef ARG_SPEC_H
#define ARG_SPEC_H

#include <string>
#include <vector>

#ifndef AS_ASSERT
    #ifndef NDEBUG
        #include <assert.h>
        #define AS_ASSERT assert
        #define AS_ASSERT_F(M_E, ...) if (!(M_E)) { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); assert(0); }
    #else
        #define AS_ASSERT(M_E)
        #define AS_ASSERT_F(M_E, ...)
    #endif
#endif

namespace AS
{
    using std::string;
    using std::vector;

    enum tHelpType : int
    {
        kHelpBrief,         ///< One-line help.
        kHelpFull,          ///< Full plain text help, including options.
        kHelpHTML,          ///< HTML-formatted help
        kHelpMarkdown,      ///< Markdown-formatted help
        kNumArgHelpTypes
    };
    
    enum tArgError : int
    {
        kArgNoError,
        kArgHelpRequested,
        kArgErrorNotEnoughArgs,
        kArgErrorTooManyArgs,
        kArgErrorBadSpec,
        kArgErrorUnknownOption,
        kArgErrorBadEnum,
        kArgErrorGarbage,
        kNumArgErrors
    };
    
    enum tArgSpecError : int
    {
        kSpecNoError,
        kSpecUnbalancedBrackets,   ///< unbalanced [/]
        kSpecEllipsisError,        ///< unexpected ...
        kSpecUnknownType,          ///< unrecognized argument type
        kNumSpecErrors
    };

    struct cArgEnumInfo
    {
        const char* mToken;
        int         mValue;
    };


    class cArgSpec
    /** Provides a specification for how to parse a command line,
        and a mechanism for performing the parsing.
    
        The cArgSpec class binds a set of default arguments
        and options to a set of C++ variables. This is done
        via a simple vararg-style specification.
        
        The class handles bools, ints, floats, strings, enums,
        vectors of any of these, and repeated arguments. It also
        handles optional arguments, error detection, and
        help.
        
        Here's a simple example:
        
            string name;
            int slot;
            float pos[3] = {};

            cArgSpec argSpec;
            argSpec.ConstructSpec
            (
                 "Description",
                    "<name:string> [<slot:int>^]", &name, &slot, kSlotPresent,
                    "Purpose of default arguments",
                 "-flag^", kFlagPresent,
                    "What the flag does",
                 "-pos <vec3>", pos,
                    "Set position",
                 nullptr
            );

            tArgError err = argSpec.Parse(argc, argv);

            if (err != kArgNoError)
            {
                printf("%s\n", argSpec.ErrorString());
                return -1;
            }

            if (argSpec.Flag(kFlagPresent))
                ...

        When you call Parse(), name, slot and the flags kSlotPresent and 
        kFlagPresent are set appropriately, or an error returned if the 
        arguments don't properly match the spec. Moreover, you can retrieve a 
        help string based on the specification via HelpString().
    */
    {
    public:
        // Creators
        cArgSpec();
        ~cArgSpec();

        // cArgSpec
        tArgSpecError ConstructSpec(const char* briefDescription, ...);
        ///< Construct the specification, binding arguments to variables. After this, Parse() may be called repeatedly.

        tArgError Parse(int argc, const char** argv);
        ///< Parse C-style args according to the previously set specification.

        bool Flag(int flag) const;
        ///< Returns value of given flag, set by Parse(). All flags are cleared before Parse() does its job.

        void SetFlag(int flag);
        ///< Set the given flag. Generally flags are set by this class as the result of a Parse() call,
        ///< but it is occasionally useful to set them externally during post-Parse() processing.

        void CreateHelpString(const char* commandName, string* pString, tHelpType helpType = kHelpFull) const;
        ///< Create the given kind of help in pString.
        const char* HelpString(const char* commandName, tHelpType helpType = kHelpFull) const;
        ///< Return given type of help: this also sets ResultString().
        
        const char* ErrorString();
        ///< Returns description of the results of the last call to Parse().
        
    protected:
        struct Internal;
        Internal&   _;
    };
}

#endif
