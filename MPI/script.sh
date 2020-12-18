mpicc programa.c -o programa
if [ $? -eq 0 ];then
	echo "programa compilado!"
else
	echo "nao foi possivel compilar o programa"
fi
