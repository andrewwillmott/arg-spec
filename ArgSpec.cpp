//
//  File:       ArgSpec.cpp
//
//  Function:   Argument parsing package
//
//  Copyright:  Andrew Willmott
//

#define _CRT_SECURE_NO_WARNINGS

#include "ArgSpec.h"

#include <string.h>
#include <stdarg.h>

#ifdef _MSC_VER
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
#endif

#define AS_MD_USE_DD     // Use <dd> tags for indentation, looks better with github-flavoured markdown

namespace AS
{

namespace
{
    const char* const kEllipsisToken   = "...";

    const char    kOpenBracketChar    = '[';
    const char    kCloseBracketChar   = ']';
    const char    kSetFlagChar        = '^';
    const char    kEnumSpecChar       = ':';
    const char    kEnumSpecInlineChar = '=';
    const char    kOptionChar         = '-';
    const char    kBeginArgChar       = '<';
    const char    kEndArgChar         = '>';
    const char    kArgSepChar         = ':';
    const char    kFormatChar         = '%';

    inline bool IsOption(const char* s)
    {
        return s[0] == kOptionChar && (isalpha(s[1]) || s[1] == kOptionChar);    // switch plus alpha character, to avoid picking up -10 etc.
    }

    inline bool Eq(const char* lhs, const char* rhs)
    {
        return strcasecmp(lhs, rhs) == 0;
    }

    inline bool Eq(const char* lhs, const char* rhs, size_t n)
    {
        return strncasecmp(lhs, rhs, n) == 0;
    }

    inline bool Eq(const string& lhs, const char* rhs)
    {
        return Eq(lhs.c_str(), rhs);
    }

    void Sprintf(string* pStr, const char* format, ...)
    {
        static char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, 1024, format, args);
        va_end(args);

        (*pStr) = buffer;
    }

    void SprintfAppend(string* pStr, const char* format, ...)
    {
        static char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, 1024, format, args);
        va_end(args);

