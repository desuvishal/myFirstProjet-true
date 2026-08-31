#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>

#include "lib/nlohmann/json.hpp"

using json = nlohmann::json;

json decode_bencoded_value(const std::string& encoded_value, size_t& pos);

json decode_string(const std::string& encoded_value, size_t& pos) {
    size_t colon_index = encoded_value.find(':', pos);
    if (colon_index == std::string::npos) {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }
    std::string number_string = encoded_value.substr(pos, colon_index - pos);
    if (number_string.empty()) {
        throw std::runtime_error("Invalid byte length: " + encoded_value);
    }
    for (char ch : number_string) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            throw std::runtime_error("Invalid byte length: " + encoded_value);
        }
    }
    size_t length = static_cast<size_t>(std::stoll(number_string));
    if (colon_index + 1 + length > encoded_value.size()) {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }
    std::string output = encoded_value.substr(colon_index + 1, length);
    pos = colon_index + 1 + length;
    return json(output);
}

json decode_int(const std::string& encoded_value, size_t& pos) {
    size_t integer_end = encoded_value.find('e', pos);
    if (integer_end == std::string::npos) {
        throw std::runtime_error("Invalid Integer encoding: " + encoded_value);
    }
    std::string integer_string = encoded_value.substr(pos + 1, integer_end - pos - 1);
    if (integer_string.empty()) {
        throw std::runtime_error("Invalid Integer in encoded integer: " + encoded_value);
    }
    for (char ch : integer_string) {
        if (ch != '-' && !std::isdigit(static_cast<unsigned char>(ch))) {
            throw std::runtime_error("Invalid Integer in encoded integer: " + encoded_value);
        }
    }
    char* end = nullptr;
    long long integer = std::strtoll(integer_string.c_str(), &end, 10);
    if (end == integer_string.c_str() || *end != '\0') {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }
    pos = integer_end + 1;
    return json(integer);
}

json decode_list(const std::string& encoded_value, size_t& pos) {
    pos++; // skip 'l'
    json list = json::array();
    while (pos < encoded_value.size() && encoded_value[pos] != 'e') {
        list.push_back(decode_bencoded_value(encoded_value, pos));
    }
    if (pos >= encoded_value.size() || encoded_value[pos] != 'e') {
        throw std::runtime_error("Invalid Encoded value: " + encoded_value);
    }
    pos++; // skip 'e'
    return list;
}

json decode_dict(const std::string& encoded_value, size_t& pos){
    
}

json decode_bencoded_value(const std::string& encoded_value, size_t& pos) {
    if (pos >= encoded_value.size()) {
        throw std::runtime_error("Invalid encoded value: " + encoded_value);
    }

    if (std::isdigit(static_cast<unsigned char>(encoded_value[pos]))) {
        return decode_string(encoded_value, pos);
    }
    else if (encoded_value[pos] == 'i') {
        return decode_int(encoded_value, pos);
    }
    else if (encoded_value[pos] == 'l') {
        return decode_list(encoded_value, pos);
    }
    else if(encoded_value[pos] == 'd'){
        return decode_dict(encoded_value,pos);
    }
    else {
        throw std::runtime_error("Unhandled encoded value: " + encoded_value);
    }
}

json decode_bencode(const std::string& encoded_value) {
    size_t pos = 0;
    return decode_bencoded_value(encoded_value, pos);
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

        std::string encoded_value = argv[2];
        json decoded_value = decode_bencode(encoded_value);
        std::cout << decoded_value.dump() << std::endl;
    } else {
        std::cerr << "unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}