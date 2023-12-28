#include "vector"
#include "iostream"
#include "random"
#include <chrono>

class Segment {
public:
    Segment *prev_, *next_;
    int left_, size_, index_heap_;
    bool is_free_;

    // отрезок [left, left+size]
    Segment(Segment *prev, Segment *next, int left, int size, int index_heap, bool is_free) {
        prev_ = prev;
        next_ = next;
        left_ = left;
        size_ = size;
        index_heap_ = index_heap;
        is_free_ = is_free;

        if (next) {
            next->prev_ = this;
        }
        if (prev) {
            prev->next_ = this;
        }
    }

    void Remove() {
        if (prev_) {
            prev_->next_ = this->next_;
        }
        if (next_) {
            next_->prev_ = this->prev_;
        }
    }
};

class MemoryManager {
    int heap_length_ = 0;
    std::vector<Segment *> heap_;


public:
    std::vector<Segment *> queries_;
    std::vector<Segment *> deleted;

    explicit MemoryManager(Segment *initial) {
        heap_length_++;
        heap_.push_back(initial);
    }

    int Allocate(int size_alloc) {
        // нет свободных отрезков или у макс. размер меньше
        if (!heap_length_ || heap_[0]->size_ < size_alloc) {
            queries_.push_back(nullptr);
            return -1;
        }
        Segment *max = heap_[0];
        auto *occupied = new Segment(max->prev_, max, max->left_, size_alloc, -1, false);
        deleted.push_back(occupied);
        queries_.push_back(occupied);
        max->left_ += size_alloc;
        max->size_ -= size_alloc;
        if (max->size_ == 0) {
            HeapRemove(0);
            if (max->next_ == nullptr) {
                occupied->next_ = nullptr;
            }
            max->Remove();
        } else {
            Heapify(max->index_heap_);
        }
        return occupied->left_ + 1;
    }

    void FreeMem(int index) {
        queries_.push_back(nullptr);
        auto *occupied = queries_[index];
        if (occupied == nullptr) {
            return;
        }
        queries_[index] = nullptr;
        bool is_right_free = occupied->next_ && occupied->next_->is_free_;
        bool is_left_free = occupied->prev_ && occupied->prev_->is_free_;
        if (!is_left_free && !is_right_free) {
            occupied->is_free_ = true;
            HeapAdd(occupied);
            return;
        }
        if (is_right_free && is_left_free) {
            auto *next = occupied->next_;
            auto *prev = occupied->prev_;
            prev->size_ += occupied->size_;
            prev->size_ += next->size_;
            occupied->Remove();
            HeapRemove(next->index_heap_);
            HeapUp(prev->index_heap_);
            next->Remove();

            return;
        }
        if (is_right_free) {
            occupied->next_->size_ += occupied->size_;
            occupied->next_->left_ = occupied->left_;
            HeapUp(occupied->next_->index_heap_);
            occupied->Remove();
            return;
        }
        if (is_left_free) {
            occupied->prev_->size_ += occupied->size_;
            HeapUp(occupied->prev_->index_heap_);
            occupied->Remove();
            return;
        }
    }

    void Swap(int aa, int bb) {
        Segment *tmp = heap_[aa];
        heap_[aa] = heap_[bb];
        heap_[bb] = tmp;

        heap_[aa]->index_heap_ = aa;
        heap_[bb]->index_heap_ = bb;
    }

    static bool IsLess(Segment *aa, Segment *bb) {
        if (aa->size_ < bb->size_) {
            return true;
        }
        if (aa->size_ == bb->size_ && aa->left_ > bb->left_) {
            return true;
        }
        return false;
    }

    void Heapify(int index) {
        while (true) {
            int new_ind = index;
            int left_neib = index * 2 + 1;
            int right_neib = index * 2 + 2;
            if (left_neib < heap_length_ && IsLess(heap_[new_ind], heap_[left_neib])) {
                new_ind = left_neib;
            }
            if (right_neib < heap_length_ && IsLess(heap_[new_ind], heap_[right_neib])) {
                new_ind = right_neib;
            }
            if (new_ind == index) {
                return;
            }
            Swap(index, new_ind);
            index = new_ind;
        }
    }

    void HeapUp(int index) {
        while (index) {
            if (!IsLess(heap_[index], heap_[(index - 1) / 2])) {
                Swap(index, (index - 1) / 2);
                index = (index - 1) / 2;
            } else {
                return;
            }
        }
    }

    void HeapAdd(Segment *new_seg) {
        new_seg->index_heap_ = heap_length_;
        heap_.push_back(new_seg);
        HeapUp(heap_length_);
        heap_length_++;
    }

    void HeapRemove(int index) {
        if (heap_length_) {
            Swap(index, heap_length_ - 1);
            heap_.pop_back();
            heap_length_--;
            if (index < heap_length_) {
                Heapify(index);
                HeapUp(index);
            }
        }
    }
};

std::vector<int> Solve(std::vector<int> &queries, int n_mem) {
    auto *initial = new Segment(nullptr, nullptr, 0, n_mem, 0, true);
    MemoryManager mem_man(initial);
    std::vector<int> res;
    for (int query: queries) {
        if (query > 0) {
            res.push_back(mem_man.Allocate(query));
        }
        if (query < 0) {
            mem_man.FreeMem(-query - 1);
        }
    }
    delete initial;
    for (auto el: mem_man.deleted) {
        delete el;
    }
    return res;
}

void StressTest() {
    std::mt19937 generator(72874);
    int from = -10000;
    int to = 10000;
    int i = 0;
    while (true) {
        if (i == 5000) {
            return;
        }
        i += 1;

        //равномерное распределение от 0 до 1000
        std::uniform_int_distribution<int> dist(from, to);
        int memory_amount = std::abs(dist(generator)) % 10000;
        if (memory_amount == 0) {
            memory_amount++;
        }
        int requests_amount = std::abs(dist(generator)) % 10000;

        std::vector<int> requests;
        for (int j = 0; j < requests_amount; ++j) {
            int random_num = dist(generator) % memory_amount;
            //запрос на освобождение памяти
            if (random_num < 0 && !requests.empty()) {
                random_num = random_num % (static_cast<int>(requests.size()));
            } else {
                //на выделение
                random_num = std::abs(random_num);
            }
            if (random_num == 0) { random_num++; }
            requests.push_back(random_num);
        }
        std::vector<int> fast = Solve(requests, memory_amount);

    }
}

int main() {
    std::chrono::high_resolution_clock::time_point start, finish;
    start = std::chrono::high_resolution_clock::now();

    StressTest();
    finish = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
    return 0;
};