#ifndef DHARTI_UTILS_JSON_HPP
#define DHARTI_UTILS_JSON_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cctype>
#include <iostream>

namespace dharti {
namespace utils {

enum class JsonType {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
};

class JsonValue {
public:
    JsonType type{JsonType::Null};
    bool bool_val{false};
    double num_val{0.0};
    std::string str_val;
    std::vector<JsonValue> arr_val;
    std::map<std::string, JsonValue> obj_val;

    JsonValue() = default;
    JsonValue(bool b) : type(JsonType::Boolean), bool_val(b) {}
    JsonValue(double n) : type(JsonType::Number), num_val(n) {}
    JsonValue(int n) : type(JsonType::Number), num_val(static_cast<double>(n)) {}
    JsonValue(int64_t n) : type(JsonType::Number), num_val(static_cast<double>(n)) {}
    JsonValue(const std::string& s) : type(JsonType::String), str_val(s) {}
    JsonValue(const char* s) : type(JsonType::String), str_val(s ? s : "") {}

    bool is_null() const { return type == JsonType::Null; }
    bool is_bool() const { return type == JsonType::Boolean; }
    bool is_number() const { return type == JsonType::Number; }
    bool is_string() const { return type == JsonType::String; }
    bool is_array() const { return type == JsonType::Array; }
    bool is_object() const { return type == JsonType::Object; }

    std::string as_string(const std::string& default_val = "") const {
        return is_string() ? str_val : default_val;
    }

    double as_double(double default_val = 0.0) const {
        return is_number() ? num_val : default_val;
    }

    int64_t as_int(int64_t default_val = 0) const {
        return is_number() ? static_cast<int64_t>(num_val) : default_val;
    }

    bool as_bool(bool default_val = false) const {
        return is_bool() ? bool_val : default_val;
    }

    bool has_key(const std::string& key) const {
        if (!is_object()) return false;
        return obj_val.find(key) != obj_val.end();
    }

    const JsonValue& operator[](const std::string& key) const {
        static const JsonValue null_val;
        if (!is_object()) return null_val;
        auto it = obj_val.find(key);
        return it != obj_val.end() ? it->second : null_val;
    }

    JsonValue& operator[](const std::string& key) {
        if (!is_object()) {
            type = JsonType::Object;
            obj_val.clear();
        }
        return obj_val[key];
    }

    const JsonValue& operator[](size_t index) const {
        static const JsonValue null_val;
        if (!is_array() || index >= arr_val.size()) return null_val;
        return arr_val[index];
    }

    size_t size() const {
        if (is_array()) return arr_val.size();
        if (is_object()) return obj_val.size();
        return 0;
    }

    static JsonValue parse(const std::string& json_str) {
        size_t idx = 0;
        skip_whitespace(json_str, idx);
        return parse_value(json_str, idx);
    }

    static JsonValue parse_file(const std::string& filepath) {
        std::ifstream f(filepath);
        if (!f.is_open()) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }
        std::stringstream ss;
        ss << f.rdbuf();
        return parse(ss.str());
    }

    std::string dump() const {
        std::stringstream ss;
        serialize(ss);
        return ss.str();
    }

private:
    void serialize(std::stringstream& ss) const {
        switch (type) {
            case JsonType::Null: ss << "null"; break;
            case JsonType::Boolean: ss << (bool_val ? "true" : "false"); break;
            case JsonType::Number: ss << num_val; break;
            case JsonType::String: {
                ss << "\"";
                for (char c : str_val) {
                    if (c == '"') ss << "\\\"";
                    else if (c == '\\') ss << "\\\\";
                    else if (c == '\n') ss << "\\n";
                    else if (c == '\r') ss << "\\r";
                    else if (c == '\t') ss << "\\t";
                    else ss << c;
                }
                ss << "\"";
                break;
            }
            case JsonType::Array: {
                ss << "[";
                for (size_t i = 0; i < arr_val.size(); ++i) {
                    if (i > 0) ss << ",";
                    arr_val[i].serialize(ss);
                }
                ss << "]";
                break;
            }
            case JsonType::Object: {
                ss << "{";
                bool first = true;
                for (const auto& pair : obj_val) {
                    if (!first) ss << ",";
                    first = false;
                    ss << "\"" << pair.first << "\":";
                    pair.second.serialize(ss);
                }
                ss << "}";
                break;
            }
        }
    }

    static void skip_whitespace(const std::string& s, size_t& idx) {
        while (idx < s.size() && (s[idx] == ' ' || s[idx] == '\t' || s[idx] == '\n' || s[idx] == '\r')) {
            idx++;
        }
    }

