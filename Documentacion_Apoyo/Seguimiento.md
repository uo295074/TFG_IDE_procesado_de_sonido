Primeros dias -> Montar el entorno, maquina virtual y Antigravity

Dia 2 -> Conseguir que compile el plugin, y muestre un simple "Hola Mundo"


Dia 3 (30/1/2026) -> En esta sesión se ha desarrollado la capacidad central del IDE para generar código fuente dinámico. Se ha pasado de la simple creación de directorios a la generación de archivos C++ funcionales mediante un sistema de Plantillas Estáticas (Templates.h). Específicamente, se ha implementado la lógica de lectura y escritura en la clase PluginGenerator, que ahora carga una estructura base de plugin JUCE, localiza un marcador predefinido (TAG) y lo sustituye por el algoritmo DSP que el usuario escribe en tiempo real en la nueva interfaz gráfica (MainComponent). Con esto, se ha cumplido el requisito funcional crítico de permitir la definición personalizada del algoritmo de procesado por parte del usuario.