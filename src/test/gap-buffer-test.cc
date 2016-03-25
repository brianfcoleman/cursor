#include "gap-buffer.hh"
#include "range.hh"

#include <iostream>
#include <iterator>
#include <string>

namespace {
using cursor::GapBuffer;
std::string to_string(const GapBuffer<char>& gap_buffer) {
    std::string message;
    for (const auto c : gap_buffer) {
        message.push_back(c);
    }
    return message;
}
}

int main(int argc, char* argv[]) {
    using namespace cursor;
    GapBuffer<char> gap_buffer;
    std::cout << to_string(gap_buffer) << std::endl;
    std::string message = "world";
    gap_buffer.append(make_range(message.begin(), message.end()));
    std::cout << to_string(gap_buffer) << std::endl;
    message = "hello";
    gap_buffer.insert(make_range(message.begin(), message.end()), 0);
    std::cout << to_string(gap_buffer) << std::endl;
    auto element = gap_buffer.cbegin();
    std::advance(element, message.size());
    message = " ";
    gap_buffer.insert(make_range(message.begin(), message.end()), element);
    std::cout << to_string(gap_buffer) << std::endl;
}
