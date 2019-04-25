//
//    File:       ArgSpecExample.cpp
//
//    Function:   Test for ArgSpec
//

#include "ArgSpec.h"


using namespace AS;

enum tColour : int
{
    kRed,
    kGreen,
    kBlue,
    kBlack
};

cArgEnumInfo kColourEnum[] =
{
   "red",    kRed,
   "green",  kGreen,
   "blue",   kBlue,
   "black",  kBlack,
   nullptr, 0
};

struct cCommand
{
    enum 
    {
        kOptionVerbose,
        kOptionHaveDest,
        kOptionSize,
        kOptionGamma,
        kOptionScaleXYZ,
        kOptionHelp,
    };

    string      mName;
    const char* mDestination    = "/dev/null";
    int         mSize           = 100;
    double      mGamma          = 2.2;
    bool        mEnableCats     = false;
    float       mLatitude       = 0;
    float       mLongitude      = 0;
    int         mJulianDay      = 1;
    
    float       mV2[2] = {};
    float       mV3[3] = {};
    float       mV4[4] = {};

    float       mScaleXYZ[3] = {};

    vector<int>         mCounts         = { 1, 2, 3 };
    vector<const char*> mWords;
    vector<tColour>     mColours;

    typedef struct { float x, y      ; } float2;
    typedef struct { float x, y, z   ; } float3;
    typedef struct { float x, y, z, w; } float4;

    vector<float3>      mV3s;

    tColour     mColour         = kBlack;
    tHelpType   mHelpType       = kHelpFull;

    cArgSpec mArgSpec;
    
    cCommand()
    {
        mArgSpec.ConstructSpec
        (
            "Provides an example of ArgSpec usage",

            "<name:string> [<dst:cstr>^]", &mName, &mDestination, kOptionHaveDest,
               "Specify name and optionally destination for display",

            "-v^", kOptionVerbose,
                "Set verbose mode",
            "-size^ %d", kOptionSize, &mSize,
                "Set image/window size",
            "-gamma^ <gamma:double>", kOptionGamma, &mGamma,
                "set gamma correction (default: 2.2)",
            "-cats <bool>", &mEnableCats,
                "Whether cats are enabled (default: false)",
            "-latlong <latitude:float> <longitude:float>",
                &mLatitude, &mLongitude,
                "Set latitude and longitude",
            "-day <day:int>", &mJulianDay,
                "Set Julian day (1..365)",

            ":colour", kColourEnum,         // defining this externally so it can be used in PrintVariables
            "-colour <colour>", &mColour,
                "Set colour",

            "-v2 <vec2>", &mV2[0],
                "Set v2",
            "-v3 <vec3>", mV3,
                "Set v3",
            "-v4 <vec4>", mV4,
                "Set v4",

            "-scale %f [%f %f^]", mScaleXYZ + 0, mScaleXYZ + 1, mScaleXYZ + 2, kOptionScaleXYZ,
                "Set uniform or xyz scale",

            "-counts <count1:int> ...", &mCounts,
                "Specify counts using repeated arguments",
            "-countArray <counts:int[]>", &mCounts,
                "Specify counts as explicit, quoted array",
            "-words <name1:cstring> ...", &mWords,
                "Specify words",
            "-v3s <vec3> ...", &mV3s,
                "Specify v3s",
            "-colours <colour> ...", &mColours,
                "Specify colours",

            "=helpType", "brief", kHelpBrief, "full", kHelpFull, "html", kHelpHTML, "md", kHelpMarkdown, nullptr,
            "-h^ [<helpType>]", kOptionHelp, &mHelpType,
                "Show full help, or help of the given type",
            0, 0
        );
    }

    void PrintVariables()
    {
        if (!mArgSpec.Flag(kOptionScaleXYZ))
            mScaleXYZ[1] = mScaleXYZ[2] = mScaleXYZ[0];

        printf("\nflags:\n");

        if (mArgSpec.Flag(kOptionVerbose))
            printf("verbose\n");
        if (mArgSpec.Flag(kOptionHaveDest))
            printf("dest\n");
        if (mArgSpec.Flag(kOptionSize))
            printf("size\n");
        if (mArgSpec.Flag(kOptionGamma))
            printf("gamma\n");

        printf("\nvalues:\n");

        printf
        (
            "Name       : %s\n"
            "Destination: %s\n"
            "Size       : %d\n"
            "Gamma      : %g\n"
            "Cats       : %s\n"
            "Lat/Long   : %g, %g\n"
            "JulianDay  : %d\n"
            "Colour     : %s\n"
            "V2         : %f %f\n"
            "V3         : %f %f %f\n"
            "V4         : %f %f %f %f\n"
            "ScaleXYZ   : %f %f %f\n",
            mName.c_str(),
            mDestination,
            mSize,
            mGamma,
            mEnableCats ? "YES" : "NO",
            mLatitude,
            mLongitude,
            mJulianDay,
            kColourEnum[mColour].mToken,
            mV2[0], mV2[1],
            mV3[0], mV3[1], mV3[2],
            mV4[0], mV4[1], mV4[2], mV4[3],
            mScaleXYZ[0], mScaleXYZ[1], mScaleXYZ[2]
        );

        if (!mCounts.empty())
        {
            printf("Counts     :");
            for (int count : mCounts)
                printf(" %d", count);
            printf("\n");
        }

        if (!mWords.empty())
        {
            printf("Words      :");
            for (const char* word : mWords)
                printf(" '%s'", word);
            printf("\n");
        }

        if (!mColours.empty())
        {
            printf("Colours   :");
            for (tColour colour : mColours)
                printf(" '%s'", kColourEnum[colour].mToken);
            printf("\n");
        }

        if (!mV3s.empty())
        {
            printf("V3s       :");
            for (int i = 0; i < (int) mV3s.size(); i++)
                printf(" [%f %f %f]", mV3s[i].x, mV3s[i].y, mV3s[i].z);
            printf("\n");
        }
    }
};


int main(int argc, const char** argv)
{
    cCommand test;

    tArgError err = test.mArgSpec.Parse(argc, argv);

    if (err != kArgNoError)
    {
        printf("%s\n", test.mArgSpec.ErrorString());
        return -1;
    }    

    if (test.mArgSpec.Flag(test.kOptionHelp))
    {
        string helpString;

        test.mArgSpec.CreateHelpString("test", &helpString, test.mHelpType);
        printf("%s\n", helpString.c_str());
        return 0;
    }

    test.PrintVariables();

    return 0;
}

