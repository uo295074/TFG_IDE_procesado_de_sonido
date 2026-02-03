#!/bin/bash

# 1. Definir rutas
ORIGEN="." # Donde estamos (Carpeta compartida)
DESTINO=~/TFG_IDE_Build/LinuxPluginMaker

# 2. Limpiar destino anterior para asegurar cambios
rm -rf $DESTINO

# 3. Copiar proyecto a la zona segura de Linux (Local)
mkdir -p $DESTINO
cp -r $ORIGEN/* $DESTINO/

# 4. Entrar y Compilar
cd $DESTINO/Builds/LinuxMakefile
make -j4 CPPFLAGS="-I/media/sf_TFG_COMPARTIDO/JUCE/modules -I../../JuceLibraryCode"

# 5. Ejecutar si todo fue bien
if [ $? -eq 0 ]; then
    echo "--- COMPILACIÓN EXITOSA ---"
    echo "Lanzando aplicación..."
    ./build/LinuxPluginMaker
else
    echo "--- ERROR DE COMPILACIÓN ---"
fi