# Una Familia de la Mafia - Sistema de Sucesión

Este proyecto implementa el sistema de sucesión para una familia de la mafia italiana. Utiliza una estructura de Árbol Binario y una Lista Enlazada propia para el manejo principal de los datos. La interfaz es controlada mediante el motor gráfico en consola GuayabaConsoleEngine.

## Estructura del Proyecto
- `bin/`: Contiene el archivo ejecutable `mafia_system` y los datos principales como `test_data.csv`.
- `src/`: Contiene el código fuente organizado en carpetas:
  - `csvReader/`: Carga y guardado de los datos de la familia desde/hacia archivos `.csv`.
  - `mafiaFamily/`: Lógica central del negocio (construcción del árbol `MafiaTree` y algoritmos de sucesión).
  - `scenes/`: Componentes de UI construidos con GuayabaConsoleEngine.
  - `external/`: Archivos base del motor y utilidades (`CGINDE.hpp`, `BinaryTree`).

## Compilación y Ejecución
### Windows
Se han añadido varias herramientas para facilitar la compilación en Windows:
*   **Batch**: Ejecuta `compile.bat` para una compilación rápida con `g++`.
*   **VS Code**: Presiona `Ctrl+Shift+B` para compilar automáticamente usando la tarea configurada.
*   **CMake**: Si tienes CMake instalado, puedes usar `cmake -B build` y `cmake --build build`.

### Linux
Se incluye un `Makefile` para facilitar la construcción:
*   **Para compilar**: Ejecuta el comando `make` en la raíz del proyecto.
*   **Para ejecutar**: Usa `make run` o `./bin/mafia_system`.
*   **Limpiar binarios**: `make clean`.

## Interfaz Gráfica
La interfaz gráfica en consola ofrece 4 opciones principales:
1. **Ver línea de sucesión**: Muestra en la terminal los miembros de la familia ordenados mediante un pre-order iterando únicamente los que se encuentran vivos y en libertad.
2. **Modificar miembro**: Permite cambiar atributos del miembro de la familia buscando por su `id`. Al hacerlo, si el miembro es el jefe actual y muere, es encarcelado, o es mayor o igual a 70 años, se activará la cadena de sucesión automática.
3. **Guardar cambios**: Escribe los datos nuevamente al archivo `csv`, conservando los cambios hechos en tiempo de ejecución.
4. **Salir**: Cierra el motor y la aplicación de consola.

## Requisitos y Observaciones Satisfechas
- **Árbol Binario**: La jerarquía se construye con una estructura de un padre y máximo dos sucesores.
- **Línea de Sucesión actual**: Implementada y mostrando solo miembros disponibles (vivos).
- **Sucesión Automática**: El algoritmo completo con las 6 reglas para buscar y promover dentro del árbol se encuentra en el método `evaluateSuccession()` de la clase `MafiaTree`.
- **Modificación y Guardado**: Integrada, a su vez se puede persistir la información escribiendo al CSV.
- **Carpeta de binarios e src**: Respetada mediante el `Makefile`.
- **Restricción de Vectores**: Las estructuras de datos principales de listas (como obtener la lista de los sucesores) han sido refactorizadas y manejan `cde::LinkedList` para cumplir con las reglas en todo el flujo local.