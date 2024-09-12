run_container:
	docker run -v "${PWD}:/root" --privileged -ti agodio/itba-so-multi-platform:3.0  

compile:
	gcc -Wall main.c utils.c -o main && gcc view.c utils.c -o view && gcc slave.c -o slave

clean:
	rm main view slave resultados.txt

run:
	./main files/*

run_with_view:
	./main files/* | ./view