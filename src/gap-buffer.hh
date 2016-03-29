#pragma once

#include "range.hh"

#include <boost/iterator/iterator_facade.hpp>

#include <algorithm>
#include <iterator>
#include <memory>
#include <stdexcept>

#include <cassert>

namespace cursor {
template<typename Element>
class GapBufferIterator : public boost::iterator_facade<GapBufferIterator<Element>, Element, boost::random_access_traversal_tag> {
public:
    using Facade = boost::iterator_facade<GapBufferIterator<Element>, Element, boost::random_access_traversal_tag>;
    using difference_type = typename Facade::difference_type;
    using value_type = typename Facade::value_type;
    using pointer = typename Facade::pointer;
    using reference = typename Facade::reference;
    using iterator_category = typename Facade::iterator_category;

    GapBufferIterator(pointer buffer_, difference_type buffer_position_, difference_type buffer_size_, difference_type gap_position_, difference_type gap_size_)
        : buffer{buffer_}, buffer_position{buffer_position_}, buffer_size{buffer_size_}, gap_position{gap_position_}, gap_size{gap_size_} {}

private:
    friend class boost::iterator_core_access;

    bool equal(const GapBufferIterator& other) const {
        assert(is_other_iterator_valid(other));
        assert(is_buffer_position_valid());
        assert(other.is_buffer_position_valid());
        return buffer_position == other.buffer_position;
    }

    reference dereference() const {
        assert(is_buffer_position_valid());
        return buffer[buffer_position];
    }

    void increment() {
        if ((buffer_position + 1) == gap_position) {
            assert(buffer_position < (buffer_size - gap_size));
            buffer_position += (1 + gap_size);
        } else {
            assert(buffer_position < buffer_size);
            buffer_position += 1;
        }
        assert(is_buffer_position_valid());
    }

    void decrement() {
        if (buffer_position == (gap_position + gap_size)) {
            assert(buffer_position > gap_size);
            buffer_position -= (1 + gap_size);
        } else {
            assert(buffer_position > 0);
            buffer_position -= 1;
        }
        assert(is_buffer_position_valid());
    }

    void advance(difference_type count) {
        if (count == 0) {
            return;
        } else if (count > 0) {
            if ((buffer_position < gap_position) && ((buffer_position + count) >= gap_position)) {
                assert(buffer_position <= (buffer_size - count - gap_size));
                buffer_position += (count + gap_size);
            } else {
                assert(buffer_position <= (buffer_size - count));
                buffer_position += count;
            }
        } else {
            if ((buffer_position >= gap_size) && ((buffer_position - count) < gap_size)) {
                assert(buffer_position >= (count + gap_size));
                buffer_position -= (count + gap_size);
            } else {
                assert(buffer_position >= count);
                buffer_position -= count;
            }
        }
        assert(is_buffer_position_valid());
    }

    difference_type distance_to(const GapBufferIterator& other) const {
        assert(is_other_iterator_valid(other));
        assert(is_buffer_position_valid());
        assert(other.is_buffer_position_valid());

        const auto is_before_gap = (buffer_position < gap_position);
        const auto is_after_gap = (buffer_position >= (gap_position + gap_size));

        const auto is_other_before_gap = (other.buffer_position < gap_position);
        const auto is_other_after_gap = (other.buffer_position >= (gap_position + gap_size));

        if (is_before_gap && is_other_after_gap) {
            return (other.buffer_position - gap_size) - buffer_position;
        } else if (is_after_gap && is_other_before_gap) {
            return other.buffer_position - (buffer_position - gap_size);
        } else {
            return other.buffer_position - buffer_position;
        }
    }

    bool is_other_iterator_valid(const GapBufferIterator& other) const {
        if (buffer != other.buffer) { return false; }
        if (buffer_size != other.buffer_size) { return false; }
        if (gap_position != other.gap_position) { return false; }
        if (gap_size != other.gap_size) { return false; }
        return true;
    }

    bool is_buffer_position_valid() const {
        if (buffer_position > buffer_size) { return false; }
        if ((buffer_position != buffer_size) && (buffer_position >= gap_position) && (buffer_position < (gap_position + gap_size))) { return false; }
        return true;
    }

    pointer buffer;
    difference_type buffer_position = 0;
    difference_type buffer_size = 0;
    difference_type gap_position = 0;
    difference_type gap_size = 0;
};


template<typename Element>
class GapBuffer {
public:
    using Buffer = std::unique_ptr<Element[]>;
    using iterator = GapBufferIterator<Element>;
    using const_iterator = GapBufferIterator<const Element>;
    using size_type = typename iterator::difference_type;
    using range = Range<iterator>;
    using const_range = Range<const_iterator>;

