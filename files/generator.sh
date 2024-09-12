#!/bin/bash

# Script para crear 100 archivos con contenido aleatorio
for i in {1..100}; do
    # Generar un tamaño aleatorio entre 1KB y 1MB
    size=$((RANDOM % 512 + 1))M
    
    # Crear el archivo con contenido aleatorio
    dd if=/dev/urandom of=archivo${i}.txt bs=$size count=1 status=none
done

echo "generator.sh: 100 archivos generados con contenido aleatorio y tamaños entre 1MB y 512MB"