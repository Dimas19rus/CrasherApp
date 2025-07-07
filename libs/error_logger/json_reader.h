#pragma once

#include <cctype>
#include <cstring>
#include <cstdlib>
#include <string>

#include "json_builder.h"

class JsonReader {
    const char* ptr;

    void skipWhitespace() {
        while (*ptr && isspace(*ptr)) ++ptr;
    }

    bool match(const char* kw) {
        size_t len = strlen(kw);
        if (strncmp(ptr, kw, len) == 0) {
            ptr += len;
            return true;
        }
        return false;
    }

    bool parseString(char* out, size_t outSize) {
        if (*ptr != '"') return false;
        ++ptr;
        size_t pos = 0;
        while (*ptr && *ptr != '"' && pos + 1 < outSize) {
            if (*ptr == '\\') {
                ++ptr;
                if (!*ptr) break;
                // Простая поддержка экранирования (\", \\, \n, \t)
                switch (*ptr) {
                    case '"':  out[pos++] = '"';  break;
                    case '\\': out[pos++] = '\\'; break;
                    case 'n':  out[pos++] = '\n'; break;
                    case 't':  out[pos++] = '\t'; break;
                    // Можно добавить другие escape-последовательности
                    default:
                        out[pos++] = *ptr;
                        break;
                }
                ++ptr;
            } else {
                out[pos++] = *ptr++;
            }
        }
        if (*ptr != '"') return false;
        ++ptr;
        out[pos] = '\0';
        return true;
    }

    bool parseNumber(int& out) {
        char buf[32];
        size_t len = 0;
        if (*ptr == '-') {
            buf[len++] = *ptr++;
        }
        while (*ptr && isdigit(*ptr)) {
            if (len < sizeof(buf) - 1)
                buf[len++] = *ptr++;
            else
                return false;
        }
        buf[len] = '\0';
        if (len == 0 || (len == 1 && buf[0] == '-')) return false;
        out = atoi(buf);
        return true;
    }

    bool parseObject(JsonObject* obj, char* strBuf, size_t strBufSize) {
        if (*ptr != '{') return false;
        ++ptr;
        skipWhitespace();

        while (*ptr && *ptr != '}') {
            skipWhitespace();
            if (*ptr != '"') return false;
            if (!parseString(strBuf, strBufSize)) return false;
            std::string key(strBuf);

            skipWhitespace();
            if (*ptr != ':') return false;
            ++ptr;
            skipWhitespace();

            JsonValue val;
            if (!parseValue(val, strBuf, strBufSize)) return false;

            if (!obj->insert(key.c_str(), val)) return false;

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

    bool parseArray(JsonArray* arr, char* strBuf, size_t strBufSize) {
        if (*ptr != '[') return false;
        ++ptr;
        skipWhitespace();

        while (*ptr && *ptr != ']') {
            JsonValue val;
            if (!parseValue(val, strBuf, strBufSize)) return false;

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

    bool parseValue(JsonValue& out, char* strBuf, size_t strBufSize) {
        skipWhitespace();
        if (*ptr == '"') {
            if (!parseString(strBuf, strBufSize)) return false;
            out = JsonValue(strBuf);
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
            JsonObject obj;
            if (!parseObject(&obj, strBuf, strBufSize)) return false;
            out = JsonValue(&obj);
            return true;
        } else if (*ptr == '[') {
            JsonArray arr;
            if (!parseArray(&arr, strBuf, strBufSize)) return false;
            out = JsonValue(&arr);
            return true;
        }
        return false;
    }

public:
    explicit JsonReader(const char* jsonText) : ptr(jsonText) {}

    bool parse(JsonValue& rootVal) {
        char tmpStr[128]; // буфер для строк
        if (!parseValue(rootVal, tmpStr, sizeof(tmpStr))) return false;
        skipWhitespace();
        return *ptr == '\0'; // проверка конца входных данных
    }
};
