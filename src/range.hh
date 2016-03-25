#pragma once

#include <iterator>

namespace cursor {
template<typename Iterator>
class Range {
public:

    using iterator = Iterator;
    using difference_type = typename std::iterator_traits<iterator>::difference_type;

    Range() {}
    Range(iterator first_, iterator last_) : first{first_}, last{last_} {}

    difference_type size() const { return std::distance(first, last); }

    iterator begin() const { return first; }
    iterator end() const { return last; }

    iterator begin() { return first; }
    iterator end() { return last; }

private:

    iterator first;
    iterator last;
};

template<typename Iterator>
auto make_range(Iterator first, Iterator last) {
    return Range<Iterator>(first, last);
}

template<typename Container>
auto make_crange(const Container& container) {
    return Range<typename Container::const_iterator>(container);
}

template<typename Container>
auto make_range(Container& container) {
    return Range<typename Container::iterator>(container);
}

}
