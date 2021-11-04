#pragma once

namespace snakeml
{

class RapidjsonUtils
{
public:
	static void ParseVectorValue(const rapidjson::Value& json, const char* name, vector& _outVector);
	static void ParseWstringValue(const rapidjson::Value& json, const char* name, std::wstring& _outWstring);
	static void ParseStringValue(const rapidjson::Value& json, const char* name, std::string& _outString);
	static void ParseArrayValue(const rapidjson::Value& json, const char* name, bool isOptional, const std::function<void(const rapidjson::Value*)>& elementCallback);
	static void ParseFloat3Value(const rapidjson::Value& json, const char* name, float3& _outFloat3);
	static void ParseFloat2Value(const rapidjson::Value& json, const char* name, float2& _outFloat2);
	static void ParseFloatValue(const rapidjson::Value& json, const char* name, float& _outFloat);
	static void ParseBoolValue(const rapidjson::Value& json, const char* name, bool& _outBool);
	static void ParseUintValue(const rapidjson::Value& json, const char* name, size_t& _outUint);

	template<typename EnumType>
	static void ParseEnumValue(const rapidjson::Value& json, const char* name, EnumType& _outEnum);
};

template<typename EnumType>
inline void RapidjsonUtils::ParseEnumValue(const rapidjson::Value& json, const char* name, EnumType& _outEnum)
{
	ASSERT(json.HasMember(name) && json[name].IsUint(), "[RapidjsonUtils::ParseEnumValue] : Invalid enum json.");
	const uint32_t enumData = json[name].GetUint();
	_outEnum = static_cast<EnumType>(enumData);
}

}