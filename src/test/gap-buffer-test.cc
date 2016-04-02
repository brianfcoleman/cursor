#include "gap-buffer.hh"
#include "range.hh"

#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <random>
#include <stdexcept>

namespace cursor {
namespace test {
namespace gap_buffer {
namespace {
std::string alphabet() {
    return "abcdefghijklmnopqrstuvwxyz";
}

template<typename RandomEngine>
auto make_random_int_generator(RandomEngine& random_engine, int min, int max) {
    std::uniform_int_distribution<> distribution{min, max};
    return [distribution, &random_engine]() mutable -> int {
        return distribution(random_engine);
    };
}

template<typename RandomIntGenerator>
auto make_random_letter_generator(RandomIntGenerator&& generate_random_int, const std::string& alphabet) {
    return [generate_random_int = std::move(generate_random_int), alphabet]() mutable -> char {
        return alphabet.at(generate_random_int());
    };
}

template<typename RandomEngine>
auto make_random_word_generator(RandomEngine& random_engine, const std::string& alphabet, int word_size) {
    auto generate_random_int = make_random_int_generator(random_engine, 0, alphabet.size() - 1);
    auto generate_random_letter = make_random_letter_generator(generate_random_int, alphabet);
    return [generate_random_letter = std::move(generate_random_letter), word_size]() -> std::string {
        std::string random_word;
        random_word.resize(word_size);
        std::generate(random_word.begin(), random_word.end(), generate_random_letter);
        return random_word;
    };
}

template<typename RandomEngine>
auto make_random_word_generators(RandomEngine& random_engine, const std::string& alphabet, int min_word_size, int max_word_size) {
    using RandomWordGenerator = decltype(make_random_word_generator(random_engine, alphabet, min_word_size));
    std::vector<RandomWordGenerator> random_word_generators;
    for (int word_size = min_word_size; word_size <= max_word_size; ++word_size) {
        random_word_generators.push_back(make_random_word_generator(random_engine, alphabet, word_size));
    }
    return [random_word_generators = std::move(random_word_generators)](int word_size) -> std::string {
        auto generate_random_word = random_word_generators.at(word_size);
        return generate_random_word();
    };
}

auto to_string(const GapBuffer<char>& gap_buffer) {
    std::string gap_buffer_content;
    gap_buffer_content.reserve(gap_buffer_content.size());
    std::copy(gap_buffer.cbegin(), gap_buffer.cend(), std::back_inserter(gap_buffer_content));
    return gap_buffer_content;
}

auto to_string(const std::vector<char>& buffer) {
    std::string buffer_content;
    buffer_content.reserve(buffer.size());
    std::copy(buffer.begin(), buffer.end(), std::back_inserter(buffer_content));
    return buffer_content;
}

auto validate_buffers(const GapBuffer<char>& gap_buffer, const std::vector<char>& buffer) {
    ASSERT_EQ(buffer.size(), gap_buffer.size());
    const auto gap_buffer_content = to_string(gap_buffer);
    const auto buffer_content = to_string(buffer);
    ASSERT_EQ(gap_buffer_content, buffer_content);
}

auto insert_at_start(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, const std::string& word) {
    gap_buffer.insert(make_crange(word), 0);
    buffer.insert(buffer.begin(), word.begin(), word.end());
    validate_buffers(gap_buffer, buffer);
}

auto insert_at_end(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, const std::string& word) {
    gap_buffer.insert(make_crange(word), gap_buffer.size());
    buffer.insert(buffer.end(), word.begin(), word.end());
    validate_buffers(gap_buffer, buffer);
}

auto insert(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int position, const std::string& word) {
    gap_buffer.insert(make_crange(word), position);
    buffer.insert(buffer.begin() + position, word.begin(), word.end());
    validate_buffers(gap_buffer, buffer);
}

auto append(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, const std::string& word) {
    gap_buffer.append(make_crange(word));
    buffer.insert(buffer.end(), word.begin(), word.end());
    validate_buffers(gap_buffer, buffer);
}

auto remove(std::vector<char>& buffer, int position, int count) {
    buffer.erase(buffer.begin() + position, buffer.begin() + position + count);
}

auto remove(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int position, int count) {
    gap_buffer.remove(position, count);
    remove(buffer, position, count);
    validate_buffers(gap_buffer, buffer);
}

auto remove_at_start(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int count) {
    gap_buffer.remove(0, count);
    remove(buffer, 0, count);
    validate_buffers(gap_buffer, buffer);
}

auto remove_at_end(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int count) {
    gap_buffer.remove(gap_buffer.size() - count, count);
    remove(buffer, buffer.size() - count, count);
    validate_buffers(gap_buffer, buffer);
}

auto replace(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int position, int count, const std::string& word) {
    gap_buffer.replace(position, count, make_crange(word));
    remove(buffer, position, count);
    buffer.insert(buffer.begin() + position, word.begin(), word.end());
}

auto replace_at_start(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int count, const std::string& word) {
    gap_buffer.replace(0, count, make_crange(word));
    remove(buffer, 0, count);
    buffer.insert(buffer.begin(), word.begin(), word.end());
}

auto replace_at_end(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int count, const std::string& word) {
    gap_buffer.replace(gap_buffer.size() - count, count, make_crange(word));
    remove(buffer, buffer.size() - count, count);
    buffer.insert(buffer.begin(), word.begin(), word.end());
}

template<typename RandomEngine, typename ElementAccessor>
auto make_fair_random_distribution(RandomEngine& random_engine, ElementAccessor&& access_element, int element_count, int access_count) {
    std::vector<int> access_tracker;
    access_tracker.resize(element_count, access_count);
    std::uniform_int_distribution<> distribution{0, element_count - 1};
    return [&random_engine, element_count, access_element = std::move(access_element), access_tracker = std::move(access_tracker), distribution = std::move(distribution)]() mutable {
        if (element_count == 0) {
            throw std::logic_error("No more accesses permitted");
        }
        int element = distribution(random_engine);
        int& access_count = access_tracker.at(element);
        access_count -= 1;
        if (access_count == 0) {
            std::stable_partition(access_tracker.begin(), access_tracker.end(), [](int access_count) { return access_count > 0; });
            element_count -= 1;
            if (element_count > 0) {
                distribution = std::uniform_int_distribution<>{0, element_count - 1};
            }
        }
        return access_element(element);
    };
}
}

void random_word_operations() {
    std::mt19937 random_engine;
    int min_word_size = 0;
    int max_word_size = 10;
    int word_size_count = max_word_size - min_word_size + 1;
    auto generate_random_word = make_random_word_generators(random_engine, alphabet(), min_word_size, max_word_size);
    int word_count = 100;
    auto generate_fair_random_word = make_fair_random_distribution(random_engine, generate_random_word, word_size_count, word_count);
    int total_word_count = word_count * word_size_count;
    std::vector<int> word_size_tracker;
    word_size_tracker.resize(word_size_count, 0);
    for (int count = 0; count < total_word_count; ++count) {
        const auto word = generate_fair_random_word();
        int& current_word_count = word_size_tracker.at(word.size());
        current_word_count += 1;
    }
    ASSERT_TRUE(std::all_of(word_size_tracker.begin(), word_size_tracker.end(), [word_count](auto count) {
        return count == word_count;
    }));
}

void validate_gap_buffer_content(const GapBuffer<char>& gap_buffer, const std::string& expected_content) {
    const auto gap_buffer_content = to_string(gap_buffer);
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

TEST(random_word_generator, random_word_operations) {
    cursor::test::gap_buffer::random_word_operations();
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
