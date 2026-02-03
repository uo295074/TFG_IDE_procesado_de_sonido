Primeros dias -> 

Montar el entorno, maquina virtual y Antigravity

Dia 2 -> 

Conseguir que compile el plugin, y muestre un simple "Hola Mundo"


Dia 3 (30/1/2026) -> 

En esta sesión se ha desarrollado la capacidad central del IDE para generar código fuente dinámico. Se ha pasado de la simple creación de directorios a la generación de archivos C++ funcionales mediante un sistema de Plantillas Estáticas (Templates.h). Específicamente, se ha implementado la lógica de lectura y escritura en la clase PluginGenerator, que ahora carga una estructura base de plugin JUCE, localiza un marcador predefinido (TAG) y lo sustituye por el algoritmo DSP que el usuario escribe en tiempo real en la nueva interfaz gráfica (MainComponent). Con esto, se ha cumplido el requisito funcional crítico de permitir la definición personalizada del algoritmo de procesado por parte del usuario.

Se ha completado el módulo de generación para que el IDE no solo produzca el código lógico (DSP), sino la estructura integral de un proyecto JUCE compilable. Se ha ampliado el sistema de plantillas estáticas (Templates.h) para incluir la generación automática de la clase PluginEditor (necesaria para la interfaz gráfica del plugin VST3) y el archivo de configuración de construcción CMakeLists.txt.

Este último es crítico para el entorno Linux, ya que automatiza la vinculación con las librerías del sistema y el framework JUCE sin intervención manual del usuario. Adicionalmente, se ha robustecido el manejo de cadenas en C++ mediante el uso de delimitadores personalizados en 'Raw String Literals' (R"jv(...)jv"), solucionando conflictos de análisis sintáctico (parsing) provocados por caracteres especiales (paréntesis y comillas) presentes en el código fuente de las plantillas.

Dias 4 y 5 (2-3/2/2026) ->

Durante la fase de compilación del plugin en formato LV2 para Linux, se encontró un error persistente relacionado con la ausencia de las cabeceras gráficas (fatal error: gtk/gtk.h). Este error se debía a una decisión de diseño interna del framework JUCE: en entornos Linux, el módulo que gestiona el formato LV2 tiene una dependencia transitiva estricta con las librerías gráficas del sistema (GTK+), incluso si el desarrollador deshabilita explícitamente la interfaz gráfica (GUI) en la configuración de CMake.

A pesar de intentar desactivar los módulos gráficos (JUCE_ENABLE_GTK=OFF) y eliminar las referencias a juce_gui_extra, el sistema de construcción de JUCE (juce_add_plugin) inyectaba automáticamente estas dependencias al detectar el flag FORMATS LV2. Esto hacía imposible compilar el proyecto en un entorno "headless" o con limitaciones de acceso a librerías gráficas del sistema, como era el caso de nuestra máquina virtual.

La Solución: Arquitectura Desacoplada (DSP Puro + Wrapper Custom)
Para resolver esta limitación estructural, se optó por rediseñar la arquitectura de compilación, pasando de un enfoque monolítico gestionado por JUCE a una arquitectura desacoplada en dos capas:

Motor DSP como Librería Estática: Se aisló la lógica de procesamiento de audio (el código generado por el IDE) en una librería estática (libDspLib.a) que utiliza exclusivamente los módulos matemáticos y de audio de JUCE (juce_audio_basics), eliminando cualquier vínculo con los módulos de interfaz de usuario (juce_gui_basics, juce_gui_extra).

Wrapper LV2 Nativo: Se sustituyó el wrapper automático de JUCE por una implementación propia y ligera en C++ estándar. Este wrapper actúa como un puente que implementa la especificación LV2 (instanciación, conexión de puertos y ejecución) y delega el procesamiento de audio a la librería estática del motor DSP.

Corrección Final: Exportación de Símbolos
Finalmente, para asegurar que el plugin fuera reconocido por los anfitriones (DAWs) como Reaper, se solucionó un problema de "Name Mangling" propio de C++. Se aplicó el modificador extern "C" a la función de entrada lv2_descriptor, garantizando que el enlazador exporte el nombre de la función tal cual lo espera el estándar LV2, permitiendo así la carga dinámica del binario .so sin errores.
