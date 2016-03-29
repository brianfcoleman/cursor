#include "gap-buffer.hh"
#include "range.hh"

#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>

namespace cursor {
namespace test {
namespace gap_buffer {
void validate_gap_buffer_content(const GapBuffer<char>& gap_buffer, const std::string& expected_content) {
    std::string gap_buffer_content;
    std::copy(gap_buffer.cbegin(), gap_buffer.cend(), std::back_inserter(gap_buffer_content));
    ASSERT_EQ(expected_content, gap_buffer_content);
}

void insert_before_position() {
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.insert(make_crange(content), 0);
    validate_gap_buffer_content(gap_buffer, content);
}

void insert_before_iterator() {
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.insert(make_crange(content), gap_buffer.cbegin());
    validate_gap_buffer_content(gap_buffer, content);
}

void append() {
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.append(make_crange(content));
    validate_gap_buffer_content(gap_buffer, content);
}

void remove_at_position() {
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World";
    gap_buffer.append(content);
    validate_gap_buffer_content(gap_buffer, content);
    gap_buffer.remove(0, content.size());
    content = "";
    validate_gap_buffer_content(gap_buffer, content);
}

void remove_at_iterator() {
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.append(content);
    validate_gap_buffer_content(gap_buffer, content);
    gap_buffer.remove(make_crange(gap_buffer));
    content = "";
    validate_gap_buffer_content(gap_buffer, content);
}

void replace_at_position() {
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.append(content);
    validate_gap_buffer_content(gap_buffer, content);
    std::string new_content = "Goodbye World!";
    gap_buffer.replace(0, content.size(), make_crange(new_content));
    validate_gap_buffer_content(gap_buffer, new_content);
}

void replace_at_iterator() {
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.append(content);
    validate_gap_buffer_content(gap_buffer, content);
    std::string new_content = "Goodbye World!";
    gap_buffer.replace(make_crange(gap_buffer), make_crange(new_content));
    validate_gap_buffer_content(gap_buffer, new_content);
}

void size() {
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.append(content);
    validate_gap_buffer_content(gap_buffer, content);
    ASSERT_EQ(content.size(), gap_buffer.size());
}
}
}
}

TEST(gap_buffer, insert_before_position) {
    cursor::test::gap_buffer::insert_before_position();
}

TEST(gap_buffer, insert_before_iterator) {
    cursor::test::gap_buffer::insert_before_iterator();
}

TEST(gap_buffer, append) {
    cursor::test::gap_buffer::append();
}

TEST(gap_buffer, remove_at_position) {
    cursor::test::gap_buffer::remove_at_position();
}

TEST(gap_buffer, remove_at_iterator) {
    cursor::test::gap_buffer::remove_at_iterator();
}

TEST(gap_buffer, replace_at_position) {
    cursor::test::gap_buffer::replace_at_position();
}

TEST(gap_buffer, replace_at_iterator) {
    cursor::test::gap_buffer::replace_at_iterator();
}

TEST(gap_buffer, size) {
    cursor::test::gap_buffer::size();
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
