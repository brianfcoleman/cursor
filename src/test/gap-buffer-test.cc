#include "gap-buffer.hh"
#include "range.hh"

#include <gtest/gtest.h>

#include <algorithm>
#include <iterator>
#include <random>
#include <stdexcept>
#include <tuple>
#include <type_traits>

namespace cursor {
namespace test {
namespace gap_buffer {
namespace {
struct Position {
    Position(int value_)
        : value{ value_ }
    {
    }
    operator int() const { return value; }
    int value;
};

struct Count {
    Count(int value_)
        : value{ value_ }
    {
    }
    operator int() const { return value; }
    int value;
};

std::string alphabet() { return "abcdefghijklmnopqrstuvwxyz"; }

template <typename RandomEngine> auto make_random_int_generator(RandomEngine& random_engine, int min, int max)
{
    std::uniform_int_distribution<> distribution{ min, max };
    return [distribution, &random_engine]() mutable -> int { return distribution(random_engine); };
}

template <typename RandomIntGenerator>
auto make_random_letter_generator(RandomIntGenerator&& generate_random_int, const std::string& alphabet)
{
    return [ generate_random_int = std::move(generate_random_int), alphabet ]() mutable->char
    {
        return alphabet.at(generate_random_int());
    };
}

template <typename RandomEngine>
auto make_random_word_generator(RandomEngine& random_engine, const std::string& alphabet, int word_size)
{
    auto generate_random_int = make_random_int_generator(random_engine, 0, alphabet.size() - 1);
    auto generate_random_letter = make_random_letter_generator(generate_random_int, alphabet);
    return [ generate_random_letter = std::move(generate_random_letter), word_size ]()->std::string
    {
        std::string random_word;
        random_word.resize(word_size);
        std::generate(random_word.begin(), random_word.end(), generate_random_letter);
        return random_word;
    };
}

template <typename RandomEngine>
auto make_random_word_generators(
    RandomEngine& random_engine, const std::string& alphabet, int min_word_size, int max_word_size)
{
    using RandomWordGenerator = decltype(make_random_word_generator(random_engine, alphabet, min_word_size));
    std::vector<RandomWordGenerator> random_word_generators;
    for (int word_size = min_word_size; word_size <= max_word_size; ++word_size) {
        random_word_generators.push_back(make_random_word_generator(random_engine, alphabet, word_size));
    }
    return [random_word_generators = std::move(random_word_generators)](int word_size)->std::string
    {
        auto generate_random_word = random_word_generators.at(word_size);
        return generate_random_word();
    };
}

auto to_string(const GapBuffer<char>& gap_buffer)
{
    std::string gap_buffer_content;
    gap_buffer_content.reserve(gap_buffer_content.size());
    std::copy(gap_buffer.cbegin(), gap_buffer.cend(), std::back_inserter(gap_buffer_content));
    return gap_buffer_content;
}

auto to_string(const std::vector<char>& buffer)
{
    std::string buffer_content;
    buffer_content.reserve(buffer.size());
    std::copy(buffer.begin(), buffer.end(), std::back_inserter(buffer_content));
    return buffer_content;
}

auto validate_buffers(const GapBuffer<char>& gap_buffer, const std::vector<char>& buffer)
{
    ASSERT_EQ(buffer.size(), gap_buffer.size());
    const auto gap_buffer_content = to_string(gap_buffer);
    const auto buffer_content = to_string(buffer);
    ASSERT_EQ(gap_buffer_content, buffer_content);
}

auto insert_at_start(GapBuffer<char>& gap_buffer, const std::string& word) { gap_buffer.insert(make_crange(word), 0); }

auto insert_at_start(std::vector<char>& buffer, const std::string& word)
{
    buffer.insert(buffer.begin(), word.begin(), word.end());
}

auto insert_at_start(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, const std::string& word)
{
    insert_at_start(gap_buffer, word);
    insert_at_start(buffer, word);
    validate_buffers(gap_buffer, buffer);
}

auto insert_at_end(GapBuffer<char>& gap_buffer, const std::string& word)
{
    gap_buffer.insert(make_crange(word), gap_buffer.size());
}

auto insert_at_end(std::vector<char>& buffer, const std::string& word)
{
    buffer.insert(buffer.end(), word.begin(), word.end());
}

auto insert_at_end(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, const std::string& word)
{
    insert_at_end(gap_buffer, word);
    insert_at_end(buffer, word);
    validate_buffers(gap_buffer, buffer);
}

auto insert(GapBuffer<char>& gap_buffer, int position, const std::string& word)
{
    gap_buffer.insert(make_crange(word), position);
}

auto insert(std::vector<char>& buffer, int position, const std::string& word)
{
    buffer.insert(buffer.begin() + position, word.begin(), word.end());
}

auto insert(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int position, const std::string& word)
{
    insert(gap_buffer, position, word);
    insert(gap_buffer, position, word);
    validate_buffers(gap_buffer, buffer);
}

auto append(GapBuffer<char>& gap_buffer, const std::string& word) { gap_buffer.append(make_crange(word)); }

auto append(std::vector<char>& buffer, const std::string& word)
{
    buffer.insert(buffer.end(), word.begin(), word.end());
}

auto append(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, const std::string& word)
{
    append(gap_buffer, word);
    append(buffer, word);
    validate_buffers(gap_buffer, buffer);
}

auto remove(GapBuffer<char>& gap_buffer, int position, int count) { gap_buffer.remove(position, count); }

auto remove(std::vector<char>& buffer, int position, int count)
{
    buffer.erase(buffer.begin() + position, buffer.begin() + position + count);
}

auto remove(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int position, int count)
{
    remove(gap_buffer, position, count);
    remove(buffer, position, count);
    validate_buffers(gap_buffer, buffer);
}

auto remove_at_start(GapBuffer<char>& gap_buffer, int count) { gap_buffer.remove(0, count); }

auto remove_at_start(std::vector<char>& buffer, int count) { remove(buffer, 0, count); }

auto remove_at_start(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int count)
{
    remove_at_start(gap_buffer, count);
    remove_at_start(buffer, count);
    validate_buffers(gap_buffer, buffer);
}

auto remove_at_end(GapBuffer<char>& gap_buffer, int count) { gap_buffer.remove(gap_buffer.size() - count, count); }

auto remove_at_end(std::vector<char>& buffer, int count) { remove(buffer, buffer.size() - count, count); }

auto remove_at_end(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int count)
{
    remove_at_end(gap_buffer, count);
    remove_at_end(buffer, count);
    validate_buffers(gap_buffer, buffer);
}

auto replace(GapBuffer<char>& gap_buffer, int position, int count, const std::string& word)
{
    gap_buffer.replace(position, count, make_crange(word));
}

auto replace(std::vector<char>& buffer, int position, int count, const std::string& word)
{
    remove(buffer, position, count);
    buffer.insert(buffer.begin() + position, word.begin(), word.end());
}

auto replace(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int position, int count, const std::string& word)
{
    replace(gap_buffer, position, count, word);
    replace(buffer, position, count, word);
    validate_buffers(gap_buffer, buffer);
}

auto replace_at_start(GapBuffer<char>& gap_buffer, int count, const std::string& word)
{
    gap_buffer.replace(0, count, make_crange(word));
}

auto replace_at_start(std::vector<char>& buffer, int count, const std::string& word)
{
    replace(buffer, 0, count, word);
}

auto replace_at_start(GapBuffer<char> gap_buffer, std::vector<char>& buffer, int count, const std::string& word)
{
    replace_at_start(gap_buffer, count, word);
    replace_at_start(buffer, count, word);
    validate_buffers(gap_buffer, buffer);
}

auto replace_at_end(GapBuffer<char>& gap_buffer, int count, const std::string& word)
{
    gap_buffer.replace(gap_buffer.size() - count, count, make_crange(word));
}

auto replace_at_end(std::vector<char>& buffer, int count, const std::string& word)
{
    remove(buffer, buffer.size() - count, count);
    buffer.insert(buffer.begin(), word.begin(), word.end());
}

auto replace_at_end(GapBuffer<char>& gap_buffer, std::vector<char>& buffer, int count, const std::string& word)
{
    replace_at_end(gap_buffer, count, word);
    replace_at_end(buffer, count, word);
    validate_buffers(gap_buffer, buffer);
}

template <typename RandomEngine> auto make_random_position_generator(RandomEngine& engine)
{
    return [&engine](int size) {
        if (size == 0) {
            return 0;
        }
        std::uniform_int_distribution<> distribution{ 0, size };
        return distribution(engine);
    };
}

template <typename RandomEngine> auto make_random_count_generator(RandomEngine& engine)
{
    return [&engine](int size) {
        if (size == 0) {
            return 0;
        }
        std::uniform_int_distribution<> distribution{ 0, size };
        return distribution(engine);
    };
}

using CharGapBuffer = GapBuffer<char>;
using CharBuffer = std::vector<char>;
using Word = std::string;

template <typename... Arguments> struct Signature {
};
using PositionWordSignature = Signature<CharGapBuffer, CharBuffer, Position, Word>;
using WordSignature = Signature<CharGapBuffer, CharBuffer, Word>;
using PositionCountSignature = Signature<CharGapBuffer, CharBuffer, Position, Count>;
using CountSignature = Signature<CharGapBuffer, CharBuffer, Count>;
using PositionCountWordSignature = Signature<CharGapBuffer, CharBuffer, Position, Count, Word>;
using CountWordSignature = Signature<CharGapBuffer, CharBuffer, Count, Word>;

template <typename... Arguments> auto make_signature(void operation(Arguments...))
{
    return Signature<std::remove_reference_t<std::remove_cv_t<Arguments> >...>{};
}

template <typename RandomPositionGenerator, typename RandomCountGenerator, typename RandomWordGenerator,
    typename BufferOperation>
auto make_random_buffer_operation(RandomPositionGenerator generate_random_position,
    RandomCountGenerator generate_random_count, RandomWordGenerator& generate_random_word,
    BufferOperation&& operate_on_buffer, PositionWordSignature signature)
{
    return [operate_on_buffer, &generate_random_position, &generate_random_word](CharGapBuffer& gap_buffer, CharBuffer& buffer) {
        const auto word_count = 1;
        const auto position = generate_random_position(gap_buffer.size());
        const auto word = generate_random_word();
        operate_on_buffer(gap_buffer, buffer, position, word);
        return word_count;
    };
}

template <typename RandomPositionGenerator, typename RandomCountGenerator, typename RandomWordGenerator,
    typename BufferOperation>
auto make_random_buffer_operation(RandomPositionGenerator generate_random_position,
    RandomCountGenerator generate_random_count, RandomWordGenerator& generate_random_word,
    BufferOperation&& operate_on_buffer, WordSignature signature)
{
    return [operate_on_buffer, &generate_random_word](CharGapBuffer& gap_buffer, CharBuffer& buffer) {
        const auto word_count = 1;
        const auto word = generate_random_word();
        operate_on_buffer(gap_buffer, buffer, word);
        return word_count;
    };
}

template <typename RandomPositionGenerator, typename RandomCountGenerator, typename RandomWordGenerator,
    typename BufferOperation>
auto make_random_buffer_operation(RandomPositionGenerator generate_random_position,
    RandomCountGenerator generate_random_count, RandomWordGenerator& generate_random_word,
    BufferOperation&& operate_on_buffer, PositionCountSignature signature)
{
    return [operate_on_buffer, &generate_random_position, &generate_random_count](CharGapBuffer& gap_buffer, CharBuffer& buffer) {
        const auto word_count = 0;
        const auto position = generate_random_position(gap_buffer.size());
        const auto count = generate_random_count(gap_buffer.size() - position);
        operate_on_buffer(gap_buffer, buffer, position, count);
        return word_count;
    };
}

template <typename RandomPositionGenerator, typename RandomCountGenerator, typename RandomWordGenerator,
    typename BufferOperation>
auto make_random_buffer_operation(RandomPositionGenerator generate_random_position,
    RandomCountGenerator generate_random_count, RandomWordGenerator& generate_random_word,
    BufferOperation&& operate_on_buffer, CountSignature signature)
{
    return [operate_on_buffer, &generate_random_count](CharGapBuffer& gap_buffer, CharBuffer& buffer) {
        const auto word_count = 0;
        const auto count = generate_random_count(gap_buffer.size());
        operate_on_buffer(gap_buffer, buffer, count);
        return word_count;
    };
}

template <typename RandomPositionGenerator, typename RandomCountGenerator, typename RandomWordGenerator,
    typename BufferOperation>
auto make_random_buffer_operation(RandomPositionGenerator generate_random_position,
    RandomCountGenerator generate_random_count, RandomWordGenerator& generate_random_word,
    BufferOperation&& operate_on_buffer, PositionCountWordSignature signature)
{
    return [operate_on_buffer, &generate_random_position, &generate_random_count, &generate_random_word](CharGapBuffer& gap_buffer, CharBuffer& buffer) {
        const auto word_count = 0;
        const auto position = generate_random_position(gap_buffer.size());
        const auto count = generate_random_count(gap_buffer.size() - position);
        const auto word = generate_random_word();
        operate_on_buffer(gap_buffer, buffer, position, count, word);
        return word_count;
    };
}

template <typename RandomPositionGenerator, typename RandomCountGenerator, typename RandomWordGenerator,
    typename BufferOperation>
auto make_random_buffer_operation(RandomPositionGenerator generate_random_position,
    RandomCountGenerator generate_random_count, RandomWordGenerator& generate_random_word,
    BufferOperation&& operate_on_buffer, CountWordSignature signature)
{
    return [operate_on_buffer, &generate_random_count, &generate_random_word](CharGapBuffer& gap_buffer, CharBuffer& buffer) {
        const auto word_count = 0;
        const auto count = generate_random_count(gap_buffer.size());
        const auto word = generate_random_word();
        operate_on_buffer(gap_buffer, buffer, count, word);
        return word_count;
    };
}

template <typename RandomPositionGenerator, typename RandomCountGenerator, typename RandomWordGenerator,
    typename BufferOperation>
auto make_random_buffer_operation(RandomPositionGenerator generate_random_position,
    RandomCountGenerator generate_random_count, RandomWordGenerator& generate_random_word,
    BufferOperation&& operate_on_buffer)
{
    using Signature = decltype(make_signature(operate_on_buffer)());
    return make_random_buffer_operation(
        generate_random_position, generate_random_count, generate_random_word, operate_on_buffer, Signature{});
}

template <typename Operation> auto repeat(Operation&& operation, int repeat_count)
{
    for (int count = 0; count < repeat_count; ++count) {
        operation();
    }
}

template <typename RandomPositionGenerator, typename RandomCountGenerator, typename RandomWordGenerator,
    typename BufferOperation>
auto make_random_sequential_buffer_operations(RandomPositionGenerator generate_random_position,
    RandomCountGenerator generate_random_count, RandomWordGenerator& generate_random_word,
    BufferOperation&& operate_on_buffer, int sequence_count, PositionWordSignature signature)
{
    return [operate_on_buffer, sequence_count, &generate_random_position, &generate_random_word](
        CharGapBuffer& gap_buffer, CharBuffer& buffer) {
        const auto word_count = sequence_count;
        auto position = generate_random_position(gap_buffer.size());
        repeat([operate_on_buffer, &generate_random_word, &position, &gap_buffer, &buffer] {
            const auto word = generate_random_word();
            operate_on_buffer(gap_buffer, buffer, position, word);
            position = std::min(position + word.size(), gap_buffer.size());
        }, sequence_count);
        return word_count;
    };
}

template <typename RandomPositionGenerator, typename RandomCountGenerator, typename RandomWordGenerator,
    typename BufferOperation>
auto make_random_sequential_buffer_operations(RandomPositionGenerator generate_random_position,
    RandomCountGenerator generate_random_count, RandomWordGenerator& generate_random_word,
    BufferOperation&& operate_on_buffer, int sequence_count, PositionCountSignature signature)
{
    return [operate_on_buffer, sequence_count, &generate_random_position, &generate_random_count](
        CharGapBuffer& gap_buffer, CharBuffer& buffer) {
        const auto word_count = 0;
        auto position = generate_random_position(gap_buffer.size());
        repeat([operate_on_buffer, &generate_random_count, &position, &gap_buffer, &buffer] {
            const auto count = generate_random_count(gap_buffer.size() - position);
            operate_on_buffer(gap_buffer, buffer, position, count);
            position = std::min(position + count, gap_buffer.size());
        }, sequence_count);
        return word_count;
    };
}

template <typename RandomPositionGenerator, typename RandomCountGenerator, typename RandomWordGenerator,
    typename BufferOperation>
auto make_random_sequential_buffer_operations(RandomPositionGenerator generate_random_position,
    RandomCountGenerator generate_random_count, RandomWordGenerator& generate_random_word,
    BufferOperation&& operate_on_buffer, int sequence_count, PositionCountWordSignature signature)
{
    return [operate_on_buffer, sequence_count, &generate_random_position, &generate_random_count,
        &generate_random_word](CharGapBuffer& gap_buffer, CharBuffer& buffer) {
        const auto word_count = 0;
        auto position = generate_random_position(gap_buffer.size());
        repeat([operate_on_buffer, &generate_random_count, &generate_random_word, &position, &gap_buffer, &buffer] {
            const auto count = generate_random_count(gap_buffer.size() - position);
            const auto word = generate_random_word();
            operate_on_buffer(gap_buffer, buffer, position, count, word);
            position = std::min(position + count, gap_buffer.size());
        }, sequence_count);
        return word_count;
    };
}

template <typename RandomPositionGenerator, typename RandomCountGenerator, typename RandomWordGenerator,
    typename BufferOperation>
auto make_random_sequential_buffer_operations(RandomPositionGenerator generate_random_position,
    RandomCountGenerator generate_random_count, RandomWordGenerator& generate_random_word,
    BufferOperation&& operate_on_buffer, int sequence_count)
{
    using Signature = decltype(make_signature(operate_on_buffer)());
    return make_random_sequential_buffer_operations(generate_random_position, generate_random_count,
        generate_random_word, operate_on_buffer, sequence_count, Signature{});
}

template <typename InRangeA, typename InRangeB, typename OutRange>
auto zip(InRangeA range_in_a, InRangeB range_in_b, OutRange range_out)
{
    if (range_in_a.size() != range_in_b.size()) {
        throw std::range_error("range_in_a.size() != range_in_b.size()");
    }
    if (range_in_a.size() != range_out.size()) {
        throw std::range_error("range_in_a.size() != range_out.size()");
    }
    for (auto it_a = range_in_a.begin(), it_b = range_in_b.begin(), it_out = range_out.begin();
         it_out != range_out.end(); ++it_a, ++it_b, ++it_out) {
        *it_out = std::make_tuple(*it_a, *it_b);
    }
}

template <typename RandomEngine, typename ElementAccessor>
auto make_fair_random_distribution(
    RandomEngine& random_engine, ElementAccessor&& access_element, int element_count, int access_count)
{
    std::vector<int> element_keys;
    element_keys.resize(element_count);
    std::iota(element_keys.begin(), element_keys.end(), 0);
    std::vector<int> access_counts;
    access_counts.resize(element_count, access_count);
    std::vector<std::tuple<int, int> > access_tracker;
    access_tracker.resize(element_count);
    zip(make_range(access_counts), make_range(element_keys), make_range(access_tracker));
    std::uniform_int_distribution<> distribution{ 0, element_count - 1 };
    return [
        &random_engine,
        element_count,
        access_element = std::move(access_element),
        access_tracker = std::move(access_tracker),
        distribution = std::move(distribution)
    ]() mutable
    {
        if (element_count == 0) {
            throw std::logic_error("No more accesses permitted");
        }
        int element_index = distribution(random_engine);
        int access_count = 0;
        int element_key = 0;
        std::tie(access_count, element_key) = access_tracker.at(element_index);
        access_count -= 1;
        access_tracker.at(element_index) = std::make_tuple(access_count, element_key);
        if (access_count == 0) {
            std::stable_partition(access_tracker.begin(), access_tracker.end(), [](const auto& tracked_access) {
                int element_index = 0;
                int access_count = 0;
                std::tie(access_count, element_index) = tracked_access;
                return access_count > 0;
            });
            element_count -= 1;
            if (element_count > 0) {
                distribution = std::uniform_int_distribution<>{ 0, element_count - 1 };
            }
        }
        return access_element(element_key);
    };
}
}

void random_word_operations()
{
    std::mt19937 random_engine;
    int min_word_size = 0;
    int max_word_size = 10;
    int word_size_count = max_word_size - min_word_size + 1;
    auto generate_random_word = make_random_word_generators(random_engine, alphabet(), min_word_size, max_word_size);
    int word_count = 100;
    auto generate_fair_random_word
        = make_fair_random_distribution(random_engine, generate_random_word, word_size_count, word_count);
    int total_word_count = word_count * word_size_count;
    std::vector<int> word_size_tracker;
    word_size_tracker.resize(word_size_count, 0);
    for (int count = 0; count < total_word_count; ++count) {
        const auto word = generate_fair_random_word();
        int& current_word_count = word_size_tracker.at(word.size());
        current_word_count += 1;
    }
    ASSERT_TRUE(std::all_of(
        word_size_tracker.begin(), word_size_tracker.end(), [word_count](auto count) { return count == word_count; }));
}

void validate_gap_buffer_content(const GapBuffer<char>& gap_buffer, const std::string& expected_content)
{
    const auto gap_buffer_content = to_string(gap_buffer);
    ASSERT_EQ(expected_content, gap_buffer_content);
}

void insert_before_position()
{
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.insert(make_crange(content), 0);
    validate_gap_buffer_content(gap_buffer, content);
}

void insert_before_iterator()
{
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.insert(make_crange(content), gap_buffer.cbegin());
    validate_gap_buffer_content(gap_buffer, content);
}

void append()
{
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.append(make_crange(content));
    validate_gap_buffer_content(gap_buffer, content);
}

void remove_at_position()
{
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World";
    gap_buffer.append(content);
    validate_gap_buffer_content(gap_buffer, content);
    gap_buffer.remove(0, content.size());
    content = "";
    validate_gap_buffer_content(gap_buffer, content);
}

void remove_at_iterator()
{
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.append(content);
    validate_gap_buffer_content(gap_buffer, content);
    gap_buffer.remove(make_crange(gap_buffer));
    content = "";
    validate_gap_buffer_content(gap_buffer, content);
}

void replace_at_position()
{
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.append(content);
    validate_gap_buffer_content(gap_buffer, content);
    std::string new_content = "Goodbye World!";
    gap_buffer.replace(0, content.size(), make_crange(new_content));
    validate_gap_buffer_content(gap_buffer, new_content);
}

void replace_at_iterator()
{
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.append(content);
    validate_gap_buffer_content(gap_buffer, content);
    std::string new_content = "Goodbye World!";
    gap_buffer.replace(make_crange(gap_buffer), make_crange(new_content));
    validate_gap_buffer_content(gap_buffer, new_content);
}

void size()
{
    GapBuffer<char> gap_buffer;
    std::string content = "Hello World!";
    gap_buffer.append(content);
    validate_gap_buffer_content(gap_buffer, content);
    ASSERT_EQ(content.size(), gap_buffer.size());
}
}
}
}

TEST(gap_buffer, insert_before_position) { cursor::test::gap_buffer::insert_before_position(); }

TEST(gap_buffer, insert_before_iterator) { cursor::test::gap_buffer::insert_before_iterator(); }

TEST(gap_buffer, append) { cursor::test::gap_buffer::append(); }

TEST(gap_buffer, remove_at_position) { cursor::test::gap_buffer::remove_at_position(); }

TEST(gap_buffer, remove_at_iterator) { cursor::test::gap_buffer::remove_at_iterator(); }

TEST(gap_buffer, replace_at_position) { cursor::test::gap_buffer::replace_at_position(); }

TEST(gap_buffer, replace_at_iterator) { cursor::test::gap_buffer::replace_at_iterator(); }

TEST(gap_buffer, size) { cursor::test::gap_buffer::size(); }

TEST(random_word_generator, random_word_operations) { cursor::test::gap_buffer::random_word_operations(); }

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
