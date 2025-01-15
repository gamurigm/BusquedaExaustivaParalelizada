#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h> // Para OpenMP

// Funci�n para buscar combinaciones �nicas que sumen un objetivo
void buscarCombinacionesUnicas(const std::vector<int>& numeros, int objetivo, std::vector<int>& combinacion, int indice) {
    if (objetivo == 0) {
        // Imprimir una combinaci�n v�lida
        std::cout << "{ ";
        for (int num : combinacion) {
            std::cout << num << " ";
        }
        std::cout << "}" << std::endl;
        return;
    }
    if (objetivo < 0 || indice >= numeros.size()) {
        return; // No es posible encontrar m�s combinaciones
    }

    // Explorar con el n�mero actual incluido
    combinacion.push_back(numeros[indice]);
    buscarCombinacionesUnicas(numeros, objetivo - numeros[indice], combinacion, indice + 1);
    combinacion.pop_back();

    // Explorar sin el n�mero actual
    buscarCombinacionesUnicas(numeros, objetivo, combinacion, indice + 1);
}

int main() {
    int tamanioArreglo, objetivo;

    // Entrada por teclado del tama�o del arreglo
    std::cout << "Ingrese el tama�o del arreglo de n�meros: ";
    std::cin >> tamanioArreglo;

    // Generar un vector de n�meros del 1 al tama�o especificado
    std::vector<int> numeros;
    for (int i = 1; i <= tamanioArreglo; ++i) {
        numeros.push_back(i);
    }

    // Entrada por teclado del objetivo
    std::cout << "Ingrese el objetivo que desea alcanzar: ";
    std::cin >> objetivo;

    // Medir el tiempo de inicio
    auto inicio = std::chrono::high_resolution_clock::now();

    std::cout << "Buscando combinaciones �nicas que sumen " << objetivo << ":" << std::endl;

    // Paralelizar las b�squedas iniciales
#pragma omp parallel
    {
#pragma omp for
        for (int i = 0; i < numeros.size(); ++i) {
            std::vector<int> combinacion; // Cada hilo tiene su propia combinaci�n
            buscarCombinacionesUnicas(numeros, objetivo - numeros[i], combinacion, i + 1);
        }
    }

    // Medir el tiempo de finalizaci�n
    auto fin = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

    // Mostrar el tiempo de ejecuci�n
    std::cout << "\nTiempo de ejecuci�n: " << duracion.count() << " milisegundos" << std::endl;

    return 0;
}
