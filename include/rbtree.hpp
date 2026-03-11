#ifndef GKERNEL_RBTREE_HPP
#define GKERNEL_RBTREE_HPP

#include <set>
#include <utility>

namespace gkernel {

template <typename T, class Comparator = std::less<T>> class RBTree {
public:
    using iterator = typename std::set<T, Comparator>::iterator;

    enum class State { inf_negative = -1, exists = 0, inf_positive = 1 };

    RBTree() : internalTree(std::less<T>{}) {}

    RBTree(const Comparator& comp) : internalTree(comp) {}

    auto begin() const {
        return internalTree.begin();
    }

    auto end() const {
        return internalTree.end();
    }

    auto insert(const T& item) {
        return internalTree.insert(item);
    }

    void insert(std::initializer_list<T> items) {
        internalTree.insert(items);
    }

    const T& max() {
        return *(--internalTree.end());
    }

    const T& min() {
        return *(internalTree.begin());
    }

    std::size_t erase(const T& item) {
        return internalTree.erase(item);
    }

    void erase(iterator it) {
        internalTree.erase(it);
    }

    std::size_t size() const {
        return internalTree.size();
    }

    iterator find(const T& item) {
        return internalTree.find(item);
    }

    std::pair<iterator, iterator> equalRange(const T& item) {
        return internalTree.equal_range(item);
    }

    std::pair<iterator, State> findNext(const T& item) {
        iterator result = internalTree.upper_bound(item);
        State flag = State::exists;

        if (result == internalTree.end()) {
            flag = State::inf_positive;
        }

        return std::make_pair(result, flag);
    }

    std::pair<iterator, State> findPrev(const T& item) {
        iterator result = internalTree.lower_bound(item);
        State flag = State::exists;

        if (result == internalTree.begin()) {
            flag = State::inf_negative;
        } else {
            --result;
        }

        return std::make_pair(result, flag);
    }

private:
    std::set<T, Comparator> internalTree;
};

} // namespace gkernel
#endif /* GKERNEL_RBTREE_HPP */
