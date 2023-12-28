#include "vector"
#include "iostream"
#include "random"
#include <chrono>

std::vector<int> SlowSolution(std::vector<int> &queries, int n_mem) {

    std::vector<int> memory(n_mem, 0);
    std::vector<std::pair<int, int>> stat;
    std::vector<int> result;

    for (auto query: queries) {
        if (query > 0) {
            bool is_start_zero = true;
            int max_zeros = 0;
            int zeros = 0, left_bound, temp_left_bound;
            for (int i = 0; i < memory.size(); ++i) {
                if (memory[i] == 0) {
                    zeros++;
                    if (is_start_zero) {
                        temp_left_bound = i;
                        is_start_zero = false;
                    }
                    if (zeros > max_zeros) {
                        max_zeros = zeros;
                        left_bound = temp_left_bound;
                    }

                } else {
                    zeros = 0;
                    is_start_zero = true;
                }
            }
            if (max_zeros < query) {
                stat.emplace_back(-1, -1);
                result.push_back(-1);
            } else {
                result.push_back(left_bound + 1);
                stat.emplace_back(left_bound, query);
                for (int i = left_bound; i < left_bound + query; ++i) {
                    memory[i] = 1;
                }
            }

        }
        if (query < 0) {
            stat.emplace_back(-1, -1);
            std::pair<int, int> *res = &stat[-query - 1];
            if (res->first == -1) {
                continue;
            } else {
                int left = res->first;
                int size = res->second;
                for (int i = left; i < left + size; ++i) {
                    memory[i] = 0;
                }
                res->first = -1;
            }
        }
    }
    return result;
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
        if (memory_amount == 0) memory_amount++;
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
        std::vector<int> slow = SlowSolution(requests, memory_amount);
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