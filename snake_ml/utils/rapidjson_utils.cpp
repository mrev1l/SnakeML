// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include "stdafx.h"
#include "rapidjson_utils.h"

#include "system/drivers/win/os/helpers/win_utils.h"// TODO Fix closs platform

namespace snakeml
{

void RapidjsonUtils::ParseVectorValue(const rapidjson::Value& json, const char* name, vector& _outVector)
{
	ASSERT(json.HasMember(name) && json[name].IsArray() && json[name].Size() == 3u, "[RapidjsonUtils::ParseVectorValue] : Invalid vector json.");
	const rapidjson::GenericArray<true, rapidjson::Value>& vectorJson = json[name].GetArray();
	_outVector = { vectorJson[0].GetFloat(), vectorJson[1].GetFloat(), vectorJson[2].GetFloat() };
}

void RapidjsonUtils::ParseWstringValue(const rapidjson::Value& json, const char* name, std::wstring& _outWstring)
{
	ASSERT(json.HasMember(name) && json[name].IsString(), "[RapidjsonUtils::ParseWstringValue] : Invalid string json.");
	const std::string string = json[name].GetString();
	win::WinUtils::StringToWstring(string.c_str(), _outWstring); // TODO Fix closs platform
}

void RapidjsonUtils::ParseStringValue(const rapidjson::Value& json, const char* name, std::string& _outString)
{
	ASSERT(json.HasMember(name) && json[name].IsString(), "[RapidjsonUtils::ParseWstringValue] : Invalid string json.");
	_outString = json[name].GetString();
}

void RapidjsonUtils::ParseArrayValue(const rapidjson::Value& json, const char* name, const std::function<void(const rapidjson::Value*)>& elementCallback)
{
	ASSERT(json.HasMember(name) && json[name].IsArray(), "[RapidjsonUtils::ParseArrayValue] : Invalid array json.");

	const rapidjson::GenericArray<true, rapidjson::Value>& array = json[name].GetArray();
	for (rapidjson::Value::ConstValueIterator vertexIt = array.Begin(); vertexIt != array.End(); ++vertexIt)
	{
		elementCallback(vertexIt);
	}
}

void RapidjsonUtils::ParseFloat3Value(const rapidjson::Value& json, const char* name, float3& _outFloat3)
{
	ASSERT(json.HasMember(name) && json[name].IsArray() && json[name].Size() == 3u, "[RapidjsonUtils::ParseFloat3Value] : Invalid float3 json.");
	const rapidjson::GenericArray<true, rapidjson::Value>& float3Json = json[name].GetArray();
	_outFloat3 = { float3Json[0].GetFloat(), float3Json[1].GetFloat(), float3Json[2].GetFloat() };
}

void RapidjsonUtils::ParseFloat2Value(const rapidjson::Value& json, const char* name, float2& _outFloat2)
{
	ASSERT(json.HasMember(name) && json[name].IsArray() && json[name].Size() == 2u, "[RapidjsonUtils::ParseFloat2Value] : Invalid float2 json.");
	const rapidjson::GenericArray<true, rapidjson::Value>& float2Json = json[name].GetArray();
	_outFloat2 = { float2Json[0].GetFloat(), float2Json[1].GetFloat() };
}

void RapidjsonUtils::ParseFloatValue(const rapidjson::Value& json, const char* name, float& _outFloat)
{
	ASSERT(json.HasMember(name) && json[name].IsFloat(), "[RapidjsonUtils::ParseFloatValue] : Invalid float json.");
	_outFloat = json[name].GetFloat();
}

void RapidjsonUtils::ParseBoolValue(const rapidjson::Value& json, const char* name, bool& _outBool)
{
	ASSERT(json.HasMember(name) && json[name].IsBool(), "[RapidjsonUtils::ParseBoolValue] : Invalid bool json.");
	_outBool = json[name].GetBool();
}

void RapidjsonUtils::ParseUintValue(const rapidjson::Value& json, const char* name, size_t& _outUint)
{
	ASSERT(json.HasMember(name) && json[name].IsUint(), "[RapidjsonUtils::ParseUintValue] : Invalid uint json.");
	_outUint = json[name].GetUint();
}

}