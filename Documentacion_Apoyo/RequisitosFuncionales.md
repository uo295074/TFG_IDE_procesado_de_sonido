Bloque 1: Gestión de Proyectos y Configuración 

    RF-01: Creación de Proyectos: El usuario debe poder crear un nuevo proyecto definiendo los metadatos básicos: Nombre del Plugin, Nombre del Fabricante (Vendor) y Versión (ej: 1.0.0).

    RF-02: Configuración de E/S: El sistema permitirá definir la configuración de canales de audio. Para esta versión inicial (MVP), se restringirá a Stereo In / Stereo Out (la más común), simplificando la lógica interna.

    RF-03: Persistencia: El IDE debe ser capaz de Guardar el estado actual del diseño en un archivo propio (formato JSON o XML) y Cargar proyectos anteriores para seguir editándolos.

Bloque 2: Diseño del Plugin 

    RF-04: Selección de Módulos DSP: El usuario dispondrá de una librería de módulos de audio predefinidos para añadir a su cadena de efectos.

        Módulos Mínimos: Ganancia (Volumen), Panner (Balance), Inversor de Fase, Distorsión .

        Módulos Avanzados (Objetivo): Delay simple y Reverb (usando algoritmos de JUCE). Estos son los basicos pero si puedo añadire mas

    RF-05: Ordenamiento de la Cadena: El usuario podrá añadir módulos en serie (uno detrás de otro). El sistema debe gestionar internamente el flujo de audio desde el Módulo 1 al Módulo N.

    RF-06: Asignación de Controles (UI): Al añadir un módulo DSP (ej: Ganancia), el IDE añadirá automáticamente su representación gráfica correspondiente (ej: un Slider o una Ruedecita/Knob) en la vista previa de la interfaz.

    RF-07: Personalización Básica de UI: El usuario podrá modificar etiquetas simples de los controles (ej: cambiar el texto de "Gain" a "Volumen").

Bloque 3: Motor de Generación 

    RF-08: Generación de Estructura de Directorios: El sistema creará en el disco duro una carpeta con la estructura estándar de un proyecto C++ moderno (carpetas Source, Build, etc.).

    RF-09: Inyección de Código C++: El sistema generará los archivos PluginProcessor.cpp y PluginEditor.cpp. Debe ser capaz de iterar sobre los módulos añadidos y escribir el código C++ correcto en las secciones prepareToPlay, processBlock y paint.

    RF-10: Generación de Sistema de Construcción: El IDE generará automáticamente un archivo CMakeLists.txt funcional y configurado para detectar la librería JUCE y compilar el proyecto sin intervención manual del usuario.

Bloque 4: Compilación y Exportación 

    RF-11: Compilación Automática (One-Click Build): El IDE tendrá un botón "Exportar Plugin". Al pulsarlo, invocará internamente a CMake y al compilador (Make/GCC) para generar el archivo binario final (.so para LV2 o .vst3).

    RF-12: Visualización de Logs: El IDE mostrará una consola o ventana de texto con la salida del compilador, para que el usuario sepa si ha habido algún error o si el plugin se ha creado con éxito.