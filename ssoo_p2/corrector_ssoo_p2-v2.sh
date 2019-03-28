#!/bin/bash
#set -x


testeo(){
   #$1 nombre del fichero test
   sh     < $1 > salida_bash 2>/dev/null
   ./msh  < $1 > salida_msh  2>/dev/null
   if diff -i -w -q --ignore-all-space salida_bash salida_msh > /dev/null; then
	 echo " OK"
	 echo -n "1 " >> excel
   else
	 echo " Error"
	 echo -n "0 " >> excel
	 echo " === test === "
         cat  $1
	 echo " === salida ./msh  === "
	 cat  salida_msh
	 echo " === salida esperada  === "		
	 cat  salida_bash
   fi
   rm -f salida* res.txt
   #sleep 1
}

testeoRedir(){
   #$1 nombre del fichero test
   sh     < $2
   ./msh  < $1 2>/dev/null
   if diff -i -w -q --ignore-all-space salida_bash salida_msh > /dev/null; then
	 echo " OK"
	 echo -n "1 " >> excel
   else
	 echo " Error"
	 echo -n "0 " >> excel
	 echo " === test === "
         cat  $1
	 echo " === salida ./msh  === "
	 cat  salida_msh
	 echo " === salida esperada  === "		
	 cat  salida_bash
   fi
   rm -f salida* res.txt
   #sleep 1
}

testeoRedirError(){
   #$1 nombre del fichero test
   sh     < $2 >/dev/null 2> salida_bash
   ./msh  < $1 >/dev/null 2>/dev/null
   if diff -i -w -q --ignore-all-space salida_bash salida_msh > /dev/null; then
	 echo " OK"
	 echo -n "1 " >> excel
   else
	 echo " Error"
	 echo -n "0 " >> excel
	 echo " === test === "
         cat  $1
	 echo " === salida ./msh  === "
	 cat  salida_msh
	 echo " === salida esperada  === "		
	 cat  salida_bash
   fi
   rm -f salida* res.txt
   #sleep 1
}


testeoRedirErrore(){
   #$1 nombre del fichero test
   sh     < $2 >/dev/null 2> salida_bash
   ./msh  < $1 >/dev/null 2> salida_msh 
   tail -n 1 salida_msh > salida_msh2
   if [ -s ${salida_msh2} ]
   then 
         echo " OK"
         echo -n "1 " >> excel
   else
         echo " Error"
         echo -n "0 " >> excel
         echo " === test === "
         cat  $1
         echo " === salida ./msh  === "
         cat  salida_msh2
         echo " === salida esperada  === "
         cat  salida_bash
   fi
   rm -f salida* res.txt salida_bash2
   #sleep 1
}


testeo2(){
   
  ./msh  < $1 > salida_msh  2>/dev/null

   if diff -i -w -q --ignore-all-space $2 salida_msh > /dev/null; then
	 echo " OK"
	 echo -n "1 " >> excel
   else
	 echo " Error"
	 echo -n "0 " >> excel
	 echo " === test === "
         cat  $1
	 echo " === salida ./msh  === "
	 cat  salida_msh
	 echo " === salida esperada  === "		
	 cat  $2
   fi
   #sleep 1

}

testeo2bak(){

  ./msh  < $1 > salida_msh  2>/dev/null
  grep "ERROR" < salida_msh > salida2
   if [ -s salida2 ]; then
         echo " OK"
         echo -n "1 " >> excel
   else
         echo " Error"
         echo -n "0 " >> excel
         echo " === test === "
         cat  $1
         echo " === salida ./msh  === "
         cat  salida_msh
         echo " === salida esperada  === "
         cat  $2
   fi
   #sleep 1

}


testeo2Error(){
   
  ./msh  <$1 2> salida_msh  >/dev/null
  cat salida_msh | head -n 4 > msh2
  mv msh2 salida_msh 
   if diff -i -w -q --ignore-all-space $2 salida_msh; then
	 echo " OK"
	 echo -n "1 " >> excel
   else
	 echo " Error"
	 echo -n "0 " >> excel
	 echo " === test === "
         cat  $1
	 echo " === salida ./msh  === "
	 cat salida_msh
	 echo " === salida esperada  === "		
	 cat  $2
   fi   
   #sleep 1

}

testeo3(){
   
  ./msh  < $1 > salida_msh  2>/dev/null

   if diff -i -w -q --ignore-all-space $2 salida_msh > /dev/null; then
	 if diff -i -w -q $3 $4 > /dev/null; then
	 	echo " OK"
		echo -n "1 " >> excel
	 else
		echo " Error"
		echo -n "0 " >> excel
		echo " === test === "
		cat  $1
		echo "Los ficheros son diferentes"
	 fi
   else
	 echo " Error"
	 echo -n "0 " >> excel
	 echo " === test === "
         cat  $1
	 echo " === salida ./msh  === "
	 cat  salida_msh
	 echo " === salida esperada  === "		
	 cat  $2
   fi

}

