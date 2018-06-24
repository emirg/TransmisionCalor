#!/bin/bash
echo 'Comprimiendo archivos...'
tar -vczf resultados.tar.gz datos.txt *{pcf,prv,row}

echo 'Eliminado trazas...'
rm *.pcf *.prv *.row

echo 'Moviendo fichero comprimido...'
mv resultados.tar.gz ../

echo 'Finalizado!'
