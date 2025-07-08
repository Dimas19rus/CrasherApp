#pragma once

#include <cctype>
#include <cstring>
#include <cstdlib>
#include <string>
#include <memory>

#include "json_builder.h"

class JsonReader {
    const char* ptr;

    void skipWhitespace() {
        while (*ptr && isspace(*ptr)) ++ptr;
    }

    bool match(const char* kw) {
        size_t len = strlen(kw);
        if (strncmp(ptr, kw, len) == 0 && !isalnum(ptr[len])) {
            ptr += len;
            return true;
        }
        return false;
    }

    bool parseString(std::string& out) {
        if (*ptr != '"') return false;
        ++ptr;
        out.clear();

        while (*ptr && *ptr != '"') {
            if (*ptr == '\\') {
                ++ptr;
                if (!*ptr) return false;
                switch (*ptr) {
                    case '"':  out.push_back('"');  break;
                    case '\\': out.push_back('\\'); break;
                    case 'n':  out.push_back('\n'); break;
                    case 't':  out.push_back('\t'); break;
                    // Добавьте другие escape-последовательности при необходимости
                    default:
                        out.push_back(*ptr);
                        break;
                }
                ++ptr;
            } else {
                out.push_back(*ptr++);
            }
        }
        if (*ptr != '"') return false;
        ++ptr;
        return true;
    }

    bool parseNumber(int& out) {
        const char* start = ptr;
        if (*ptr == '-') ++ptr;
        if (!isdigit(*ptr)) return false;
        while (isdigit(*ptr)) ++ptr;

        std::string numStr(start, ptr - start);
        try {
            out = std::stoi(numStr);
        } catch (...) {
            return false;
        }
        return true;
    }

    bool parseObject(std::shared_ptr<JsonObject>& obj, std::string& strBuf) {
        if (*ptr != '{') return false;
        ++ptr;
        skipWhitespace();

        obj = std::make_shared<JsonObject>();

        while (*ptr && *ptr != '}') {
            skipWhitespace();
            if (*ptr != '"') return false;

            std::string key;
            if (!parseString(key)) return false;

            skipWhitespace();
            if (*ptr != ':') return false;
            ++ptr;
            skipWhitespace();

            JsonValue val;
            if (!parseValue(val, strBuf)) return false;

            // Копируем строку ключа в отдельный буфер (JsonObject лучше хранит std::string)
            // Примерно так, чтобы ключ был валиден:
            char* key_cstr = new char[key.size() + 1];
            strcpy(key_cstr, key.c_str());

            if (!obj->insert(key_cstr, val)) {
                delete[] key_cstr;
                return false;
            }

            skipWhitespace();
            if (*ptr == ',') {
                ++ptr;
                skipWhitespace();
            } else if (*ptr != '}') {
                return false;
            }
        }
        if (*ptr != '}') return false;
        ++ptr;
        return true;
    }

    bool parseArray(std::shared_ptr<JsonArray>& arr, std::string& strBuf) {
        if (*ptr != '[') return false;
        ++ptr;
        skipWhitespace();

        arr = std::make_shared<JsonArray>();

        while (*ptr && *ptr != ']') {
            JsonValue val;
            if (!parseValue(val, strBuf)) return false;

            if (!arr->append(val)) return false;

            skipWhitespace();
            if (*ptr == ',') {
                ++ptr;
                skipWhitespace();
            } else if (*ptr != ']') {
                return false;
            }
        }
        if (*ptr != ']') return false;
        ++ptr;
        return true;
    }

    bool parseValue(JsonValue& out, std::string& strBuf) {
        skipWhitespace();
        if (*ptr == '"') {
            if (!parseString(strBuf)) return false;
            // Здесь нужно хранить строку отдельно, чтобы lifetime не сломался
            // Можно копировать в heap (JsonValue конструктор с const char* хранит указатель)
            char* str_copy = new char[strBuf.size() + 1];
            strcpy(str_copy, strBuf.c_str());
            out = JsonValue(str_copy);
            return true;
        } else if (isdigit(*ptr) || *ptr == '-') {
            int val;
            if (!parseNumber(val)) return false;
            out = JsonValue(val);
            return true;
        } else if (match("true")) {
            out = JsonValue(true);
            return true;
        } else if (match("false")) {
            out = JsonValue(false);
            return true;
        } else if (match("null")) {
            out = JsonValue();
            return true;
        } else if (*ptr == '{') {
            std::shared_ptr<JsonObject> obj;
            if (!parseObject(obj, strBuf)) return false;
            out = JsonValue(std::move(obj));
            return true;
        } else if (*ptr == '[') {
            std::shared_ptr<JsonArray> arr;
            if (!parseArray(arr, strBuf)) return false;
            out = JsonValue(std::move(arr));
            return true;
        }
        return false;
    }

    JsonValue parseJson(const char* data) {
        ptr = data;
        std::string strBuf;
        JsonValue result;
        if (!parseValue(result, strBuf)) {
            return JsonValue();
        }
        return result;
    }

public:
    explicit JsonReader(const char* jsonText) : ptr(jsonText) {}

    bool parse(JsonValue& rootVal) {
        std::string tmpStr;
        if (!parseValue(rootVal, tmpStr)) return false;
        skipWhitespace();
        return *ptr == '\0'; // проверка конца
    }
};
