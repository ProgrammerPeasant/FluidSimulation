#ifndef FLUIDFINALVER_TYPES_H
#define FLUIDFINALVER_TYPES_H

#include <unordered_map>
#include <charconv>

#include "originalFunctions.h"

//#define TYPES FLOAT,DOUBLE,FIXED(32, 9),FAST_FIXED(40, 5),FAST_FIXED(50, 16)

#define FLOAT 1
#define DOUBLE 2
#define FIXED(n, k) ((n) * 1000 + (k))
#define FAST_FIXED(n, k) ((n) * 100000 + (k))
#define PAIR(n, k) std::pair<int, int>((n), (k))
#define S(n, m) std::pair<int, int>((n), (m))
//#define SIZES S(35, 286),S(36, 84)


#ifndef TYPES
#define TYPES FLOAT, DOUBLE, FIXED(32, 9), FAST_FIXED(40, 5), FAST_FIXED(50, 16)
#endif

#ifndef SIZES
#define SIZES S(-1, -1)
#endif


enum class EType {
    FIXED,
    F_FIXED,
    FL,
    DB,
};

class OptionsParser {
public:
    OptionsParser(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            std::string_view line(argv[i]);
            auto eqPos = line.find('=');
            if (eqPos == std::string_view::npos) {
                throw std::invalid_argument("Invalid option '" + std::string(line) + "'");
            }
            auto key = line.substr(0, eqPos);
            auto val = line.substr(eqPos + 1);
            opts_.emplace(std::string(key), std::string(val));
        }
    }

    std::string getOptVal(const std::string& opt) const {
        auto it = opts_.find(opt);
        if (it == opts_.end()) {
            throw std::invalid_argument("No such option '" + opt + "'");
        }
        return it->second;
    }

    int getOptValAsInt(const std::string& opt) const {
        std::string val = getOptVal(opt);
        int result;
        auto [ptr, ec] = std::from_chars(val.data(), val.data() + val.size(), result);
        if (ec != std::errc()) {
            throw std::invalid_argument("Option '" + opt + "' value is not a valid integer");
        }
        return result;
    }

    std::pair<int, int> getOptValAsPair(const std::string& opt) const {
        std::string val = getOptVal(opt);
        size_t commaPos = val.find(',');
        if (commaPos == std::string::npos) {
            throw std::invalid_argument("Option '" + opt + "' value is not a valid pair");
        }
        int first = 0, second = 0;
        auto [ptr1, ec1] = std::from_chars(val.data(), val.data() + commaPos, first);
        if (ec1 != std::errc()) {
            throw std::invalid_argument("First part of option '" + opt + "' is not a valid integer");
        }
        auto [ptr2, ec2] = std::from_chars(val.data() + commaPos + 1, val.data() + val.size(), second);
        if (ec2 != std::errc()) {
            throw std::invalid_argument("Second part of option '" + opt + "' is not a valid integer");
        }
        return {first, second};
    }

private:
    std::unordered_map<std::string, std::string> opts_;
};


inline int GetTypeCode(std::string_view typeName) {
    if (typeName.substr(0, 6) == "FIXED(" && typeName.back() == ')') {
        int n = 0, k = 0;
        auto inner = typeName.substr(6, typeName.size() - 7);
        size_t commaPos = inner.find(',');
        if (commaPos == std::string::npos) {
            throw std::invalid_argument("Invalid FIXED type format: " + std::string(typeName));
        }
        auto [ptr1, ec1] = std::from_chars(inner.data(), inner.data() + commaPos, n);
        if (ec1 != std::errc()) {
            throw std::invalid_argument("Invalid number in FIXED type: " + std::string(typeName));
        }
        auto [ptr2, ec2] = std::from_chars(inner.data() + commaPos + 1, inner.data() + inner.size(), k);
        if (ec2 != std::errc()) {
            throw std::invalid_argument("Invalid number in FIXED type: " + std::string(typeName));
        }
        return FIXED(n, k);
    }

    if (typeName.substr(0, 11) == "FAST_FIXED(" && typeName.back() == ')') {
        int n = 0, k = 0;
        auto inner = typeName.substr(11, typeName.size() - 12);
        size_t commaPos = inner.find(',');
        if (commaPos == std::string::npos) {
            throw std::invalid_argument("Invalid FAST_FIXED type format: " + std::string(typeName));
        }
        auto [ptr1, ec1] = std::from_chars(inner.data(), inner.data() + commaPos, n);
        if (ec1 != std::errc()) {
            throw std::invalid_argument("Invalid number in FAST_FIXED type: " + std::string(typeName));
        }
        auto [ptr2, ec2] = std::from_chars(inner.data() + commaPos + 1, inner.data() + inner.size(), k);
        if (ec2 != std::errc()) {
            throw std::invalid_argument("Invalid number in FAST_FIXED type: " + std::string(typeName));
        }
        return FAST_FIXED(n, k);
    }

    if (typeName == "DOUBLE") {
        return DOUBLE;
    }
    if (typeName == "FLOAT") {
        return FLOAT;
    }

    throw std::invalid_argument("Unknown type '" + std::string(typeName) + "'");
}

#endif //FLUIDFINALVER_TYPES_H
