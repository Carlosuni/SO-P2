#!/bin/sh

clear
echo "CREANDO ZIP PARA LA ENTREGA"
cd ssoo_p2
zip -r ssoo_p2_100074907.zip * 

echo "PASANDO CORRECTOR"
./corrector_ssoo_p2-v3.sh ssoo_p2_100074907.zip

echo ""

