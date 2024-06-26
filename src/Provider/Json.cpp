#include <MHTR/Provider/Json.h>

using namespace MHTR;

JsonProvider::JsonProvider(const char* jsonSrc)
    : mJson(nlohmann::json::parse(jsonSrc))
{}

JsonProvider::JsonProvider(const nlohmann::json& json)
    : mJson(json)
{}

nlohmann::json* JsonProvider::GetJson()
{
    return &mJson;
}
