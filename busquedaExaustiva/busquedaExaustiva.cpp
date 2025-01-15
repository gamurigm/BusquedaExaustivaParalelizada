#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h> // Para OpenMP

// Función para buscar combinaciones únicas que sumen un objetivo
void buscarCombinacionesUnicas(const std::vector<int>& numeros, int objetivo, std::vector<int>& combinacion, int indice) {
    if (objetivo == 0) {
        // Imprimir una combinación válida
        std::cout << "{ ";
        for (int num : combinacion) {
            std::cout << num << " ";
        }
        std::cout << "}" << std::endl;
        return;
    }
    if (objetivo < 0 || indice >= numeros.size()) {
        return; // No es posible encontrar más combinaciones
    }

    // Explorar con el número actual incluido
    combinacion.push_back(numeros[indice]);
    buscarCombinacionesUnicas(numeros, objetivo - numeros[indice], combinacion, indice + 1);
    combinacion.pop_back();

    // Explorar sin el número actual
    buscarCombinacionesUnicas(numeros, objetivo, combinacion, indice + 1);
}

int main() {
    int tamanioArreglo, objetivo;

    // Entrada por teclado del tamaño del arreglo
    std::cout << "Ingrese el tamaño del arreglo de números: ";
    std::cin >> tamanioArreglo;

    // Generar un vector de números del 1 al tamaño especificado
    std::vector<int> numeros;
    for (int i = 1; i <= tamanioArreglo; ++i) {
        numeros.push_back(i);
    }

    // Entrada por teclado del objetivo
    std::cout << "Ingrese el objetivo que desea alcanzar: ";
    std::cin >> objetivo;

    // Medir el tiempo de inicio
    auto inicio = std::chrono::high_resolution_clock::now();

    std::cout << "Buscando combinaciones únicas que sumen " << objetivo << ":" << std::endl;

    // Paralelizar las búsquedas iniciales
#pragma omp parallel
    {
#pragma omp for
        for (int i = 0; i < numeros.size(); ++i) {
            std::vector<int> combinacion; // Cada hilo tiene su propia combinación
            buscarCombinacionesUnicas(numeros, objetivo - numeros[i], combinacion, i + 1);
        }
    }

    // Medir el tiempo de finalización
    auto fin = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

    // Mostrar el tiempo de ejecución
    std::cout << "\nTiempo de ejecución: " << duracion.count() << " milisegundos" << std::endl;

    return 0;
}
