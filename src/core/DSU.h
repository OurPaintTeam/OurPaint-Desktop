#ifndef OURPAINT_APPLICATION_DSU_H_
#define OURPAINT_APPLICATION_DSU_H_

#include <unordered_map>
#include <utility>

template <typename T>
class DSU {
private:
    std::unordered_map<T, T> parent_;
    std::unordered_map<T, std::size_t> size_;

public:
    void makeSet(const T& value) {
        if (parent_.contains(value)) {
            return;
        }

        parent_[value] = value;
        size_[value] = 1;
    }

    T find(const T& value) {
        makeSet(value);

        T root = value;
        while (root != parent_[root]) {
            root = parent_[root];
        }

        T current = value;
        while (current != root) {
            T next = parent_[current];
            parent_[current] = root;
            current = next;
        }

        return root;
    }

    void unite(const T& a, const T& b) {
        T rootA = find(a);
        T rootB = find(b);

        if (rootA == rootB) {
            return;
        }

        if (size_[rootA] < size_[rootB]) {
            std::swap(rootA, rootB);
        }

        parent_[rootB] = rootA;
        size_[rootA] += size_[rootB];
    }

    bool same(const T& a, const T& b) {
        return find(a) == find(b);
    }

    std::size_t size(const T& value) {
        return size_[find(value)];
    }
};

#endif