    template<typename ElementRange>
    void insert(ElementRange insert_range, size_type position) {
        validate_position(position);

        move_gap(position);
        expand_gap(insert_range.size());

        auto gap_begin = buffer_begin() + gap_position;
        std::copy(insert_range.begin(), insert_range.end(), gap_begin);

        gap_position += insert_range.size();
        gap_size -= insert_range.size();
    }

    template<typename ElementRange>
    void insert(ElementRange insert_range, const_iterator element) {
        const auto position = std::distance(cbegin(), element);
        insert(insert_range, position);
    }

    template<typename ElementRange>
    void append(ElementRange append_range) {
        insert(append_range, size());
    }

    void remove(size_type position, size_type count) {
        validate_position(position);
        validate_position(position + count);

        move_gap(position);
        gap_size += count;
    }

    void remove(const_range remove_range) {
        const auto position = std::distance(cbegin(), remove_range.begin());
        const auto count = std::distance(remove_range.begin(), remove_range.end());
        remove(position, count);
    }

    template<typename ElementRange>
    void replace(size_type position, size_type count, ElementRange insert_range) {
        remove(position, count);
        insert(insert_range, position);
    }

    template<typename ElementRange>
    void replace(const_range remove_range, ElementRange insert_range) {
        const auto position = std::distance(cbegin(), remove_range.begin());
        const auto count = std::distance(remove_range.begin(), remove_range.end());
        replace(position, count, insert_range);
    }

    size_type size() const { return buffer_size - gap_size; }

    iterator begin() {
        auto position = (gap_position == 0) ? gap_size : 0;
        return iterator(buffer.get(), position, buffer_size, gap_position, gap_size);
    }
    iterator end() {
        return iterator(buffer.get(), buffer_size, buffer_size, gap_position, gap_size);
    }

    const_iterator begin() const {
        auto position = (gap_position == 0) ? gap_size : 0;
        return const_iterator(buffer.get(), position, buffer_size, gap_position, gap_size);
    }
    const_iterator end() const {
        return const_iterator(buffer.get(), buffer_size, buffer_size, gap_position, gap_size);
    }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

private:

    bool is_valid_position(size_type position) {
        return position <= size();
    }

    void validate_position(size_type position) {
        if (!is_valid_position(position)) {
            throw std::out_of_range("Invalid position");
        }
    }

    size_type to_buffer_position(size_type position) {
        if (position < gap_position) {
            return position;
        } else {
            return position + gap_size;
        }
    }

    const Element* buffer_begin() const { return buffer.get(); }
    const Element* buffer_end() const { buffer_begin() + buffer_size; }

    Element* buffer_begin() { return buffer.get(); }
    Element* buffer_end() { return buffer_begin() + buffer_size; }

    void move_gap(size_type new_gap_position) {
        if (gap_position == new_gap_position) {
            return;
        }

        auto gap_begin = buffer_begin() + gap_position;
        auto gap_end = gap_begin + gap_size;
        auto new_gap_begin = buffer_begin() + new_gap_position;
        auto new_gap_end = new_gap_begin + gap_size;

        if (new_gap_position < gap_position) {
            std::copy(new_gap_begin, gap_begin, new_gap_end);
        } else {
            std::copy(gap_end, new_gap_begin, gap_begin);
        }

        gap_position = new_gap_position;
    }

    void expand_gap(size_type min_gap_size) {
        if (gap_size >= min_gap_size) {
            return;
        }

        const auto min_buffer_size = (buffer_size - gap_size) + min_gap_size;
        auto new_buffer_size = std::max(buffer_size, static_cast<decltype(buffer_size)>(1));
        while (new_buffer_size <= min_buffer_size) {
            new_buffer_size *= 2;
        }
        const auto new_gap_size = new_buffer_size - buffer_size;

        auto new_buffer = std::make_unique<Element[]>(new_buffer_size);

        auto buffer_end = buffer_begin() + buffer_size;

        auto gap_begin = buffer_begin() + gap_position;
        auto gap_end = gap_begin + gap_size;

        auto new_buffer_begin = new_buffer.get();

        auto new_gap_begin = new_buffer_begin + gap_position;
        auto new_gap_end = new_gap_begin + new_gap_size;

        std::copy(buffer_begin(), gap_begin, new_buffer_begin);
        std::copy(gap_end, buffer_end, new_gap_end);

        buffer = std::move(new_buffer);
        buffer_size = new_buffer_size;
        gap_size = new_gap_size;
    }

    Buffer buffer;
    size_type buffer_size = 0;
    size_type gap_position = 0;
    size_type gap_size = 0;
};

}
