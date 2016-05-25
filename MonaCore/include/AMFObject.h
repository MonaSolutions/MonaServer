#pragma once
#include <string>
#include <map>
#include "Mona/AMF.h"
#include "Mona/DataWriter.h"
using namespace std;
namespace Mona
{
	struct AMFObject {
		typedef map<string, AMFObject> AMFMap;

		unsigned char type;
		union Value
		{
			bool b;
			string* str;
			double num;
			AMFMap* prop;
		} value;
		bool isNull() const
		{
			return type == AMF_NULL || type == AMF_UNDEFINED;
		}
		bool isMap() const
		{
			return type == AMF_BEGIN_OBJECT || type == AMF_MIXED_ARRAY;
		}
		AMFObject()
		{
			type = AMF_NULL;
			value.prop = nullptr;
		}
		AMFObject(AMFObject && move) {
			type = move.type;
			value = move.value;
			move.type = AMF_NULL;
		}
		AMFObject& operator = (AMFObject && move) {
			if (this == &move)return *this;
			type = move.type;
			value = move.value;
			move.type = AMF_NULL;
			return *this;
		}
		AMFObject(const AMFObject& copy) {
			type = copy.type;
			if (type == AMF_STRING) {
				value.str = new string(*copy.value.str);
			}
			else if (isMap()) {
				value.prop = new AMFMap();
				for (auto &i : *copy.value.prop) {
					(*value.prop)[i.first] = i.second;
				}
			}
			else {
				value = copy.value;
			}
		}
		AMFObject& operator = (const AMFObject& copy) {
			type = copy.type;
			if (type == AMF_STRING) {
				value.str = new string(*copy.value.str);
			}
			else if (isMap()) {
				value.prop = new AMFMap();
				for (auto &i : *copy.value.prop) {
					(*value.prop)[i.first] = i.second;
				}
			}
			else {
				value = copy.value;
			}
			return *this;
		}

		AMFObject(double number) {
			type = AMF_NUMBER;
			value.num = number;
		}
		AMFObject(const char* str) {
			type = AMF_STRING;
			value.str = new string(str);
		}
		AMFObject(string* str) {
			type = AMF_STRING;
			value.str = str;
		}
		AMFObject(bool b) {
			type = AMF_BOOLEAN;
			value.b = b;
		}

		AMFObject(AMFMap* map) {
			if(map)
			{
				type = AMF_BEGIN_OBJECT;
				value.prop = map;
			}else
			{
				type = AMF_NULL;
				value.prop = nullptr;
			}
		}
		void Write(DataWriter& writer) const
		{
			switch (type) {
			case AMF_NULL:
			case AMF_UNDEFINED:
				writer.packet.write8(type);
				break;
			case AMF_NUMBER:
				writer.packet.write8(type);
				writer.packet.write64(value.num);
				break;
			case AMF_BOOLEAN:
				writer.packet.write8(type);
				writer.packet.write8(value.b);
				break;
			case AMF_STRING:
				//writer.packet.write8(type);
				writer.writeString(value.str->c_str(), value.str->length());
				break;
			case AMF_MIXED_ARRAY:
				/*	result.write4BE(((*this)["length"]).value.num);
				this->amf0encObject(result);*/
				break;
			case AMF_BEGIN_OBJECT:
			{
				writer.beginObject(nullptr);
				for (auto& item : *value.prop) {
					writer.writePropertyName(item.first.c_str());
					item.second.Write(writer);
				}
				writer.endObject();
			}
				break;
			}
		}
	};
	struct AMFObjectWriter:DataWriter, virtual Object
	{
		AMFObject amfObject;
		AMFObject* currentObject;
		string currentPropertyName;
		map<AMFObject*, AMFObject*> parentObjects;
		AMFObjectWriter() :DataWriter(), amfObject(), currentObject(&amfObject), currentPropertyName("")
		{
			
		}
		AMFObjectWriter(const PoolBuffers& pool_buffers):DataWriter(pool_buffers),amfObject(),currentObject(&amfObject), currentPropertyName("")
		{
			
		}
		bool repeat(UInt64 reference) override {
			if (!currentPropertyName.empty())
			{
				AMFObject temp = *(AMFObject*)reference;
				currentObject->value.prop->emplace(currentPropertyName, temp);
				AMFObject* newObject = &currentObject->value.prop->operator[](currentPropertyName);
				parentObjects[newObject] = currentObject;
				currentPropertyName = nullptr;
			}
			else
			{
				AMFObject& temp = *(AMFObject*)reference;
				currentObject->type = temp.type;
				currentObject->value = temp.value;
			}
			return true;
		}
		void clear(UInt32 size = 0) override {

		}

		UInt64 beginObject(const char* type = nullptr) override {
			if(!currentPropertyName.empty())
			{
				currentObject->value.prop->emplace(currentPropertyName, new AMFObject::AMFMap());
				AMFObject* newObject = &currentObject->value.prop->operator[](currentPropertyName);
				parentObjects[newObject] = currentObject;
				currentObject = newObject;
			}else
			{
				currentObject->type = AMF_BEGIN_OBJECT;
				currentObject->value.prop = new AMFObject::AMFMap();
			}
			return (UInt64)currentObject;
		}
		void writePropertyName(const char* value) override {
			currentPropertyName = value;
		}
		void  endObject() override { 
			currentPropertyName.clear();
			if MAP_HAS1(parentObjects,currentObject)
			{
				currentObject = parentObjects[currentObject];
			}
		}

		UInt64 beginArray(UInt32 size) override {
			//todo 
			return (UInt64)currentObject;
		}
		void endArray() override{ endObject(); }

		UInt64 beginObjectArray(UInt32 size) override {
			//todo
			return (UInt64)currentObject;
		}

		UInt64 beginMap(Exception& ex, UInt32 size, bool weakKeys = false) override
		{
			//todo
			return (UInt64)currentObject;
		}
		void endMap() override{ endObject(); }

		void writeNumber(double value) override
		{
			if(!currentPropertyName.empty())
			{
				currentObject->value.prop->emplace(currentPropertyName, value);
			}else
			{
				currentObject->type = AMF_NUMBER;
				currentObject->value.num = value;
			}
		}
		void writeString(const char* value, UInt32 size) override
		{
			string* s = new string();
			s->resize(size + 1);
			memcpy((void*)s->c_str(), value, size);
			if(!currentPropertyName.empty())
			{
				currentObject->value.prop->emplace(currentPropertyName, s);
			}
			else{
				currentObject->type = AMF_STRING;
				currentObject->value.str = s;
			}
		}
		void writeBoolean(bool value) override {
			if (!currentPropertyName.empty())
			{
				currentObject->value.prop->emplace(currentPropertyName, value);
			}
			else
			{
				currentObject->type = AMF_BOOLEAN;
				currentObject->value.b = value;
			}
		}
		void writeNull() override {
			if (!currentPropertyName.empty())
			{
				currentObject->value.prop->emplace(currentPropertyName,AMFObject());
			}
			else
			{
				currentObject->type = AMF_NULL;
				currentObject->value.prop = nullptr;
			}
			
		}
		UInt64 writeDate(const Date& date) override {
			currentObject->type = AMF_DATE;
			currentObject->value.num = date.clock();
			return (UInt64)currentObject;
		}
		UInt64 writeBytes(const UInt8* data, UInt32 size) override {
			currentObject->type = AMF3_BYTEARRAY;
			currentObject->value.str = new string((const char*)data,size);
			return (UInt64)currentObject;
		}
	};
}
