gcc -o programa -fopenacc programa.c
if [ $? -eq 0 ];then
	echo "Programa compilado com sucesso!"
else
	echo "Programa nao compilado!"
fi