        pStr->append(buffer);
    }

    void Split(const char* line, vector<const char*>* a, const char* separators = " \t", vector<char>* scratch = nullptr)
    // Splits 'line' into an array of tokens 'a', where each token is separated
    // by the characters in "sep" (default is white space).
    {
        static vector<char> sBuffer;
        if (!scratch)
            scratch = &sBuffer;

        const char* s = line;

        scratch->clear();
        size_t len = strlen(line);
        scratch->insert(scratch->begin(), s, s + len + 1);

        a->clear();

        s = strtok(scratch->data(), separators);
        if (!s)
            return;

        a->push_back(s);

        while ((s = strtok(0, separators)))
            a->push_back(s);
    }


    string NextToken(string& str, const char* separators = " \t\n")
    {
        string result(str);
        str = "";

        if (!separators || strlen(separators) == 0)
            return result;

        size_t index = result.find_first_of(separators);

        while (index == 0)
        {
            result.erase(0, 1);
            index = result.find_first_of(separators);
        }

        if (index == string::npos)
            return result;

        str.assign(result, index + 1, string::npos); // for some reason default last argument doesn't exist under current STL?
        result.assign(result, 0, index);

        return result;
    }


    // Numbers
    tArgError Parse(int* location, const char* arg, string* errorString)
    {
        char* sEnd;
        int result = (int) strtol(arg, &sEnd, 0);

        if (sEnd[0] != 0)
        {
            Sprintf(errorString, "Garbage at end of number: '%s' ", arg);
            return kArgErrorGarbage;
        }

        if (location)
            *location = result;

        return kArgNoError;
    }

    tArgError Parse(bool* location, const char* arg, string* errorString)
    {
        bool result;

        if (Eq(arg, "true"))
            result = true;
        else if (Eq(arg, "false"))
            result = false;
        else if (Eq(arg, "on"))
            result = true;
        else if (Eq(arg, "off"))
            result = false;
        else
        {
            int i;

            tArgError error = Parse(&i, arg, errorString);
            if (error != kArgNoError)
                return error;

            result = i != 0;
        }

        if (location)
            *location = result;

        return kArgNoError;
    }

    tArgError Parse(float* location, const char* arg, string* errorString)
    {
        char* sEnd;
        float result = strtof(arg, &sEnd);

        if (sEnd[0] != 0)
        {
            Sprintf(errorString, "Garbage at end of number: '%s' ", arg);
            return kArgErrorGarbage;
        }

        if (location)
            *location = result;

        return kArgNoError;
    }

    tArgError Parse(double* location, const char* arg, string* errorString)
    {
        char* sEnd;
        float result = strtof(arg, &sEnd);

        if (sEnd[0] != 0)
        {
            Sprintf(errorString, "Garbage at end of number: '%s' ", arg);
            return kArgErrorGarbage;
        }

        if (location)
            *location = result;

        return kArgNoError;
    }

    tArgError Parse(int n, float v[], const char**& argv, const char** argvEnd, string* errorString)
    {
        int i = 0;

        for (i = 0; i < n && argv < argvEnd && !IsOption(*argv); i++)
        {
            tArgError err = Parse(v + i, *argv++, errorString);

            if (err != kArgNoError)
                return err;
        }

        if (i == 1)     // treat single arg as vecn(s)
            for (int j = i; j < n; j++)
                v[j] = v[0];
        else            // otherwise zero-fill
            for (int j = i; j < n; j++)
                v[j] = 0.0f;

        return kArgNoError;
    }

    tArgError Parse(const char** location, const char* arg, string* )
    {
        if (location)
            *location = arg;
        return kArgNoError;
    }
    tArgError Parse(string* location, const char* arg, string* )
    {
        if (location)
            *location = arg;
        return kArgNoError;
    }

    // Enums
    struct cEnumSpec
    {
        cEnumSpec(const char* name, const cArgEnumInfo* enumInfo) :
            mName(name),
            mEnumInfo(enumInfo)
        {}

        string               mName;
        const cArgEnumInfo*  mEnumInfo;
        vector<cArgEnumInfo> mEnumInfoStore;    // for inline enums
    };

    tArgError Parse(int* location, const cEnumSpec& enumSpec, const char* arg, string* errorString)
    {
        const cArgEnumInfo* parseInfo = enumSpec.mEnumInfo;

        while (parseInfo->mToken)
        {
            if (Eq(arg, parseInfo->mToken))
            {
                if (location)
                    *location = parseInfo->mValue;

                return kArgNoError;
            }
            
            parseInfo++;
        }

        Sprintf(errorString, "Unknown enum '%s' of type %s", arg, enumSpec.mName.c_str());
        return kArgErrorBadEnum;
    }


    // Doc helpers
    void AddDocString(string* helpString, const char* leader, const string& docString)
    {
        size_t lastLF = 0;
        size_t nextLF;

        do
        {
            nextLF = docString.find('\n', lastLF);
            SprintfAppend(helpString, "%s%s\n", leader, docString.substr(lastLF, nextLF - lastLF).c_str());
            lastLF = nextLF + 1;
        }
        while (nextLF != string::npos);
    }

    void AddEnumDocs(string* helpString, const char* leader, const vector<cEnumSpec>& enumSpecs, tHelpType helpType)
    {
        for (size_t i = 0, n = enumSpecs.size(); i < n; i++)
        {
            if (helpType == kHelpHTML)
                SprintfAppend(helpString, "<p><b>%s</b></p><blockquote><i>", enumSpecs[i].mName.c_str());
            else if (helpType == kHelpMarkdown)
                SprintfAppend(helpString, "\n%s**%s**\n\n", leader, enumSpecs[i].mName.c_str());
            else
                SprintfAppend(helpString, "\n%s%s:\n", leader, enumSpecs[i].mName.c_str());

            const cArgEnumInfo* argEnum = enumSpecs[i].mEnumInfo;

            while (argEnum->mToken)
            {
                if (helpType == kHelpHTML)
                    SprintfAppend(helpString, "%s</br>\n", argEnum->mToken);
                else if (helpType == kHelpMarkdown)
                    SprintfAppend(helpString, "%s- %s\n", leader, argEnum->mToken);
                else
                    SprintfAppend(helpString, "%s   %s\n", leader, argEnum->mToken);
                argEnum++;
            }

            if (helpType == kHelpHTML)
                SprintfAppend(helpString, "</i></blockquote>");
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// cArgSpec::Internal declaration
//

namespace
{
    // Declarations
    enum tArgType : unsigned short
    {
        kTypeInvalid,
        kTypeBool,
        kTypeInt,
        kTypeFloat,
        kTypeDouble,
        kTypeCString,    // char*
        kTypeString,     // string
        kTypeVec2,
        kTypeVec3,
        kTypeVec4,
        kTypeEnumBegin,
        kTypeEnumEnd = kTypeEnumBegin + 1024,   // 1024 different enums should be enough for anyone™

        kNumTypes,

        kTypeArraySplitFlag  = 0x4000,     // array type: expecting "a b c ..."
        kTypeArrayListFlag   = 0x8000,     // array type, expecting to use remaining argument list (or until next option)
        kTypeBaseMask        = 0x3FFF
    };
    inline tArgType operator | (tArgType a, tArgType b)
    { return tArgType(int(a) | int(b)); }

    struct cArgInfo
    {
        tArgType     mType;        // type of argument
        string       mName;        // name for argument (optional)
        void*        mLocation;    // pointer to result
        bool         mIsRequired;  // present iff the previous argument is.
        int          mFlagToSet;   // if +ve, set this flag if we see this argument
    };

    struct cArgsSpec
    {
        vector<cArgInfo> mArguments;     // Arguments
        string           mDescription;   // What they do
    };

    struct cOptionsSpec : public cArgsSpec
    {
        string           mName;          // Name of option (-mName)
        int              mFlagToSet;     // if +ve, set this flag if we see this argument
    };
}

struct cArgSpec::Internal
{
    string               mCommandName;
    string               mCommandDescription;
    cArgsSpec            mMainArgs;
    vector<cOptionsSpec> mOptions;
    vector<cEnumSpec>    mEnumSpecs;
    uint32_t             mFlags = 0;
    bool                 mHelpRequested = false;

    mutable string       mErrorString;

    // Utilities
    tArgSpecError   ConstructSpec(const char* briefDescription, va_list args);

    tArgError       Parse(int argc, const char** argv);

    void            CreateHelpString(const char* commandName, string* pString, tHelpType helpType) const;

    tArgError       ParseArgument     (const cArgInfo& info, const char**& argv, const char** argvEnd);
    tArgError       ParseArrayArgument(const cArgInfo& info, const char**& argv, const char** argvEnd);

    tArgError       ParseOptionArgs(const vector<cArgInfo>& optArgs, const char**& argv, const char** argvEnd);
    tArgError       ParseOption(const char**& argv, const char** argvEnd);

    void            AddArgDocs(string* pString, const vector<cArgInfo>& args, tHelpType helpType) const;
    void            FindNameAndTypeFromOption(const string& str, tArgType* type, string* name) const;
    const char*     NameFromArgType(tArgType argType) const;
    tArgType        ArgTypeFromName(const char* typeName) const;
};


////////////////////////////////////////////////////////////////////////////////
// cArgSpec
//

cArgSpec::cArgSpec() :
    _(*(new Internal))
{
}

cArgSpec::~cArgSpec()
{
    delete &_;
}

tArgSpecError cArgSpec::ConstructSpec(const char* briefDescription, ...)
{
    va_list args;
    va_start(args, briefDescription);

    tArgSpecError err = _.ConstructSpec(briefDescription, args);

    va_end(args);
    return err;
}

tArgError cArgSpec::Parse(int argc, const char** argv)
{
    return _.Parse(argc, argv);
}

bool AS::cArgSpec::Flag(int flag) const
{
    AS_ASSERT(flag < int(sizeof(_.mFlags) * 8));
    return (_.mFlags & (1 << flag)) != 0;
}

void AS::cArgSpec::SetFlag(int flag)
{
    AS_ASSERT(flag < int(sizeof(_.mFlags) * 8));
    _.mFlags |= 1 << flag;
}

void cArgSpec::CreateHelpString(const char* commandName, string* pString, tHelpType helpType) const
{
    _.CreateHelpString(commandName, pString, helpType);
}

const char* cArgSpec::HelpString(const char* commandName, tHelpType helpType) const
{
    CreateHelpString(commandName, &_.mErrorString, helpType);
    return _.mErrorString.c_str();
}

const char* AS::cArgSpec::ErrorString()
{
    return _.mErrorString.c_str();
}


////////////////////////////////////////////////////////////////////////////////
// cArgSpec::Internal implementation
//

tArgSpecError cArgSpec::Internal::ConstructSpec(const char* description, va_list args)
{
    mCommandDescription = description;
    mMainArgs.mDescription.clear();
    mMainArgs.mArguments.clear();
    mOptions.clear();
    mEnumSpecs.clear();
    mErrorString.clear();

    tArgSpecError err = kSpecNoError;

    while (true)
    {
        const char* optionCStr = va_arg(args, const char*);
        if (!optionCStr)
            break;

        string options(optionCStr);
        string nextToken;

        // is this an enum specification? (:enumName).
        if (!options.empty() && (options[0] == kEnumSpecChar || options[0] == kEnumSpecInlineChar))
        {
            if (options[0] == kEnumSpecChar)
                mEnumSpecs.push_back(cEnumSpec(options.c_str() + 1, va_arg(args, cArgEnumInfo*)));
            else
            {
                mEnumSpecs.push_back(cEnumSpec(options.c_str() + 1, nullptr));

                while (true)
                {
                    const char* enumToken = va_arg(args, const char*);
                    if (!enumToken)
                        break;

                    int enumValue = va_arg(args, int);  // skip nth value
                    mEnumSpecs.back().mEnumInfoStore.push_back( { enumToken, enumValue } );
                }

                mEnumSpecs.back().mEnumInfoStore.push_back( { nullptr, 0 } );
                mEnumSpecs.back().mEnumInfo = mEnumSpecs.back().mEnumInfoStore.data();
            }

            continue;
        }

        nextToken = NextToken(options);

        vector<cArgInfo>* argsToAddTo = 0;
        cOptionsSpec newOption;

        // is it an option, or should it be added to the default arguments?
        bool isOption = IsOption(nextToken.c_str());

        if (isOption)
        {
            if (nextToken[nextToken.length() - 1] == kSetFlagChar)
            {
                nextToken.erase(nextToken.length() - 1, 1);
                newOption.mFlagToSet = va_arg(args, int);
            }
            else
                newOption.mFlagToSet = -1;

            newOption.mName.assign(nextToken, 1, string::npos);

            nextToken = NextToken(options);
            argsToAddTo = &newOption.mArguments;
        }
        else
            argsToAddTo = &mMainArgs.mArguments;

        int optionLevel = 0;
        int lastOptionLevel = 0;

        while (!nextToken.empty())
        {
            cArgInfo newArgInfo;

            if (nextToken[0] == kOpenBracketChar)
            {
                optionLevel++;
                nextToken.erase(0, 1);
            }

            newArgInfo.mIsRequired = (optionLevel == lastOptionLevel);

            while (!nextToken.empty() && nextToken[nextToken.size() - 1] == kCloseBracketChar)
            {
                optionLevel--;
                nextToken.erase(nextToken.length() - 1, 1);
            }

            if (Eq(nextToken, kEllipsisToken))
            {
                if (!options.empty())
                    return kSpecEllipsisError;
                if (argsToAddTo->empty())
                    return kSpecEllipsisError;

                argsToAddTo->back().mType = argsToAddTo->back().mType | kTypeArrayListFlag;
                nextToken = NextToken(options);
                continue;
            }

            // fetch var args: location and optionally flag
            newArgInfo.mLocation    = va_arg(args, void*);

            if (nextToken[nextToken.length() - 1] == kSetFlagChar)
            {
                nextToken.erase(nextToken.length() - 1, 1);
                newArgInfo.mFlagToSet = va_arg(args, int);
            }
            else
                newArgInfo.mFlagToSet = -1;

            FindNameAndTypeFromOption(nextToken, &newArgInfo.mType, &newArgInfo.mName);

            if (newArgInfo.mType == kTypeInvalid)
                err = kSpecUnknownType; // register error but carry on; we can mostly survive this.

            argsToAddTo->push_back(newArgInfo);

            lastOptionLevel = optionLevel;
            nextToken = NextToken(options);
        }

        if (optionLevel != 0)
        {
            err = kSpecUnbalancedBrackets;
            break;
        }

        const char* docCString = va_arg(args, const char*);
        AS_ASSERT_F(docCString && strlen(docCString) < 500, "Bad argument spec around '%s'\n", optionCStr);   // often we'll crash here if the spec is confused.
        string docString = docCString;

        if (isOption)
        {
            newOption.mDescription = docString;
            mOptions.push_back(newOption);
        }
        else
            mMainArgs.mDescription += docString;
    }

    return err;
}

tArgError cArgSpec::Internal::Parse(int argc, const char** argv)
{
    // chomp name.
    const char** argvEnd = argv + argc;
    const char* commandName = *argv++;

    // if they've supplied nothing at all, just show them the help
    if (argc == 1 && !mMainArgs.mArguments.empty())
    {
        CreateHelpString(commandName, &mErrorString, kHelpFull);
        return kArgHelpRequested;
    }

    // clear state
    mFlags = 0;
    mErrorString.clear();

    tArgError error;
    size_t i = 0;
    size_t n = mMainArgs.mArguments.size();

    while (argv < argvEnd)
    {
        if (IsOption(*argv))
        {
            error = ParseOption(argv, argvEnd);

            if (error != kArgNoError)
            {
                if (error == kArgHelpRequested)
                    CreateHelpString(commandName, &mErrorString, kHelpFull);

                return error;
            }
        }
        else if (i < n)
        {
            error = ParseArgument(mMainArgs.mArguments[i], argv, argvEnd);

            if (error != kArgNoError)
                return error;

            i++;
        }
        else
        {
            Sprintf(&mErrorString, "Too many main arguments (expecting at most %d)\n", n);
            return kArgErrorTooManyArgs;
        }
    }

    if (!mHelpRequested && i < n && mMainArgs.mArguments[i].mIsRequired)
    {
        size_t numHave = i;

        do
            i++;
        while (i < n && mMainArgs.mArguments[i].mIsRequired);

        Sprintf(&mErrorString, "Not enough main arguments: expecting at least %d more", i - numHave);

        return kArgErrorNotEnoughArgs;
    }

    return kArgNoError;
}

void cArgSpec::Internal::CreateHelpString(const char* commandName, string* helpString, tHelpType helpType) const
{
    if (helpType == kHelpBrief)
    {
        Sprintf(helpString, "%s, %s", commandName, mCommandDescription.c_str());
        return;
    }

    if (helpType == kHelpHTML)
    {
        SprintfAppend(helpString, "<tr><td><a name=\"%s\"></a>", commandName);    // start frame
        SprintfAppend(helpString, "<p>%s</p>\n\n", mCommandDescription.c_str());
        SprintfAppend(helpString, "<p><h3>Usage</h3></p>\n");

        SprintfAppend(helpString, "<b>%s</b> ", commandName);

        if (!mOptions.empty())
            *helpString += "[options] ";

        AddArgDocs(helpString, mMainArgs.mArguments, helpType);

        helpString->append("<br><blockquote><p>");
        AddDocString(helpString, "", mMainArgs.mDescription);
        helpString->append("</blockquote>\n");

        if (!mOptions.empty())
        {
            SprintfAppend(helpString, "<p><h3>Options</h3></p>\n");

            for (size_t j = 0, nj = mOptions.size(); j < nj; j++)
            {
                SprintfAppend(helpString, "<b>-%s</b> ", mOptions[j].mName.c_str());
                AddArgDocs(helpString, mOptions[j].mArguments, helpType);
                helpString->append("<br><blockquote>");
                AddDocString(helpString, "", mOptions[j].mDescription);
                helpString->append("</blockquote>");
            }
        }

        if (!mEnumSpecs.empty())
        {
            helpString->append("\n<p><h3>Types</h3></p>");
            AddEnumDocs(helpString, "", mEnumSpecs, helpType);
        }

        helpString->append("</td></tr>");       // end frame
        return;
    }

    if (helpType == kHelpMarkdown)
    {
    #ifdef AS_MD_USE_DD
        SprintfAppend(helpString, "%s\n\n### Usage\n\n**%s** ", mCommandDescription.c_str(), commandName);
    #else
        SprintfAppend(helpString, "%s\n\n### Usage\n> **%s** ", mCommandDescription.c_str(), commandName);
    #endif

        if (!mOptions.empty())
            *helpString += "[*options*] ";

        AddArgDocs(helpString, mMainArgs.mArguments, helpType);
    #ifdef AS_MD_USE_DD
        *helpString += "\n<dl><dd>    ";
        *helpString += mMainArgs.mDescription;
        *helpString += "    </dd></dl>\n\n";
    #else
        *helpString += ">>  ";
        *helpString += mMainArgs.mDescription;
        *helpString += "\n";
    #endif
        if (!mOptions.empty())
        {
            SprintfAppend(helpString, "\n### Options\n\n");

            for (size_t j = 0, nj = mOptions.size(); j < nj; j++)
            {
            #ifdef AS_MD_USE_DD
                SprintfAppend(helpString, "**-%s** ", mOptions[j].mName.c_str());
            #else
                SprintfAppend(helpString, "> **-%s** ", mOptions[j].mName.c_str());
            #endif
                AddArgDocs(helpString, mOptions[j].mArguments, helpType);
            #ifdef AS_MD_USE_DD
                *helpString += "\n<dl><dd>    ";
                *helpString += mOptions[j].mDescription;
                *helpString += "    </dd></dl>\n\n";
            #else
                *helpString += ">>  ";
                *helpString += mOptions[j].mDescription;
                *helpString += "\n\n";
            #endif
            }
        }

        if (!mEnumSpecs.empty())
        {
            SprintfAppend(helpString, "\n### Types\n");
        #ifdef AS_MD_USE_DD
            AddEnumDocs(helpString, "", mEnumSpecs, helpType);
        #else
            AddEnumDocs(helpString, "> ", mEnumSpecs, helpType);
        #endif
        }

        return;
    }

    SprintfAppend(helpString, "%s\n\nUsage:\n    %s ", mCommandDescription.c_str(), commandName);

    if (!mOptions.empty())
        *helpString += "[options] ";

    AddArgDocs(helpString, mMainArgs.mArguments, helpType);
    AddDocString(helpString, "        ", mMainArgs.mDescription);

    if (!mOptions.empty())
    {
        SprintfAppend(helpString, "\nOptions:\n");

        for (size_t j = 0, nj = mOptions.size(); j < nj; j++)
        {
            SprintfAppend(helpString, "    -%s ", mOptions[j].mName.c_str());
            AddArgDocs(helpString, mOptions[j].mArguments, helpType);
            AddDocString(helpString, "        ", mOptions[j].mDescription);
        }
    }

    if (!mEnumSpecs.empty())
    {
        SprintfAppend(helpString, "\nTypes:");
        AddEnumDocs(helpString, "    ", mEnumSpecs, helpType);
    }
}

tArgError cArgSpec::Internal::ParseArgument(const cArgInfo& info, const char**& argv, const char** argvEnd)
{
    if (info.mFlagToSet >= 0)
        mFlags |= 1 << info.mFlagToSet;

    if (info.mType & kTypeArrayListFlag)
    {
        // We are making the assumption here that vector<> has the same layout and clear implementation for all types
        static_cast<vector<int>*>(info.mLocation)->clear();

        do
        {
            if (IsOption(argv[0]))
                return kArgNoError;

            tArgError error = ParseArrayArgument(info, argv, argvEnd);

            if (error != kArgNoError)
                return error;
        }
        while (argv < argvEnd);

        return kArgNoError;
    }

    if (info.mType & kTypeArraySplitFlag)
    {
        tArgError err;

        vector<const char*> arrayArgs;
        Split(*argv++, &arrayArgs);

        // We are making the assumption here that vector<> has the same layout and clear implementation for all types
        if (info.mLocation)
            static_cast<vector<int>*>(info.mLocation)->clear();

        const char** aargv    = arrayArgs.data();
        const char** aargvEnd = aargv + arrayArgs.size();

        while (aargv < aargvEnd)
            if ((err = ParseArrayArgument(info, aargv, aargvEnd)) != kArgNoError)
                return err;

        return kArgNoError;
    }

    switch (info.mType)
    {
    case kTypeBool:
        return AS::Parse(static_cast<bool  *>     (info.mLocation), *argv++, &mErrorString);
    case kTypeInt:
        return AS::Parse(static_cast<int   *>     (info.mLocation), *argv++, &mErrorString);
    case kTypeFloat:
        return AS::Parse(static_cast<float *>     (info.mLocation), *argv++, &mErrorString);
    case kTypeDouble:
        return AS::Parse(static_cast<double*>     (info.mLocation), *argv++, &mErrorString);
    case kTypeCString:
        return AS::Parse(static_cast<const char**>(info.mLocation), *argv++, &mErrorString);
    case kTypeString:
        return AS::Parse(static_cast<string*>     (info.mLocation), *argv++, &mErrorString);
    case kTypeVec2:
    case kTypeVec3:
    case kTypeVec4:
        return AS::Parse(2 + info.mType - kTypeVec2, static_cast<float*>(info.mLocation), argv, argvEnd, &mErrorString);

    default:
        if (info.mType >= kTypeEnumBegin && size_t(info.mType - kTypeEnumBegin) < mEnumSpecs.size())
        {
            int enumIndex = info.mType - kTypeEnumBegin;

            return AS::Parse(static_cast<int*>(info.mLocation), mEnumSpecs[enumIndex], *argv++,&mErrorString);
        }

        Sprintf(&mErrorString, "Unknown arg type %d", info.mType);
        return kArgErrorBadSpec;
    }
}

namespace
{
    typedef struct { float _[2]; } Vec2;
    typedef struct { float _[3]; } Vec3;
    typedef struct { float _[4]; } Vec4;

    template<class T> tArgError Parse(vector<T>* location, const char* arg, string* errorString)
    {
        T result;

        tArgError error = AS::Parse(&result, arg, errorString);
        if (error != kArgNoError)
            return error;

        if (location)
            location->push_back(result);

        return error;
    }

    template<class T> tArgError Parse(vector<T>* location, const char**& argv, const char** argvEnd, string* errorString)
    {
        T result;

        tArgError error = AS::Parse(sizeof(T) / sizeof(T()._[0]), result._, argv, argvEnd, errorString);

        if (error != kArgNoError)
            return error;

        if (location)
            location->push_back(result);

        return error;
    }

    template<class T> tArgError Parse(vector<T>* location, const cEnumSpec& enumSpec, const char* arg, string* errorString)
    {
        T result;

        tArgError error = AS::Parse(&result, enumSpec, arg, errorString);

        if (error != kArgNoError)
            return error;

        if (location)
            location->push_back(result);

        return error;
    }

}

tArgError cArgSpec::Internal::ParseArrayArgument(const cArgInfo& info, const char**& argv, const char** argvEnd)
{
    int type = info.mType & kTypeBaseMask;

    switch (type)
    {
    case kTypeBool:
        return AS::Parse(static_cast<vector<bool>*>       (info.mLocation), *argv++, &mErrorString);
    case kTypeInt:
        return AS::Parse(static_cast<vector<int>*>        (info.mLocation), *argv++, &mErrorString);
    case kTypeFloat:
        return AS::Parse(static_cast<vector<float>*>      (info.mLocation), *argv++, &mErrorString);
    case kTypeDouble:
        return AS::Parse(static_cast<vector<double>*>     (info.mLocation), *argv++, &mErrorString);
    case kTypeCString:
        return AS::Parse(static_cast<vector<const char*>*>(info.mLocation), *argv++, &mErrorString);
    case kTypeString:
        return AS::Parse(static_cast<vector<string>*>     (info.mLocation), *argv++, &mErrorString);
    case kTypeVec2:
        return AS::Parse(static_cast<vector<Vec2>*>       (info.mLocation), argv, argvEnd, &mErrorString);
    case kTypeVec3:
        return AS::Parse(static_cast<vector<Vec3>*>       (info.mLocation), argv, argvEnd, &mErrorString);
    case kTypeVec4:
        return AS::Parse(static_cast<vector<Vec4>*>       (info.mLocation), argv, argvEnd, &mErrorString);

    default:
        if (type >= kTypeEnumBegin && size_t(type - kTypeEnumBegin) < mEnumSpecs.size())
        {
            int enumIndex = type - kTypeEnumBegin;

            return AS::Parse(static_cast<vector<int>*>(info.mLocation), mEnumSpecs[enumIndex], *argv++, &mErrorString);
        }

        Sprintf(&mErrorString, "Unknown array argument type %d", type);
        return kArgErrorBadSpec;
    }
}

tArgError cArgSpec::Internal::ParseOptionArgs(const vector<cArgInfo>& optArgs, const char**& argv, const char** argvEnd)
{
    size_t i = 0;
    size_t n = optArgs.size();

    while (argv < argvEnd && i < n && !IsOption(*argv))
    {
        tArgError error = ParseArgument(optArgs[i], argv, argvEnd);

        if (error != kArgNoError)
            return error;

        i++;
    }

    if (i < n && optArgs[i].mIsRequired)
    {
        size_t numHave = i;

        do
            i++;
        while (i < n && optArgs[i].mIsRequired);

        Sprintf(&mErrorString, "Not enough arguments: expecting at least %d more", i - numHave);

        return kArgErrorNotEnoughArgs;
    }

    return kArgNoError;
}

tArgError cArgSpec::Internal::ParseOption(const char**& argv, const char** argvEnd)
{
    const char* optionName = argv[0] + 1;

    argv++;

    if (optionName[0] == kOptionChar)   // handle '--option', and skip '--', used as an open-ended list terminator
    {
        optionName++;
        if (optionName[0] == 0)
            return kArgNoError;
    }

    for (size_t i = 0, n = mOptions.size(); i < n; i++)
    {
        if (Eq(optionName, "h"))
            mHelpRequested = true;

        if (Eq(mOptions[i].mName, optionName))
        {
            if (mOptions[i].mFlagToSet >= 0)
                mFlags |= 1 << mOptions[i].mFlagToSet;

            tArgError err = ParseOptionArgs(mOptions[i].mArguments, argv, argvEnd);

            if (err != kArgNoError)
            {
                mErrorString += " in -";
                mErrorString += optionName;
            }

            return err;
        }
    }

    if (mHelpRequested)
        return kArgHelpRequested;

    Sprintf(&mErrorString, "Unknown option '%s'", optionName);
    return kArgErrorUnknownOption;
}

void cArgSpec::Internal::AddArgDocs(string* helpString, const vector<cArgInfo>& args, tHelpType helpType) const
{
    int numClauses = 0;

    if (helpType == kHelpHTML)
        helpString->append("<i>");

    for (size_t i = 0, n = args.size(); i < n; i++)
    {
        const cArgInfo& ai = args[i];

        if (i != 0)
            helpString->append(" ");

        if (!ai.mIsRequired)
        {
            helpString->push_back(kOpenBracketChar);
            numClauses++;
        }

        if (helpType == kHelpHTML)
            helpString->append("&lt;");
        else if (helpType != kHelpMarkdown)
            helpString->append("<");

        if (!ai.mName.empty())
            SprintfAppend(helpString, "%s:", ai.mName.c_str());

        if (helpType == kHelpMarkdown)
            helpString->append("_");

        SprintfAppend(helpString, "%s", NameFromArgType(ai.mType));

        if (helpType == kHelpHTML)
            helpString->append("&gt;");
        else if (helpType == kHelpMarkdown)
            helpString->append("_");
        else
            helpString->append(">");

        if (ai.mType & kTypeArrayListFlag)
        {
            helpString->append(" ");
            helpString->append(kEllipsisToken);
        }
    }

    for (int i = 0; i < numClauses; i++)
        helpString->push_back(kCloseBracketChar);

    if (helpType == kHelpHTML)
        helpString->append("</i>");

    helpString->append("\n");
}

void cArgSpec::Internal::FindNameAndTypeFromOption(const string& str, tArgType* type, string* name) const
{
    (*name) = "";

    if (str.empty())
    {
        *type = kTypeInvalid;
        return;
    }
    if (str.size() > 1 && str[0] == kFormatChar)
    {
        switch (str[1])
        {
        case 'f':
        case 'g':
            *type = kTypeFloat;
            break;
        case 'F':
        case 'G':
            *type = kTypeDouble;
            break;
        case 'd':
            *type = kTypeInt;
            break;
        case 'b':
            *type = kTypeBool;
            break;
        case 's':
            *type = kTypeCString;
            break;
        default:
            *type = kTypeInvalid;
        }
    }
    else
    {
        string typeStr(str);

        if (typeStr[0] == kBeginArgChar && typeStr[typeStr.size() - 1] == kEndArgChar)
        {
            typeStr.erase(typeStr.size() - 1, 1);
            typeStr.erase(0, 1);
        }

        size_t sepPos = typeStr.find(kArgSepChar);
        if (sepPos != string::npos)
        {
            name->assign(typeStr, 0, sepPos);
            typeStr = string(typeStr, sepPos + 1, string::npos);
        }

        (*type) = ArgTypeFromName(typeStr.c_str());
    }
}

const char* cArgSpec::Internal::NameFromArgType(tArgType argType) const
{
    static string sResult;

    int baseArgType = argType & kTypeBaseMask;

    switch (baseArgType)
    {
    case kTypeBool:
        sResult = "bool";
        break;
    case kTypeInt:
        sResult = "int";
        break;
    case kTypeFloat:
        sResult = "float";
        break;
    case kTypeDouble:
        sResult = "double";
        break;
    case kTypeCString:
    case kTypeString:
        sResult = "string";
        break;

    case kTypeVec2:
        sResult = "vec2";
        break;
    case kTypeVec3:
        sResult = "vec3";
        break;
    case kTypeVec4:
        sResult = "vec4";
        break;

    case kTypeInvalid:
        sResult = "invalid";
        break;
    default:
        if (baseArgType >= kTypeEnumBegin && baseArgType < kTypeEnumEnd && size_t(baseArgType - kTypeEnumBegin) < mEnumSpecs.size())
            sResult = mEnumSpecs[baseArgType - kTypeEnumBegin].mName;
        else
            sResult = "unknown";
    }

    if (argType & (kTypeArraySplitFlag))
        sResult += "[]";

    return sResult.c_str();
}

tArgType cArgSpec::Internal::ArgTypeFromName(const char* typeName) const
{
    tArgType arrayFlag = tArgType(0);
    size_t typeLen = strlen(typeName);

    if (typeLen > 2 && typeName[typeLen - 2] == '[' && typeName[typeLen - 1] == ']')
    {
        arrayFlag = kTypeArraySplitFlag;
        typeLen -= 2;
    }

    if (Eq(typeName, "bool", typeLen))
        return kTypeBool | arrayFlag;
    if (Eq(typeName, "int", typeLen))
        return kTypeInt | arrayFlag;
    if (Eq(typeName, "float", typeLen))
        return kTypeFloat | arrayFlag;
    if (Eq(typeName, "double", typeLen))
        return kTypeDouble | arrayFlag;
    if (Eq(typeName, "string", typeLen))
        return kTypeString | arrayFlag;
    if (Eq(typeName, "cstr", typeLen) || Eq(typeName, "cstring", typeLen))
        return kTypeCString | arrayFlag;

    if (typeName[0] == 'v')
    {
        if (Eq(typeName, "v2", typeLen) || Eq(typeName, "vec2", typeLen) || Eq(typeName, "vector2", typeLen))
            return kTypeVec2 | arrayFlag;
        if (Eq(typeName, "v3", typeLen) || Eq(typeName, "vec3", typeLen) || Eq(typeName, "vector3", typeLen))
            return kTypeVec3 | arrayFlag;
        if (Eq(typeName, "v4", typeLen) || Eq(typeName, "vec4", typeLen) || Eq(typeName, "vector4", typeLen))
            return kTypeVec4 | arrayFlag;
    }

    for (size_t i = 0, n = mEnumSpecs.size(); i < n; i++)
        if (Eq(mEnumSpecs[i].mName, typeName))
            return tArgType(kTypeEnumBegin + i) | arrayFlag;

    AS_ASSERT_F(0, "bad argument spec type\n");
    return kTypeInvalid;
}

}