    static JsonValue parse_value(const std::string& s, size_t& idx) {
        skip_whitespace(s, idx);
        if (idx >= s.size()) return JsonValue();

        char c = s[idx];
        if (c == '{') return parse_object(s, idx);
        if (c == '[') return parse_array(s, idx);
        if (c == '"') return parse_string(s, idx);
        if (c == 't' || c == 'f') return parse_bool(s, idx);
        if (c == 'n') return parse_null(s, idx);
        if (c == '-' || std::isdigit(c)) return parse_number(s, idx);

        throw std::runtime_error(std::string("Unexpected character: ") + c);
    }

    static JsonValue parse_object(const std::string& s, size_t& idx) {
        JsonValue val;
        val.type = JsonType::Object;
        idx++; // Skip '{'

        while (idx < s.size()) {
            skip_whitespace(s, idx);
            if (idx < s.size() && s[idx] == '}') {
                idx++;
                return val;
            }

            JsonValue key_val = parse_string(s, idx);
            std::string key = key_val.as_string();

            skip_whitespace(s, idx);
            if (idx >= s.size() || s[idx] != ':') {
                throw std::runtime_error("Expected ':' after key in object");
            }
            idx++; // Skip ':'

            JsonValue sub_val = parse_value(s, idx);
            val.obj_val[key] = sub_val;

            skip_whitespace(s, idx);
            if (idx < s.size() && s[idx] == ',') {
                idx++;
            } else if (idx < s.size() && s[idx] == '}') {
                idx++;
                return val;
            }
        }
        throw std::runtime_error("Unterminated object");
    }

    static JsonValue parse_array(const std::string& s, size_t& idx) {
        JsonValue val;
        val.type = JsonType::Array;
        idx++; // Skip '['

        while (idx < s.size()) {
            skip_whitespace(s, idx);
            if (idx < s.size() && s[idx] == ']') {
                idx++;
                return val;
            }

            JsonValue sub_val = parse_value(s, idx);
            val.arr_val.push_back(sub_val);

            skip_whitespace(s, idx);
            if (idx < s.size() && s[idx] == ',') {
                idx++;
            } else if (idx < s.size() && s[idx] == ']') {
                idx++;
                return val;
            }
        }
        throw std::runtime_error("Unterminated array");
    }

    static JsonValue parse_string(const std::string& s, size_t& idx) {
        if (s[idx] != '"') throw std::runtime_error("Expected '\"'");
        idx++; // Skip opening quote
        std::string res;
        while (idx < s.size()) {
            char c = s[idx++];
            if (c == '"') {
                return JsonValue(res);
            }
            if (c == '\\' && idx < s.size()) {
                char esc = s[idx++];
                if (esc == '"') res += '"';
                else if (esc == '\\') res += '\\';
                else if (esc == 'n') res += '\n';
                else if (esc == 'r') res += '\r';
                else if (esc == 't') res += '\t';
                else res += esc;
            } else {
                res += c;
            }
        }
        throw std::runtime_error("Unterminated string");
    }

    static JsonValue parse_number(const std::string& s, size_t& idx) {
        size_t start = idx;
        if (s[idx] == '-') idx++;
        while (idx < s.size() && std::isdigit(s[idx])) idx++;
        if (idx < s.size() && s[idx] == '.') {
            idx++;
            while (idx < s.size() && std::isdigit(s[idx])) idx++;
        }
        if (idx < s.size() && (s[idx] == 'e' || s[idx] == 'E')) {
            idx++;
            if (idx < s.size() && (s[idx] == '+' || s[idx] == '-')) idx++;
            while (idx < s.size() && std::isdigit(s[idx])) idx++;
        }
        std::string sub = s.substr(start, idx - start);
        return JsonValue(std::stod(sub));
    }

    static JsonValue parse_bool(const std::string& s, size_t& idx) {
        if (s.compare(idx, 4, "true") == 0) {
            idx += 4;
            return JsonValue(true);
        }
        if (s.compare(idx, 5, "false") == 0) {
            idx += 5;
            return JsonValue(false);
        }
        throw std::runtime_error("Invalid boolean literal");
    }

    static JsonValue parse_null(const std::string& s, size_t& idx) {
        if (s.compare(idx, 4, "null") == 0) {
            idx += 4;
            return JsonValue();
        }
        throw std::runtime_error("Invalid null literal");
    }
};

} // namespace utils
} // namespace dharti

#endif // DHARTI_UTILS_JSON_HPP
