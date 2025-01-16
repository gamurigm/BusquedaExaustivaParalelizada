#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>
#include <atomic>
#include <mutex>
#include <memory>
#include <thread>

// Estructura para almacenar resultados de manera thread-safe
struct ResultadoCombinacion {
    std::mutex mtx;
    void agregar(const std::vector<int>& comb) {
        std::lock_guard<std::mutex> lock(mtx);
        
        // Automáticamente bloquea el mutex al construirse 
        // y desbloquea el mutex al destruirse (cuando termina el scope)
      
        std::cout << "{ ";
        for (int num : comb) {    // Todo este bloque está protegido por el mutex
            std::cout << num << " ";
        }
        std::cout << "}" << std::endl;
    }// Evita que las salidas de diferentes hilos se mezclen
};

class BuscadorCombinaciones {
private:
    std::shared_ptr<ResultadoCombinacion> resultados;
    
    // Permite compartir de manera segura el objeto ResultadoCombinacion entre hilos
    // Libera automáticamente la memoria cuando ya no se necesita

    const std::vector<int>& numeros; //Evita copias innecesarias
    std::atomic<size_t> combinacionesEncontradas{ 0 }; // Contador atómico para las combinaciones encontradas
    const size_t CHUNK_SIZE = 1000; //Constante para optimizar el balanceo de carga

    void buscarCombinacionesRecursivo(int objetivo, std::vector<int>& combinacion, int indice) {
        if (objetivo == 0) {
            resultados->agregar(combinacion);
            combinacionesEncontradas++;
            return;
        }
        if (objetivo < 0 || indice >= numeros.size()) {
            return;
        }
        if (objetivo > 0) {
            int sumaMinima = numeros[indice];
            if (sumaMinima > objetivo) {
                return;
            }
        }
        if (combinacion.capacity() < numeros.size()) { // Reserva memoria anticipadamente
            combinacion.reserve(numeros.size());       // Evita realocaciones costosas durante la recursión
        }
        combinacion.push_back(numeros[indice]);
        buscarCombinacionesRecursivo(objetivo - numeros[indice], combinacion, indice + 1);
        combinacion.pop_back();
        buscarCombinacionesRecursivo(objetivo, combinacion, indice + 1);
    }

public:
    BuscadorCombinaciones(const std::vector<int>& nums)
        : numeros(nums), resultados(std::make_shared<ResultadoCombinacion>()) {
    }

    void buscar(int objetivo, int num_threads) {
        omp_set_num_threads(num_threads);
        omp_set_nested(1);
        omp_set_dynamic(0);

#pragma omp parallel
        {
            std::vector<int> combinacionLocal;
            combinacionLocal.reserve(numeros.size());
#pragma omp for schedule(dynamic, CHUNK_SIZE)
            for (int i = 0; i < numeros.size(); ++i) {
                combinacionLocal.clear();
                combinacionLocal.push_back(numeros[i]);
                buscarCombinacionesRecursivo(objetivo - numeros[i], combinacionLocal, i + 1);
            }
        }
    }

    size_t obtenerTotalCombinaciones() const {
        return combinacionesEncontradas;
    }
};

// Función para realizar mediciones múltiples
std::vector<double> realizarMediciones(BuscadorCombinaciones& buscador, int objetivo, int num_threads, int num_mediciones) {
    std::vector<double> tiempos;
    tiempos.reserve(num_mediciones);

    for (int i = 0; i < num_mediciones; ++i) {
        auto inicio = std::chrono::high_resolution_clock::now();
        buscador.buscar(objetivo, num_threads);
        auto fin = std::chrono::high_resolution_clock::now();
        auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);
        tiempos.push_back(duracion.count());
    }

    return tiempos;
}

// Función para calcular estadísticas
void mostrarEstadisticas(const std::vector<double>& tiempos) {
    double suma = 0;
    double min = tiempos[0];
    double max = tiempos[0];

    for (double tiempo : tiempos) {
        suma += tiempo;
        if (tiempo < min) min = tiempo;
        if (tiempo > max) max = tiempo;
    }

    double promedio = suma / tiempos.size();
    std::cout << "Estadísticas de tiempo (ms):" << std::endl;
    std::cout << "Mínimo: " << min << std::endl;
    std::cout << "Máximo: " << max << std::endl;
    std::cout << "Promedio: " << promedio << std::endl;
}

int main() {
    int tamanioArreglo, objetivo;
    std::cout << "Ingrese el tamaño del arreglo de números: ";
    std::cin >> tamanioArreglo;

    std::vector<int> numeros;
    numeros.reserve(tamanioArreglo);
    for (int i = 1; i <= tamanioArreglo; ++i) {
        numeros.push_back(i);
    }

    std::cout << "Ingrese el objetivo que desea alcanzar: ";
    std::cin >> objetivo;

    // Obtener número de núcleos disponibles
    int max_threads = std::thread::hardware_concurrency();
    int opcion;
    std::cout << "\nSeleccione el modo de ejecución:" << std::endl;
    std::cout << "1. Usar todos los núcleos (" << max_threads << " núcleos)" << std::endl;
    std::cout << "2. Usar la mitad de los núcleos (" << max_threads / 2 << " núcleos)" << std::endl;
    std::cout << "Opción: ";
    std::cin >> opcion;

    int num_threads = (opcion == 1) ? max_threads : max_threads / 2;
    const int NUM_MEDICIONES = 50;

    BuscadorCombinaciones buscador(numeros);

    std::cout << "\nRealizando " << NUM_MEDICIONES << " mediciones con " << num_threads << " núcleos..." << std::endl;
    auto tiempos = realizarMediciones(buscador, objetivo, num_threads, NUM_MEDICIONES);

    std::cout << "\nResultados finales:" << std::endl;
    std::cout << "Combinaciones encontradas: " << buscador.obtenerTotalCombinaciones() << std::endl;
    mostrarEstadisticas(tiempos);

    return 0;
}