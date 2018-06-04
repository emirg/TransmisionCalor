# Transmisión de Calor

Una ecuación de calor 2-D describe el cambio de temperatura sobre una cierta región, a lo largo del
tiempo, dada la distribución de temperatura inicial y las condiciones de contorno. Se emplea un
esquema de diferencias finitas para resolver numéricamente la ecuación de calor:

* Los elementos de una matriz bidimensional representan la temperatura en cada punto de una
región cuadrada.
* La temperatura inicial es cero en el contorno y alta en el medio.
* La temperatura del contorno se mantiene en cero.
* La simulación avanza paso a paso y se detiene después de un cierto número de pasos.

El cálculo de un elemento depende de los valores del elemento vecino.

Construir un programa serie que resuelva la simulación, utilizando el lenguaje de programación C.
El programa requerirá dos matrices, una para los valores del paso de simulación P y otra matriz para
el paso de simulación P+1. El programa debe soportar matrices de tamaños grandes, limitadas
únicamente por la cantidad de memoria instalada en el sistema. Un punto de calor en la región se
deben representar como un número de coma flotante de precisión simple (float). El programa debe
tener dos parámetros: el primero indica el tamaño por lado de la región cuadrada, y el segundo (P)
el número de pasos de simulación a ejecutar. Los coeficientes Cx y Cy tendrán el valor 0,1.
Los valores iniciales para cada punto de la región se deben calcular como:

    Ux,y= x * (Tlado – x – 1) * y * (Tlado – y – 1)

donde Tlado es el tamaño por lado de la región.

La salida será por pantalla (al ejecutar el programa se redireccionará la salida a un archivo), y
consistirá de la impresión de la matriz correspondiente al último paso de simulación. La impresión
será por filas con un salto de línea al final de cada fila. La impresión de una fila consiste de la
impresión de cada valor float, que representa la temperatura en cada punto, con el formato "%8.3f"
y separados por un espacio.
