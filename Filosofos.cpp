#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

class Filosofos {
public:
    Filosofos(int num_philosophers) : num_philosophers(num_philosophers) {
        states = new State[num_philosophers];
        cv = new condition_variable[num_philosophers];
        for (int i = 0; i < num_philosophers; ++i) {
            states[i] = THINKING;
        }
    }

    ~Filosofos() {
        delete[] states;
        delete[] cv;
    }

    void pickup_forks(int id) {
        unique_lock<mutex> lock(mtx);
        states[id] = HUNGRY;
        delay(cv[id], [this, id]() { return can_eat(id); });
        states[id] = EATING;
    }

    void putdown_forks(int id) {
        unique_lock<mutex> lock(mtx);
        states[id] = THINKING;
        resume(cv[left(id)]);
        resume(cv[right(id)]);
    }

private:
    enum State { THINKING, HUNGRY, EATING };

    int num_philosophers;
    State* states;
    mutex mtx;
    condition_variable* cv;

    int left(int id) { return (id + num_philosophers - 1) % num_philosophers; }
    int right(int id) { return (id + 1) % num_philosophers; }

    bool can_eat(int id) {
        return states[id] == HUNGRY &&
               states[left(id)] != EATING &&
               states[right(id)] != EATING;
    }

    void delay(condition_variable &cv, function<bool()> pred) {
        cv.wait(mtx, pred);
    }

    void resume(condition_variable &cv) {
        cv.notify_one();
    }
};

// Función de ejemplo para el filósofo
void Filosofo(Filosofos &table, int id) {
    while (true) {
        cout << "Filósofo " << id << " está pensando" << endl;
        this_thread::sleep_for(chrono::milliseconds(1000)); // Simula el tiempo de pensamiento

        table.pickup_forks(id);
        cout << "Filósofo " << id << " está comiendo" << endl;
        this_thread::sleep_for(chrono::milliseconds(1000)); // Simula el tiempo de comida

        table.putdown_forks(id);
    }
}

int main() {
    const int num_philosophers = 5;
    Filosofos table(num_philosophers);

    thread philosophers[num_philosophers];
    for (int i = 0; i < num_philosophers; ++i) {
        philosophers[i] = thread(Filosofo, ref(table), i);
    }

    for (int i = 0; i < num_philosophers; ++i) {
        philosophers[i].join();
    }

    return 0;
}
