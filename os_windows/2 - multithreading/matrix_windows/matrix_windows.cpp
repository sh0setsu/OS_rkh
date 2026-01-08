#include <iostream>
#include <vector>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <windows.h>

struct dot {
	size_t x = 0;
	size_t y = 0;
};

void print_matrix(std::vector <std::vector <int>>& M) {
	for (size_t i = 0; i < M.size(); i++) {
		for (size_t j = 0; j < M[0].size(); j++) {
			std::cout << M[i][j] << " ";
		}
		std::cout << '\n';
	}
}

void block_mult(std::vector <std::vector <int>>& first, std::vector <std::vector <int>>& second,
	size_t x1, size_t y1, size_t x2, size_t y2, size_t x3, size_t y3, size_t x4, size_t y4,
	std::vector <std::vector <int>>& result, CRITICAL_SECTION& cs) {
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
	EnterCriticalSection(&cs);
	for (size_t i = x1; i <= x2; i++) {
		for (size_t j = y3; j <= y4; j++) {
			result[i][j] += temp[i - x1][j - y3];
		}
	}
	LeaveCriticalSection(&cs);
}

struct ThreadParams {
	std::vector<std::vector<int>>* first;
	std::vector<std::vector<int>>* second;
	size_t x1, y1, x2, y2, x3, y3, x4, y4;
	std::vector<std::vector<int>>* result;
	CRITICAL_SECTION* cs;
};

DWORD WINAPI block_mult_wrapper(LPVOID param) {
	ThreadParams* p = (ThreadParams*)param;
	block_mult(*p->first, *p->second,
		p->x1, p->y1, p->x2, p->y2,
		p->x3, p->y3, p->x4, p->y4,
		*p->result, *p->cs);
	delete p;
	return 0;
}

auto mult_threading(std::vector <std::vector <int>>& first, std::vector <std::vector <int>>& second,
	size_t block_size, std::vector <std::vector <int>>& result) {

	dot first_left_corner;
	dot first_right_corner;
	dot second_left_corner;
	dot second_right_corner;
	size_t n = first.size();
	std::vector<HANDLE> threads;
	size_t block_count = std::ceil(static_cast<double>(n) / static_cast<double>(block_size));
	std::vector<std::vector<CRITICAL_SECTION>> mutexes(block_count);
	for (int i = 0; i < block_count; ++i) {
		mutexes[i].resize(block_count);
		for (int j = 0; j < block_count; ++j) {
			InitializeCriticalSection(&mutexes[i][j]);
		}
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
				ThreadParams* params = new ThreadParams{
					&first, &second,
					first_left_corner.x, first_left_corner.y,
					first_right_corner.x, first_right_corner.y,
					second_left_corner.x, second_left_corner.y,
					second_right_corner.x, second_right_corner.y,
					&result,
					&mutexes[first_left_corner.x / block_size][second_left_corner.y / block_size]
				};
				HANDLE hThread = CreateThread(NULL, 0, block_mult_wrapper, params, 0, NULL);
				threads.push_back(hThread);

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

	if (!threads.empty()) {
		const DWORD MAX_WAIT = MAXIMUM_WAIT_OBJECTS; // 64
		size_t total = threads.size();

		for (size_t i = 0; i < total; i += MAX_WAIT) {
			DWORD count = (DWORD)min(MAX_WAIT, total - i);
			WaitForMultipleObjects(count, &threads[i], TRUE, INFINITE);
		}

		for (auto& h : threads) {
			CloseHandle(h);
		}
	}

	for (int i = 0; i < block_count; ++i) {
		for (int j = 0; j < block_count; ++j) {
			DeleteCriticalSection(&mutexes[i][j]);
		}
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