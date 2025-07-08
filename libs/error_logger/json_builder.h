#pragma once

#include <cstring>
#include <cstdio>
#include <cstdint>
#include <memory>
#include <inttypes.h>

enum JsonType {
    JT_NULL,
    JT_STRING,
    JT_INT,
    JT_BOOL,
    JT_OBJECT,
    JT_ARRAY,
    JT_HEX
};

class JsonObject;
class JsonArray;

class JsonValue {
    JsonType type;
    union {
        const char* str;
        int i;
        bool b;
        std::shared_ptr<JsonObject> obj;
        std::shared_ptr<JsonArray> arr;
        uintptr_t hex;
    };

public:
    JsonValue() : type(JT_NULL), i(0) {}
    JsonValue(const char* s) : type(JT_STRING), str(s) {}
    JsonValue(int v) : type(JT_INT), i(v) {}
    JsonValue(bool v) : type(JT_BOOL), b(v) {}
    JsonValue(std::shared_ptr<JsonObject> o) : type(JT_OBJECT), obj(std::move(o)) {}
    JsonValue(std::shared_ptr<JsonArray> a) : type(JT_ARRAY), arr(std::move(a)) {}
    JsonValue(uintptr_t v) : type(JT_HEX), hex(v) {}

    JsonValue(const JsonValue& other);
    JsonValue& operator=(const JsonValue& other);
    ~JsonValue();

    JsonType getType() const { return type; }
    const char* getString() const { return str; }
    int getInt() const { return i; }
    bool getBool() const { return b; }
    std::shared_ptr<JsonObject> getObject() const { return obj; }
    std::shared_ptr<JsonArray> getArray() const { return arr; }
    uintptr_t getHex() const { return hex; }
};

// Для корректного управления union с std::shared_ptr — реализуем конструктор копирования, оператор присваивания и деструктор:

JsonValue::JsonValue(const JsonValue& other) : type(other.type) {
    switch (type) {
    case JT_STRING:
        str = other.str;
        break;
    case JT_INT:
        i = other.i;
        break;
    case JT_BOOL:
        b = other.b;
        break;
    case JT_OBJECT:
        new (&obj) std::shared_ptr<JsonObject>(other.obj);
        break;
    case JT_ARRAY:
        new (&arr) std::shared_ptr<JsonArray>(other.arr);
        break;
    case JT_HEX:
        hex = other.hex;
        break;
    case JT_NULL:
        i = 0;
        break;
    }
}

JsonValue& JsonValue::operator=(const JsonValue& other) {
    if (this == &other) return *this;

    // Очистка предыдущих данных при необходимости
    if (type == JT_OBJECT) obj.~shared_ptr<JsonObject>();
    else if (type == JT_ARRAY) arr.~shared_ptr<JsonArray>();

    type = other.type;
    switch (type) {
    case JT_STRING:
        str = other.str;
        break;
    case JT_INT:
        i = other.i;
        break;
    case JT_BOOL:
        b = other.b;
        break;
    case JT_OBJECT:
        new (&obj) std::shared_ptr<JsonObject>(other.obj);
        break;
    case JT_ARRAY:
        new (&arr) std::shared_ptr<JsonArray>(other.arr);
        break;
    case JT_HEX:
        hex = other.hex;
        break;
    case JT_NULL:
        i = 0;
        break;
    }
    return *this;
}

JsonValue::~JsonValue() {
    if (type == JT_OBJECT) obj.~shared_ptr<JsonObject>();
    else if (type == JT_ARRAY) arr.~shared_ptr<JsonArray>();
}

class JsonObject {
    static const int MaxPairs = 16;
    const char* keys[MaxPairs]{};
    JsonValue values[MaxPairs]{};
    int count = 0;

public:
    bool insert(const char* key, const JsonValue& value) {
        if (count >= MaxPairs) return false;
        keys[count] = key;
        values[count++] = value;
        return true;
    }
    int size() const { return count; }
    const char* keyAt(int i) const { return keys[i]; }
    const JsonValue& valueAt(int i) const { return values[i]; }
    const JsonValue* find(const char* key) const {
        for (int i = 0; i < count; ++i)
            if (strcmp(keys[i], key) == 0)
                return &values[i];
        return nullptr;
    }
};

class JsonArray {
    static const int MaxElements = 16;
    JsonValue elements[MaxElements]{};
    int count = 0;

public:
    bool append(const JsonValue& val) {
        if (count >= MaxElements) return false;
        elements[count++] = val;
        return true;
    }
    int size() const { return count; }
    const JsonValue& at(int i) const { return elements[i]; }
};

class JsonWriter {
    char* buf;
    size_t capacity;
    size_t pos;

    bool writeRaw(const char* data, size_t len) {
        if (pos + len >= capacity) return false;
        memcpy(buf + pos, data, len);
        pos += len;
        return true;
    }

    bool writeString(const char* s) {
        if (!writeRaw("\"", 1)) return false;
        for (; *s; ++s) {
            switch (*s) {
            case '\"':
            case '\\':
                if (!writeRaw("\\", 1)) return false;
                if (!writeRaw(s, 1)) return false;
                break;
            case '\n':
                if (!writeRaw("\\n", 2)) return false;
                break;
            case '\r':
                if (!writeRaw("\\r", 2)) return false;
                break;
            case '\t':
                if (!writeRaw("\\t", 2)) return false;
                break;
            default:
                if (!writeRaw(s, 1)) return false;
            }
        }
        return writeRaw("\"", 1);
    }

    bool writeValue(const JsonValue& val) {
        switch (val.getType()) {
        case JT_STRING:
            return writeString(val.getString());
        case JT_INT: {
            char numbuf[20];
            int len = snprintf(numbuf, sizeof(numbuf), "%d", val.getInt());
            return writeRaw(numbuf, len);
        }
        case JT_BOOL:
            return writeRaw(val.getBool() ? "true" : "false", val.getBool() ? 4 : 5);
        case JT_OBJECT:
            return writeObject(*val.getObject());
        case JT_ARRAY:
            return writeArray(*val.getArray());
        case JT_NULL:
            return writeRaw("null", 4);
        case JT_HEX: {
            char buf[32];
            int len = snprintf(buf, sizeof(buf), "\"0x%" PRIXPTR "\"", val.getHex());
            return writeRaw(buf, len);
        }
        }
        return false;
    }

    bool writeObject(const JsonObject& obj) {
        if (!writeRaw("{", 1)) return false;
        for (int i = 0; i < obj.size(); ++i) {
            if (i > 0 && !writeRaw(",", 1)) return false;
            if (!writeString(obj.keyAt(i))) return false;
            if (!writeRaw(":", 1)) return false;
            if (!writeValue(obj.valueAt(i))) return false;
        }
        return writeRaw("}", 1);
    }

    bool writeArray(const JsonArray& arr) {
        if (!writeRaw("[", 1)) return false;
        for (int i = 0; i < arr.size(); ++i) {
            if (i > 0 && !writeRaw(",", 1)) return false;
            if (!writeValue(arr.at(i))) return false;
        }
        return writeRaw("]", 1);
    }

public:
    JsonWriter(char* buffer, size_t size) : buf(buffer), capacity(size), pos(0) {}

    bool write(const JsonValue& val) {
        pos = 0;
        return writeValue(val);
    }

    const char* data() const { return buf; }
    size_t length() const { return pos; }
};
