#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ID_BOTON       200   /* los botones tienen ids 200, 201, 202... */
#define MAX_CARACTERES 15    /* largo maximo de un numero en el visor */

/* ---------- Variables globales (el "estado" de la calculadora) ---------- */
HWND   visor;                /* la pantalla de resultados */
double primero      = 0;     /* primer numero (o resultado acumulado) */
char   operador     = 0;     /* operacion pendiente: + - * /  (0 = ninguna) */
int    nuevo_numero = 1;     /* 1 = el proximo digito empieza un numero nuevo */
int    hay_error    = 0;     /* 1 = el visor muestra un error */

/* Texto de cada boton, en el orden en que se dibujan */
const char *etiquetas[] = {
    "7", "8", "9", "/",
    "4", "5", "6", "*",
    "1", "2", "3", "-",
    "0", ".", "=", "+",
    "C"
};
#define NUM_BOTONES (sizeof etiquetas / sizeof etiquetas[0])

/* ---------- Logica de la calculadora ---------- */

/* Muestra un numero double en el visor */
void mostrar(double valor) {
    char texto[64];
    snprintf(texto, sizeof texto, "%.10g", valor);
    SetWindowTextA(visor, texto);
}

/* Boton C: vuelve todo al estado inicial */
void limpiar(void) {
    primero = 0;
    operador = 0;
    nuevo_numero = 1;
    hay_error = 0;
    SetWindowTextA(visor, "0");
}

/* Calcula "a op b". Si se divide por cero, deja error = 1.
   "error" es un puntero: asi la funcion puede modificar la variable
   del que la llamo. */
double calcular(double a, double b, char op, int *error) {
    *error = 0;
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (b != 0) return a / b;
            *error = 1;            /* division por cero */
            return 0;
        default:
            return b;
    }
}

/* Se presiona un digito (0-9) o el punto decimal */
void escribir_caracter(char c) {
    char texto[64];

    if (nuevo_numero) {            /* empezamos un numero nuevo: borrar visor */
        SetWindowTextA(visor, "");
        nuevo_numero = 0;
    }
    GetWindowTextA(visor, texto, sizeof texto);   /* leer lo que hay escrito */

    if (c == '.' && strchr(texto, '.') != NULL) return;   /* bloquea 12.5.3 */
    if (strlen(texto) >= MAX_CARACTERES) return;          /* evita desborde */
    if (strcmp(texto, "0") == 0 && c != '.') texto[0] = '\0';  /* evita 007 */
    if (texto[0] == '\0' && c == '.') strcpy(texto, "0");      /* .5 -> 0.5 */

    size_t largo = strlen(texto);  /* agregar el caracter al final */
    texto[largo] = c;
    texto[largo + 1] = '\0';
    SetWindowTextA(visor, texto);
}

/* Se presiona + - * / o = */
void presionar_operador(char c) {
    char texto[64];
    GetWindowTextA(visor, texto, sizeof texto);
    double segundo = atof(texto);           /* texto -> double */

    if (operador != 0 && !nuevo_numero) {   /* hay una cuenta pendiente: resolverla */
        int error;
        double resultado = calcular(primero, segundo, operador, &error);

        if (error) {                        /* division por cero */
            SetWindowTextA(visor, "Error: Div/0");
            primero = 0;
            operador = 0;
            nuevo_numero = 1;
            hay_error = 1;
            return;
        }
        primero = resultado;
        mostrar(primero);
    } else if (operador == 0) {
        primero = segundo;                  /* primer numero de la cuenta */
    }

    operador = (c == '=') ? 0 : c;          /* "=" no deja operacion pendiente */
    nuevo_numero = 1;
}

/* Se llama cada vez que se hace clic en un boton */
void al_presionar(char c) {
    if (c == 'C') {
        limpiar();
        return;
    }
    if (hay_error) return;                  /* con error, solo funciona C */

    if ((c >= '0' && c <= '9') || c == '.')
        escribir_caracter(c);
    else
        presionar_operador(c);
}

/* ---------- Ventana (Win32) ---------- */

/* Callback: Windows la llama cada vez que ocurre un evento en la ventana */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {                   /* la ventana se esta creando */
            HFONT fuente = CreateFontA(22, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                                       DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY,
                                       0, "Segoe UI");

            /* Visor: un campo de texto de solo lectura, alineado a la derecha */
            visor = CreateWindowA("EDIT", "0",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_READONLY,
                10, 10, 275, 40, hwnd, NULL, NULL, NULL);
            SendMessage(visor, WM_SETFONT, (WPARAM)fuente, TRUE);

            /* Botones: grilla de 4 columnas; la "C" ocupa todo el ancho abajo */
            for (int i = 0; i < (int)NUM_BOTONES; i++) {
                int x = 10 + (i % 4) * 65;
                int y = 60 + (i / 4) * 55;
                int ancho = 60;
                if (i == 16) ancho = 275;       /* boton C */

                HWND b = CreateWindowA("BUTTON", etiquetas[i],
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    x, y, ancho, 50, hwnd,
                    (HMENU)(INT_PTR)(ID_BOTON + i), NULL, NULL);
                SendMessage(b, WM_SETFONT, (WPARAM)fuente, TRUE);
            }
            return 0;
        }

        case WM_COMMAND: {                  /* se hizo clic en un boton */
            int i = LOWORD(wp) - ID_BOTON;  /* que boton fue */
            if (i >= 0 && i < (int)NUM_BOTONES)
                al_presionar(etiquetas[i][0]);   /* primer caracter del texto */
            return 0;
        }

        case WM_DESTROY:                    /* se cerro la ventana */
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);   /* comportamiento por defecto */
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int show) {
    (void)hPrev; (void)cmd;                 /* parametros que no usamos */

    /* 1. Registrar la "clase" de ventana e indicar cual es su callback */
    WNDCLASSA wc = {0};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "CalculadoraWin";
    RegisterClassA(&wc);

    /* 2. Crear la ventana (tamano fijo) y mostrarla */
    RECT r = {0, 0, 295, 345};
    DWORD estilo = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&r, estilo, FALSE);

    HWND ventana = CreateWindowA("CalculadoraWin", "Calculadora", estilo,
        CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
        NULL, NULL, hInst, NULL);
    ShowWindow(ventana, show);
    UpdateWindow(ventana);

    /* 3. Bucle de mensajes: recibe los eventos y se los pasa a WndProc */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}