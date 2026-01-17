#include <iostream>
#include <vector>
#include <ctime>
#include <thread>
#include <chrono>
#include <mutex>
#include <algorithm>
#include "buffered_channel.h"

struct dot {
	size_t x = 0;
	size_t y = 0;
};
struct BlockTask {
	size_t x1, y1, x2, y2;
	size_t x3, y3, x4, y4;
	size_t result_row, result_col;
};

void print_matrix(std::vector <std::vector <int>>& M) {
	for (size_t i = 0; i < M.size(); i++) {
		for (size_t j = 0; j < M[0].size(); j++) {
			std::cout << M[i][j] << " ";
		}
		std::cout << '\n';
	}
}

void block_mult(const std::vector <std::vector <int>>& first, const std::vector <std::vector <int>>& second, 
	size_t x1, size_t y1, size_t x2, size_t y2, size_t x3, size_t y3, size_t x4, size_t y4,
	std::vector <std::vector <int>>& result, std::mutex& m) {
	/*if (y2 - y1 != x4 - x3) {
		throw std::exception("invalid pair");
	}*/
	std::vector <std::vector <int>> temp(x2 - x1 + 1, std::vector <int>(y4 - y3 + 1));
	for (size_t i = 0; i <= x2 - x1; i++) {
		for (size_t j = 0; j <= y4 - y3; j++) {
			int x = 0;
			for (size_t k = 0; k <= y2 - y1; k++) {
				x += first[x1 + i][y1 + k] * second[x3 + k][y3 + j];
			}
			temp[i][j] += x;
		}
	}
	std::lock_guard<std::mutex> lock(m);
	for (size_t i = x1; i <= x2; i++) {
		for (size_t j = y3; j <= y4; j++) {
			result[i][j] += temp[i - x1][j - y3];
		}
	}
}

void worker(BufferedChannel<BlockTask>& tasks,
	const std::vector<std::vector<int>>& first,
	const std::vector<std::vector<int>>& second,
	std::vector<std::vector<int>>& result,
	std::mutex& result_mutex) {
	while (true) {
		auto [task, ok] = tasks.Recv();
		if (!ok) break;

		block_mult(first, second,
			task.x1, task.y1, task.x2, task.y2,
			task.x3, task.y3, task.x4, task.y4,
			result, result_mutex);
	}
}

auto mult_threading(std::vector <std::vector <int>>& first, std::vector <std::vector <int>>& second,
	size_t block_size, std::vector <std::vector <int>>& result) {
	
	dot first_left_corner;
	dot first_right_corner;
	dot second_left_corner;
	dot second_right_corner;
	size_t n = first.size();
	size_t block_count = std::ceil(static_cast<double>(n) / static_cast<double>(block_size));
	size_t num_workers = std::thread::hardware_concurrency();
	BufferedChannel<BlockTask> tasks(static_cast<size_t>(std::thread::hardware_concurrency()) * 4);
	std::mutex result_mutex;
	std::vector<std::thread> workers;
	for (size_t i = 0; i < num_workers; ++i) {
		workers.emplace_back(worker, std::ref(tasks),
			std::cref(first), std::cref(second),
			std::ref(result), std::ref(result_mutex));
	}

	auto start = std::chrono::high_resolution_clock::now();

	first_left_corner.x = 0;
	first_right_corner.x = block_size - 1;
	while (true) { //переход к след строке левой матрицы
		first_left_corner.y = 0;
		first_right_corner.y = block_size - 1; //новая строка - возвращаем блок в начало
		while (true) { //проход по строке блоков левой матрицы
			second_left_corner.x = first_left_corner.y;
			second_right_corner.x = second_left_corner.x + block_size - 1;
			if (second_right_corner.x >= n) {
				second_right_corner.x = n - 1;
			}
			second_left_corner.y = 0;
			second_right_corner.y = block_size - 1;
			while (true) { //перемножение блока из левой матрицы на все требуемые из правой
				BlockTask task{
					first_left_corner.x, first_left_corner.y,
					first_right_corner.x, first_right_corner.y,
					second_left_corner.x, second_left_corner.y,
					second_right_corner.x, second_right_corner.y,
					first_left_corner.x / block_size, second_left_corner.y / block_size };
				tasks.Send(task);
				second_left_corner.y += block_size;
				second_right_corner.y += block_size;
				if (second_right_corner.y >= n) {
					if (second_left_corner.y < n) { //обрезанный блок
						second_right_corner.y = n - 1;
					}
					else {
						break; //прошли всю строку правой матрицы
					}
				}
			}
			first_left_corner.y += block_size;
			first_right_corner.y += block_size;
			if (first_right_corner.y >= n) {
				if (first_left_corner.y < n) { //обрезанный блок
					first_right_corner.y = n - 1;
				}
				else {
					break;
				}
			}

		}
		first_left_corner.x += block_size;
		first_right_corner.x += block_size;
		if (first_right_corner.x >= n) {
			if (first_left_corner.x < n) { //обрезанная строка
				first_right_corner.x = n - 1;
				//тут нужно пройти заново весь цикл
			}
			else {
				break;
			}
		}
	}

	tasks.Close();
	for (auto& t : workers) {
		t.join();
	}

	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
}

void test_matrix_size_of(size_t n, bool print_all_k, size_t start_k) {
	const int int_floor = 1000;
	std::srand(std::time(0));
	std::vector <std::vector <int>> A(n, std::vector <int>(n));
	std::vector <std::vector <int>> B(n, std::vector <int>(n));
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			A[i][j] = std::rand() % int_floor;
			B[i][j] = std::rand() % int_floor;
		}
	}
	std::cout << "n: " << n << '\n';

	//без потоков
	std::cout << "no threads:\n";
	auto start = std::chrono::high_resolution_clock::now();
	std::vector <std::vector <int>> result(n, std::vector <int>(n));
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			for (size_t k = 0; k < n; k++) {
				result[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	auto end = std::chrono::high_resolution_clock::now();
	auto no_thread_time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
	std::cout << no_thread_time << '\n';

	//потоки
	std::cout << "---------------\nmultithreading:\n";
	std::vector<std::chrono::nanoseconds> data;
	for (size_t k = start_k; k <= n; k++) {
		std::vector <std::vector <int>> C(n, std::vector <int>(n));
		data.push_back(mult_threading(A, B, k, C));
		if (print_all_k) {
			size_t block_count = std::ceil(static_cast<double>(n) / static_cast<double>(k));
			std::cout << "k = " << k << ", thread_count = " << block_count * block_count * block_count << " -> " << data.back() << "\n";
			//std::cout << k << "," << block_count << "," << data.back().count() << "\n";
		}
		if (C != result) {
			std::cout << "^error!!\n";
		}
	}

	//анализ
	auto it = std::min_element(data.begin(), data.end());
	if (it != data.end()) {
		std::size_t pos = std::distance(data.begin(), it);
		std::cout << "Минимум = " << *it << ", при k = " << pos + 1 << "\n";
		std::cout << "что в " << std::chrono::duration<double>(no_thread_time) / std::chrono::duration<double>(*it) << " быстрее чем без потоков";
	}
}

int main()
{
	std::cout << "размер изначальных матриц n: ";
	size_t n;
	std::cin >> n;
	std::cout << "вывести для время для каждого k?\n1 - да, 0 - нет\n";
	bool a;
	std::cin >> a;
	std::cout << "начиная с k: ";
	size_t k;
	std::cin >> k;
	test_matrix_size_of(n, a, k);
}