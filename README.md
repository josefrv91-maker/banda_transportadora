# Controlador de banda transportadora

Base del Proyecto 1, PlatformIO + Arduino + IdeaBoard ESP32.

## Uso

1. Abrir esta carpeta en VS Code con PlatformIO.
2. Compilar: `pio run`.
3. Conectar la placa y cargar: `pio run --target upload`.
4. Abrir monitor: `pio device monitor -b 115200`.
5. Enviar `R` o `r` para reiniciar completamente mediante evento RESET.

El firmware arranca automaticamente despues de inicializar si el paro esta
liberado. El microswitch a GND en IO33 detiene con freno dinamico y enclava FAULT.
No se ha cargado ni probado fisicamente en la placa.

Consultar documentation/arquitectura.md para transiciones, concurrencia,
limitaciones de coast y pruebas pendientes; hardware_pines.json para conexiones.

## Entrega pendiente

Completar cuestionario teorico y verificacion de hardware/coast; crear repositorio
GitHub accesible al profesor y subir commits incrementales. En Classroom solo
entregar el PDF con la URL, llamado apellido1_apellido2_nombre_proyecto1.pdf.
No se genera ese PDF hasta tener la URL real del repositorio.
