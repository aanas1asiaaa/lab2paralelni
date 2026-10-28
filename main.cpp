#include <iostream>
#include <thread>
#include <Windows.h>
#include <mutex>
#include <condition_variable>
#include <future>
#include <queue>
#include <deque>
#include <cmath>
#include <chrono>
#include <vector>

using namespace std;

// Універсальний предикат для перевірки на простоту числа
bool checkPrime(int value) {
    if (value <= 2) return value == 2;
    if (value % 2 == 0) return false;
    for (int d = 3; d * d <= value; d += 2) {
        if (value % d == 0) return false;
    }
    return true;
}

// Ефективніший пошук k-го простого числа
int computePrimeByIndex(int idx) {
    if (idx <= 0) return 0;
    int found_cnt = 0;
    int current_num = 1;
    while (found_cnt < idx) {
        current_num++;
        if (checkPrime(current_num)) {
            found_cnt++;
        }
    }
    return current_num;
}


// Завдання 1.2.1
queue<int> queue_a;
mutex mtx_a;
bool ready_flag_a = false;

void producerFunc_121() {
    int val;
    cout << "Введіть числа (0 для завершення введення):\n";
    while (cin >> val && val != 0) {
        lock_guard<mutex> guard(mtx_a);
        queue_a.push(val);
    }
    lock_guard<mutex> guard(mtx_a);
    ready_flag_a = true;
}

void consumerFunc_121() {
    unique_lock<mutex> ulk(mtx_a, defer_lock);
    while (true) {
        ulk.lock();
        if (ready_flag_a) {
            cout << "\nЗнайдені прості елементи: ";
            while (!queue_a.empty()) {
                int item = queue_a.front();
                queue_a.pop();
                if (checkPrime(item)) {
                    cout << item << " ";
                }
            }
            cout << "\n";
            ulk.unlock();
            break;
        }
        ulk.unlock();
        this_thread::sleep_for(chrono::milliseconds(80));
    }
}

void task_1_2_1() {
    cout << "--- Завдання 1.2.1 (Модифіковане) ---\n";
    thread th1(producerFunc_121);
    th1.detach();
    thread th2(consumerFunc_121);
    th2.join();
}


// Завдання 1.2.2
condition_variable cv_b;
mutex mtx_b;
int state_b = 0;

void waitingWorker_122(int thread_id) {
    cout << "Потік [" << thread_id << "] перейшов у режим очікування.\n";
    unique_lock<mutex> lock(mtx_b);
    cv_b.wait(lock, [] { return state_b == 42; });
    cout << "Потік [" << thread_id << "] розблоковано та продовжує роботу.\n";
}

void signalingWorker_122() {
    this_thread::sleep_for(chrono::milliseconds(90));
    cout << "[Сигнал] Перша розсилка (notify_all) без зміни стану...\n";
    cv_b.notify_all();

    this_thread::sleep_for(chrono::milliseconds(90));
    {
        lock_guard<mutex> lock(mtx_b);
        state_b = 42;
    }
    cout << "[Сигнал] Зміна стану на 42 та друга розсилка (notify_all)...\n";
    cv_b.notify_all();
}

void task_1_2_2() {
    cout << "\n--- Завдання 1.2.2 (Модифіковане) ---\n";
    vector<thread> pool;
    for (int k = 1; k <= 3; ++k) {
        pool.emplace_back(waitingWorker_122, k);
    }
    thread controller(signalingWorker_122);
    controller.join();
    for (auto& th : pool) {
        th.join();
    }
}


// Завдання 1.2.3
condition_variable cv_c;
mutex mtx_c;
bool trigger_c = false;

void targetThread_123(int id) {
    unique_lock<mutex> lock(mtx_c);
    cv_c.wait(lock, [] { return trigger_c; });
    cout << "Отримано сигнал від notify_one у потоці №" << id << "\n";
}

void senderThread_123() {
    this_thread::sleep_for(chrono::milliseconds(110));
    {
        lock_guard<mutex> lock(mtx_c);
        trigger_c = true;
    }
    cout << "Відправляємо єдиний сигнал (notify_one)...\n";
    cv_c.notify_one();
}

void task_1_2_3() {
    cout << "\n--- Завдання 1.2.3 (Модифіковане) ---\n";
    thread t_alpha(targetThread_123, 101);
    thread t_beta(targetThread_123, 102);
    thread t_gamma(targetThread_123, 103);

    t_alpha.detach();
    t_beta.detach();
    t_gamma.detach();

    thread t_snd(senderThread_123);
    t_snd.join();

    this_thread::sleep_for(chrono::milliseconds(60));
}


// Завдання 1.2.4

queue<int> queue_d;
mutex mtx_d;
condition_variable cv_d;
bool finished_d = false;

void producer_124() {
    int number;
    cout << "Введіть числа (0 для кінця введення):\n";
    while (cin >> number && number != 0) {
        lock_guard<mutex> lk(mtx_d);
        queue_d.push(number);
    }
    {
        lock_guard<mutex> lk(mtx_d);
        finished_d = true;
    }
    cv_d.notify_one();
}

void consumer_124() {
    unique_lock<mutex> lk(mtx_d);
    cv_d.wait(lk, [] { return !queue_d.empty() || finished_d; });

    cout << "\nПрості числа (через cv): ";
    while (!queue_d.empty()) {
        int val = queue_d.front();
        queue_d.pop();
        if (checkPrime(val)) {
            cout << val << " ";
        }
    }
    cout << "\n";
}

