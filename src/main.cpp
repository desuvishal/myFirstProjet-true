#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>

#include "lib/nlohmann/json.hpp"

using json = nlohmann::json;

json decode_string(const std::string& encoded_value){
    size_t colon_index = encoded_value.find(':');
    if (colon_index == std::string::npos) {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }

    std::string number_string = encoded_value.substr(0, colon_index);
    if (number_string.empty()) {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }

    for (char ch : number_string) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            throw std::runtime_error("Invalid encoded value: " + encoded_value);
        }
    }

    size_t length = static_cast<size_t>(std::stoll(number_string));
    if (colon_index + 1 + length != encoded_value.size()) {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }

    return json(encoded_value.substr(colon_index + 1, length));
}

json decode_int(const std::string& encoded_value){
    if (encoded_value.size() < 3 || encoded_value[0] != 'i' || encoded_value.back() != 'e') {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }

    std::string number_string = encoded_value.substr(1, encoded_value.size() - 2);
    if (number_string.empty()) {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }

    for (char ch : number_string) {
        if (ch != '-' && !std::isdigit(static_cast<unsigned char>(ch))) {
            throw std::runtime_error("Invalid encoded value: " + encoded_value);
        }
    }

    char* end = nullptr;
    long long value = std::strtoll(number_string.c_str(), &end, 10);
    if (end == number_string.c_str() || *end != '\0') {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }

    return json(value);
}

json decode_list(const std::string& encoded_value){
    if (encoded_value.size() < 2 || encoded_value[0] != 'l' || encoded_value.back() != 'e') {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }

    size_t index = 1;
    json list = json::array();

    while (index < encoded_value.size() && encoded_value[index] != 'e') {
        if (encoded_value[index] == 'i') {
            size_t end = encoded_value.find('e', index);
            if (end == std::string::npos) {
                throw std::runtime_error("Invalid encoded value: " + encoded_value);
            }
            std::string token = encoded_value.substr(index, end - index + 1);
            list.push_back(decode_int(token));
            index = end + 1;
        }
        else if (std::isdigit(static_cast<unsigned char>(encoded_value[index]))) {
            size_t colon_index = encoded_value.find(':', index);
            if (colon_index == std::string::npos) {
                throw std::runtime_error("Invalid encoded value: " + encoded_value);
            }

            std::string number_string = encoded_value.substr(index, colon_index - index);
            for (char ch : number_string) {
                if (!std::isdigit(static_cast<unsigned char>(ch))) {
                    throw std::runtime_error("Invalid encoded value: " + encoded_value);
                }
            }

            size_t length = static_cast<size_t>(std::stoll(number_string));
            size_t token_length = (colon_index - index) + 1 + length;
            if (index + token_length > encoded_value.size()) {
                throw std::runtime_error("Invalid encoded value: " + encoded_value);
            }

            std::string token = encoded_value.substr(index, token_length);
            list.push_back(decode_string(token));
            index += token_length;
        }
        else if (encoded_value[index] == 'l') {
            size_t nested_start = index;
            size_t depth = 1;
            ++index;

            while (index < encoded_value.size() && depth > 0) {
                if (encoded_value[index] == 'l') {
                    ++depth;
                }
                else if (encoded_value[index] == 'e') {
                    --depth;
                }
                ++index;
            }

            if (depth != 0) {
                throw std::runtime_error("Invalid encoded value: " + encoded_value);
            }

            std::string token = encoded_value.substr(nested_start, index - nested_start);
            list.push_back(decode_list(token));
        }
        else {
            throw std::runtime_error("Invalid encoded value: " + encoded_value);
        }
    }

    if (index != encoded_value.size() - 1 || encoded_value[index] != 'e') {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }

    return list;
}

json decode_bencoded_value(const std::string& encoded_value) {
    if (encoded_value.empty()) {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }

    if (std::isdigit(static_cast<unsigned char>(encoded_value[0]))) {
        return decode_string(encoded_value);
    }
    else if (encoded_value[0] == 'i') {
        return decode_int(encoded_value);
    }
    else if (encoded_value[0] == 'l') {
        return decode_list(encoded_value);
    }
    else {
        throw std::runtime_error("Unhandled encoded value: " + encoded_value);
    }
}

int main(int argc, char* argv[]) {

    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
        return 1;
    }

    std::string command = argv[1];

    if (command == "decode") {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " decode <encoded_value>" << std::endl;
            return 1;
        }
        // You can use print statements as follows for debugging, they'll be visible when running tests.
        std::cerr << "Logs from your program will appear here!" << std::endl;

        // TODO: Uncomment the code below to pass the first stage
        std::string encoded_value = argv[2];
        json decoded_value = decode_bencoded_value(encoded_value);
        std::cout << decoded_value.dump() << std::endl;
    } else {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}
