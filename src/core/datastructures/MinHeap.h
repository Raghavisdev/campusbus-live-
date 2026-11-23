#ifndef MINHEAP_H
#define MINHEAP_H

#include <string>
#include <utility>
#include <vector>

using namespace std;

// One heap entry: a priority (minutes) and the stop it belongs to
struct HeapItem
{
    double key;
    string value;
};

// Binary min-heap stored in a vector: the smallest key is always at index 0.
// For the item at index i:  parent = (i - 1) / 2,  children = 2i + 1 and 2i + 2.
class MinHeap
{
private:

    vector<HeapItem> items;

    // Move the item at index i up while it is smaller than its parent (used after push)
    void siftUp(size_t i)
    {
        while (i > 0)
        {
            size_t parent = (i - 1) / 2;

            if (items[parent].key <= items[i].key)
            {
                break;
            }

            swap(items[parent], items[i]);
            i = parent;
        }
    }

    // Min-heapify: move the item at index i down until neither child is smaller (used after pop)
    void heapify(size_t i)
    {
        size_t n = items.size();

        while (true)
        {
            size_t left = 2 * i + 1;
            size_t right = 2 * i + 2;
            size_t smallest = i;

            if (left < n && items[left].key < items[smallest].key)
            {
                smallest = left;
            }

            if (right < n && items[right].key < items[smallest].key)
            {
                smallest = right;
            }

            if (smallest == i)
            {
                break;
            }

            swap(items[i], items[smallest]);
            i = smallest;
        }
    }

public:

    bool empty() const
    {
        return items.empty();
    }

    size_t size() const
    {
        return items.size();
    }

    // Add an item: append at the end, then sift it up.  O(log n)
    void push(double key, string value)
    {
        HeapItem item;
        item.key = key;
        item.value = value;

        items.push_back(item);
        siftUp(items.size() - 1);
    }

    // Remove and return the smallest item: move the last item to the root, then heapify.  O(log n)
    // The caller must check empty() first.
    HeapItem pop()
    {
        HeapItem smallest = items[0];

        items[0] = items.back();
        items.pop_back();

        if (!items.empty())
        {
            heapify(0);
        }

        return smallest;
    }
};

#endif
