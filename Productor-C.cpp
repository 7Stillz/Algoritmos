#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>

using namespace std;

class Buffer {
public:
    Buffer(int size) : N(size), tam(0), frente(0), final(0) {
        cola = new int[N];
    }

    ~Buffer() {
        delete[] cola;
    }

    void insertar(int elemento) {
        unique_lock<mutex> lock(mtx);
        clleno.wait(lock, [this]() { return tam < N; });

        cola[final] = elemento;
        final = (final + 1) % N;
        tam++;
        cvacio.notify_one();
    }

    void extraer(int &elemento) {
        unique_lock<mutex> lock(mtx);
        cvacio.wait(lock, [this]() { return tam > 0; });

        elemento = cola[frente];
        frente = (frente + 1) % N;
        tam--;
        clleno.notify_one();
    }

private:
    const int N;
    int tam;
    int frente;
    int final;
    int* cola;
    mutex mtx;
    condition_variable clleno;
    condition_variable cvacio;
};

// Función de ejemplo para el productor
void productor(Buffer &buffer, int id, int num_items) {
    for (int i = 0; i < num_items; ++i) {
        buffer.insertar(id * 100 + i);
        cout << "Productor " << id << " inserto " << id * 100 + i << endl;
    }
}

// Función de ejemplo para el consumidor
void consumidor(Buffer &buffer, int id, int num_items) {
    for (int i = 0; i < num_items; ++i) {
        int item;
        buffer.extraer(item);
        cout << "Consumidor " << id << " extrajo " << item << endl;
    }
}

int main() {
    const int buffer_size = 10;
    const int num_items = 20;

    Buffer buffer(buffer_size);

    thread prod1(productor, ref(buffer), 1, num_items);
    thread cons1(consumidor, ref(buffer), 1, num_items);

    prod1.join();
    cons1.join();

    return 0;
}
