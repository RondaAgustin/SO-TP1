#Tiene que estar comentada para correr el pvs sin problemas
# run_container:
# 	docker run -v "${PWD}:/root" --privileged -ti agodio/itba-so-multi-platform:3.0  

compile:
	gcc -Wall main.c utils.c -o main && gcc view.c utils.c -o view && gcc slave.c -o slave

clean:
	rm main view slave resultados.txt

run:
	./main files/*

run_with_view:
	./main files/* | ./view

pvs_analyzer:
	pvs-studio-analyzer trace -- make
	pvs-studio-analyzer analyze
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log

valgrind_with_main:
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./main *

valgrind_with_view:
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./view md5_shm 

valgrind_with_pipe:
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./main * | ./view