void task_1_2_4() {
    cout << "\n--- Завдання 1.2.4 (Модифіковане) ---\n";
    thread p_th(producer_124);
    p_th.detach();
    thread c_th(consumer_124);
    c_th.join();
}


// Завдання 1.2.5
void task_1_2_5() {
    cout << "\n--- Завдання 1.2.5 (Модифіковане) ---\n";
    int limit_val;
    cout << "Введіть порядковий номер (n): ";
    cin >> limit_val;

    auto policy = launch::async;

    auto t_start = chrono::steady_clock::now();
    future<int> async_prime = async(policy, computePrimeByIndex, limit_val);

    int user_action;
    cout << "Оберіть математичну операцію (1 - корінь, 2 - синус, 3 - логарифм): ";
    cin >> user_action;

    if (user_action == 1) {
        cout << "sqrt(" << limit_val << ") = " << sqrt(limit_val) << "\n";
    }
    else if (user_action == 2) {
        cout << "sin(" << limit_val << ") = " << sin(limit_val) << "\n";
    }
    else if (user_action == 3) {
        cout << "ln(" << limit_val << ") = " << log(limit_val) << "\n";
    }

    cout << "Очікування завершення асинхронної задачі...\n";
    int outcome = async_prime.get();
    auto t_finish = chrono::steady_clock::now();

    chrono::duration<double> elapsed = t_finish - t_start;

    cout << limit_val << "-те просте число становить: " << outcome << "\n";
    cout << "Витрачений час: " << elapsed.count() << " секунд.\n";
}

// Завдання 1.2.6
deque<packaged_task<int()>> task_queue_e;
deque<int> arg_queue_e;
mutex mtx_e;
condition_variable cv_e;
bool termination_flag = false;

void workerRoutine_126() {
    while (true) {
        packaged_task<int()> active_task;
        int local_n = 0;
        {
            unique_lock<mutex> lk(mtx_e);
            cv_e.wait(lk, [] { return !task_queue_e.empty() || termination_flag; });

            if (termination_flag && task_queue_e.empty()) return;

            active_task = move(task_queue_e.front());
            task_queue_e.pop_front();
            local_n = arg_queue_e.front();
            arg_queue_e.pop_front();
        }
        cout << "[Фоновий потік] Виконується розрахунок для n = " << local_n << "...\n";
        active_task();
    }
}

void task_1_2_6() {
    cout << "\n--- Завдання 1.2.6 (Модифіковане) ---\n";
    thread worker_thread(workerRoutine_126);
    deque<future<int>> fut_list;
    deque<int> num_list;

    cout << "Вводьте числа n (0 для припинення і запуску обробки):\n";
    int val_n;
    while (cin >> val_n && val_n != 0) {
        packaged_task<int()> p_task(bind(computePrimeByIndex, val_n));
        fut_list.push_back(p_task.get_future());
        num_list.push_back(val_n);

        {
            lock_guard<mutex> lk(mtx_e);
            task_queue_e.push_back(move(p_task));
            arg_queue_e.push_back(val_n);
        }
        cv_e.notify_one();
    }

    {
        lock_guard<mutex> lk(mtx_e);
        termination_flag = true;
    }
    cv_e.notify_one();

    for (size_t idx = 0; idx < fut_list.size(); ++idx) {
        cout << "Результат для " << num_list[idx] << "-го: " << fut_list[idx].get() << "\n";
    }

    worker_thread.join();
}


// Завдання 1.2.7

void primaryWorker_127(int val, promise<int> prom_res, promise<bool> prom_sig, promise<int> prom_res10) {
    prom_res.set_value(computePrimeByIndex(val));
    prom_sig.set_value(true);
    prom_res10.set_value(computePrimeByIndex(val * 10));
}

void secondaryWorker_127(int val, future<bool> fut_sig) {
    fut_sig.wait();
    this_thread::sleep_for(chrono::seconds(1));
    cout << "[Потік 2] Квадратний корінь з введеного " << val << " дорівнює: " << sqrt(val) << "\n";
}

void task_1_2_7() {
    cout << "\n--- Завдання 1.2.7 (Модифіковане) ---\n";
    int base_n;
    cout << "Введіть базове значення n: ";
    cin >> base_n;

    promise<int> pr_val;
    promise<bool> pr_signal;
    promise<int> pr_val10;

    future<int> ft_val = pr_val.get_future();
    future<bool> ft_signal = pr_signal.get_future();
    future<int> ft_val10 = pr_val10.get_future();

    thread th_first(primaryWorker_127, base_n, move(pr_val), move(pr_signal), move(pr_val10));
    thread th_second(secondaryWorker_127, base_n, move(ft_signal));

    cout << "Головний потік отримує значення через future:\n";
    cout << base_n << "-те просте число: " << ft_val.get() << "\n";
    cout << (base_n * 10) << "-те просте число: " << ft_val10.get() << "\n";

    th_first.join();
    th_second.join();
}


int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    // Можете раскомментовувати потрібне завдання для перевірки:
    // task_1_2_1();
    // task_1_2_2();
    // task_1_2_3();
    // task_1_2_4();
    // task_1_2_5();
    // task_1_2_6();
    task_1_2_7();

    return 0;
}
