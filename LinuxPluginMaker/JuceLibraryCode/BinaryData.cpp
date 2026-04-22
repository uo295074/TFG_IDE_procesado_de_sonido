/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#include <cstring>

namespace BinaryData
{

//================== effects.xml ==================
static const unsigned char temp_binary_data_0[] =
"<effects>\r\n"
"\r\n"
"  <effect name=\"Gain\">\r\n"
"    <param name=\"Gain\"/>\r\n"
"  </effect>\r\n"
"\r\n"
"  <effect name=\"Distortion\">\r\n"
"    <param name=\"Drive\"/>\r\n"
"    <param name=\"Mix\"/>\r\n"
"    <selector name=\"Mode\">\r\n"
"      <option>Soft</option>\r\n"
"      <option>Hard</option>\r\n"
"      <option>Foldback</option>\r\n"
"    </selector>\r\n"
"  </effect>\r\n"
"\r\n"
"  <effect name=\"Filter\">\r\n"
"    <param name=\"Cutoff\"/>\r\n"
"    <selector name=\"Mode\">\r\n"
"      <option>Low-pass</option>\r\n"
"      <option>High-pass</option>\r\n"
"      <option>Band-pass</option>\r\n"
"    </selector>\r\n"
"  </effect>\r\n"
"\r\n"
"  <effect name=\"Tremolo\">\r\n"
"    <param name=\"Rate\"/>\r\n"
"    <param name=\"Depth\"/>\r\n"
"    <selector name=\"Wave\">\r\n"
"      <option>Sine</option>\r\n"
"      <option>Square</option>\r\n"
"      <option>Triangle</option>\r\n"
"    </selector>\r\n"
"  </effect>\r\n"
"\r\n"
"</effects>";

const char* effects_xml = (const char*) temp_binary_data_0;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes);
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;

    if (resourceNameUTF8 != nullptr)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x6415275a:  numBytes = 769; return effects_xml;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "effects_xml"
};

const char* originalFilenames[] =
{
    "effects.xml"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
        if (strcmp (namedResourceList[i], resourceNameUTF8) == 0)
            return originalFilenames[i];

    return nullptr;
}

}
