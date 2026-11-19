# Calculadora con interfaz gráfica en C

Evaluación Recuperativa - Analista Programador, CFT San Agustín (Cauquenes).

Calculadora de escritorio para **Windows**, escrita en C11 con la **Win32 API**.
Opera con números `double` (decimales) y controla la división por cero.

## Características

- Ventana gráfica nativa con visor de solo lectura y botones en grilla:
  `0-9`, `.`, `+ - * /`, `=` y `C`.
- Operaciones encadenadas de izquierda a derecha (`2 + 3 * 4 =` da `20`).
- Decimales con `double`: `12.75 + 3.40 = 16.15`, `10.5 - 15.25 = -4.75`.
- Bloquea un segundo punto decimal en el mismo número.
- Usa siempre el punto como separador decimal, sin depender de la
  configuración regional (es_CL / en_US), porque no llama a `setlocale()`.
- División por cero: muestra `Error: Div/0`; con `C` se limpia y se sigue operando.

## Archivos

| Archivo | Descripción |
|---|---|
| `calculadora.c` | Código fuente (Win32 API) |
| `Makefile` | Compilación con `make` |

## Instalación de dependencias (Windows)

1. Instalar [MSYS2](https://www.msys2.org/) con las opciones por defecto.
2. Abrir la terminal **MSYS2 MINGW64** (`C:\msys64\mingw64.exe`).
3. Instalar el compilador y make:
   ```bash
   pacman -S mingw-w64-x86_64-toolchain make
   ```

## Compilar y ejecutar

En la terminal **MINGW64**, dentro de la carpeta del proyecto:

```bash
make
./calculadora.exe
```

Comando equivalente sin `make`:

```bash
gcc -std=c11 -Wall -Wextra calculadora.c -o calculadora.exe -mwindows -luser32 -lgdi32
```

No debe mostrar warnings ni errores.

## Casos de prueba

| Teclas | Resultado esperado |
|---|---|
| `7 + 5 =` | `12` |
| `12.75 + 3.40 =` | `16.15` |
| `10.5 - 15.25 =` | `-4.75` |
| `2.5 * 4 =` | `10` |
| `1 / 4 =` | `0.25` |
| `2 + 3 * 4 =` | `20` |
| `5 / 0 =` | `Error: Div/0` |
| `C` tras el error, luego `7 + 1 =` | `8` |

## Estructura del código

- `al_presionar(c)`: recibe cada botón y decide qué hacer.
- `escribir_caracter(c)`: dígitos y punto (bloquea doble punto).
- `presionar_operador(c)`: `+ - * /` y `=`; resuelve la cuenta pendiente.
- `calcular(a, b, op, &error)`: el `switch` con las cuatro operaciones.
- `WndProc`: callback que Windows llama por cada evento (`WM_CREATE`,
  `WM_COMMAND` para los clics, `WM_DESTROY`).