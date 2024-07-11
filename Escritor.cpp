#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

class RWMonitor {
public:
    RWMonitor() : readers(0), writer_active(false) {}

    void start_read() {
        unique_lock<mutex> lock(mtx);
        delay(read_cv, [this]() { return !writer_active; });
        readers++;
        resume(read_cv);
    }

    void end_read() {
        unique_lock<mutex> lock(mtx);
        readers--;
        if (readers == 0) {
            writer_cv.notify_one();
        }
    }

    void start_write() {
        unique_lock<mutex> lock(mtx);
        delay(writer_cv, [this]() { return !writer_active && readers == 0; });
        writer_active = true;
    }

    void end_write() {
        unique_lock<mutex> lock(mtx);
        writer_active = false;
        read_cv.notify_all();
        writer_cv.notify_one();
    }

private:
    mutex mtx;
    condition_variable read_cv;
    condition_variable writer_cv;
    int readers;
    bool writer_active;

    void delay(condition_variable &cv, function<bool()> pred) {
        cv.wait(mtx, pred);
    }

    void resume(condition_variable &cv) {
        cv.notify_all();
    }
};

// Función de ejemplo para el lector
void lector(RWMonitor &monitor, int id) {
    monitor.start_read();
    cout << "Lector " << id << " está leyendo" << endl;
    this_thread::sleep_for(chrono::milliseconds(100)); // Simula el tiempo de lectura
    cout << "Lector " << id << " terminó de leer" << endl;
    monitor.end_read();
}

// Función de ejemplo para el escritor
void escritor(RWMonitor &monitor, int id) {
    monitor.start_write();
    cout << "Escritor " << id << " está escribiendo" << endl;
    this_thread::sleep_for(chrono::milliseconds(100)); // Simula el tiempo de escritura
    cout << "Escritor " << id << " terminó de escribir" << endl;
    monitor.end_write();
}

int main() {
    RWMonitor monitor;

    thread lectores[5];
    thread escritores[3];

    // Crea y lanza los hilos lectores
    for (int i = 0; i < 5; ++i) {
        lectores[i] = thread(lector, ref(monitor), i + 1);
    }

    // Crea y lanza los hilos escritores
    for (int i = 0; i < 3; ++i) {
        escritores[i] = thread(escritor, ref(monitor), i + 1);
    }

    // Espera a que todos los hilos terminen
    for (int i = 0; i < 5; ++i) {
        lectores[i].join();
    }
    for (int i = 0; i < 3; ++i) {
        escritores[i].join();
    }

    return 0;
}