testeoTimeError(){
   
  ./msh < $1 > salida_msh 2>/dev/null

  if diff -i -w -q --ignore-all-space $2 salida_msh > /dev/null; then
 	echo " OK"
	echo -n "1 " >> excel
  else
	echo " Error"
	echo -n "0 " >> excel
	echo " === test === "
	cat  $1
	echo "Los ficheros son diferentes"
 fi
}

testeoTime(){
   
  ./msh < $1 > salida_msh 2>/dev/null

  arr=(`cat salida_msh | cut -d " "  --output-delimiter=":" -f 1-`)
  IFS=': ' read -r -a array <<< "$arr"
  value=${array[3]}
  IFS='.' read -r -a timearr <<< "$value"
  duration=${timearr[0]}

  if [ $duration -eq $2 ]
  then
	echo " OK"
	echo -n "1 " >> excel
  else
	echo " Error"
	echo -n "0 " >> excel
	echo " === test === "
	cat  $1
	echo "Los tiempos son diferentes"
  fi
}

testeoPwd(){
   #$1 nombre del fichero test
   echo -n "Current dir: " > salida_bash
   pwd >> salida_bash 2>/dev/null
   ./msh  < $1 > salida_msh  2>/dev/null
   if diff -i -w -q --ignore-all-space salida_bash salida_msh > /dev/null; then
	 echo " OK"
	 echo -n "1 " >> excel
   else
	 echo " Error"
	 echo -n "0 " >> excel
	 echo " === test === "
         cat  $1
	 echo " === salida ./msh  === "
	 cat  salida_msh
	 echo " === salida esperada  === "		
	 cat  salida_bash
   fi
   rm -f salida* res.txt
   #sleep 1
}

testeoPwdError(){
   #$1 nombre del fichero test
   ./msh  < $1 2> salida_msh  >/dev/null
   tail -n 2 salida_msh | head -n1 | tee salida_msh_2 >/dev/null
   if diff -i -w -q --ignore-all-space $2 salida_msh_2 > /dev/null; then
	 echo " OK"
	 echo -n "1 " >> excel
   else
	 echo " Error"
	 echo -n "0 " >> excel
	 echo " === test === "
         cat  $1
	 echo " === salida ./msh  === "
	 cat  salida_msh_2
	 echo " === salida esperada  === "		
	 cat  $2
   fi
   rm -f salida* res.txt salida_msh_2
   #sleep 1
}


clear
if [ $# -ne 1 ]; then
	echo "Uso: $0 <ZIP con los fuentes>"
	exit
fi

echo "$1" | grep -q "ssoo_p2_[0-9]*[_]*[0-9]*[_]*[0-9]*.zip"

if [ ! $? -eq 0 ] ;then
	echo "Nombre de fichero incorrecto"
	exit
fi

echo "Nombre de fichero correcto"
ZIP=$(basename "$1")
ZIP="${ZIP%.*}"

unzip $1 -d $ZIP

cp Makefile $ZIP
cp parser.y $ZIP
cp scanner.l $ZIP
cp y.c $ZIP

cd $ZIP
mkdir tmp
find . -name msh.c | awk '{print "cp " $1 " ."}' | sh
echo -n "Compilando: "

make clean 2> /dev/null > /dev/null
make       2> /dev/null > /dev/null


if [ ! -f msh ]; then
	echo "Error"
	cd ..
	rm -r $ZIP
	exit
else
	echo "OK"
fi


#Preparacion de las pruebas
cat > foo.txt << EOF 
123432
67890
12345
46789
90a
10a
EOF

#test

#simples
echo "wc -l  foo.txt"                 	> test1
echo "wc -l  < foo.txt"               	> test2
#1 pipe
echo "cat foo.txt | grep a"            >test5
#2 pipes
echo "cat foo.txt | grep a | grep 1"           >test8
#N pipes
echo "cat foo.txt | grep 1 | grep 2 | grep 3 | grep 4"  >test11

#Background
echo "sleep 1 &"			> test12

#mytime
echo "mytime sleep 2" > time2

#mypwd
echo "mypwd" > pwd1


#Basicos
#mandato simple
echo -n "Test1: 1 comando -> "
testeo test1
#mandato simple mas red entrada
echo -n "Test2: 1 comando + redirección de entrada -> "
testeo test2

echo -n "Test3: 2 comandos -> "
testeo test5

echo -n "Test4: 3 comandos -> "
testeo test8

echo -n "Test5: N comandos -> "
testeo test11

#mandatos internos
echo -n "Test6: mytime + sleep 2-> "
testeoTime time2 2

echo -n "Test7: mypwd-> "
testeoPwd pwd1


#background
echo -n "Test8: Background -> "
testeo test12 test12.res

echo "Resumen de pruebas: -> "
cat excel

rm -fr foo.txt test* excel tmp/msh.c salida_msh time* pwd1 pwd2*

#salimos del directorio
cd ..
