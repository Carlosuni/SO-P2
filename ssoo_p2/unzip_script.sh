#/bin/bash

for file in *.zip
do
	unzip -d "${file%.zip}" $file
done
for directory in dso*/
do 
	cp Tests/* ${directory}
done 
