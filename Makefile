run_container:
	docker run -v "${PWD}:/root" --privileged -ti agodio/itba-so-multi-platform:3.0  

compile:
	gcc main.c utils.c -o main && gcc view.c utils.c -o view && gcc slave.c -o slave

run:
	./main files/archivo1.txt files/archivo2.txt files/archivo3.txt files/archivo4.txt files/archivo5.txt files/archivo6.txt files/archivo7.txt files/archivo8.txt files/archivo9.txt files/archivo10.txt files/archivo11.txt files/archivo12.txt files/archivo13.txt files/archivo14.txt files/archivo15.txt files/archivo16.txt files/archivo17.txt files/archivo18.txt files/archivo19.txt files/archivo20.txt files/archivo21.txt files/archivo22.txt files/archivo23.txt files/archivo24.txt files/archivo25.txt files/archivo26.txt files/archivo27.txt files/archivo28.txt files/archivo29.txt files/archivo30.txt files/archivo31.txt files/archivo32.txt files/archivo33.txt files/archivo34.txt files/archivo35.txt files/archivo36.txt files/archivo37.txt files/archivo38.txt files/archivo39.txt files/archivo40.txt files/archivo41.txt files/archivo42.txt files/archivo43.txt files/archivo44.txt files/archivo45.txt files/archivo46.txt files/archivo47.txt files/archivo48.txt files/archivo49.txt files/archivo50.txt files/archivo51.txt files/archivo52.txt files/archivo53.txt files/archivo54.txt files/archivo55.txt files/archivo56.txt files/archivo57.txt files/archivo58.txt files/archivo59.txt files/archivo60.txt files/archivo61.txt files/archivo62.txt files/archivo63.txt files/archivo64.txt files/archivo65.txt files/archivo66.txt files/archivo67.txt files/archivo68.txt files/archivo69.txt files/archivo70.txt files/archivo71.txt files/archivo72.txt files/archivo73.txt files/archivo74.txt files/archivo75.txt files/archivo76.txt files/archivo77.txt files/archivo78.txt files/archivo79.txt files/archivo80.txt files/archivo81.txt files/archivo82.txt files/archivo83.txt files/archivo84.txt files/archivo85.txt files/archivo86.txt files/archivo87.txt files/archivo88.txt files/archivo89.txt files/archivo90.txt files/archivo91.txt files/archivo92.txt files/archivo93.txt files/archivo94.txt files/archivo95.txt files/archivo96.txt files/archivo97.txt files/archivo98.txt files/archivo99.txt files/archivo100.txt

run_with_view:
	./main files/archivo1.txt files/archivo2.txt files/archivo3.txt files/archivo4.txt files/archivo5.txt files/archivo6.txt files/archivo7.txt files/archivo8.txt files/archivo9.txt files/archivo10.txt files/archivo11.txt files/archivo12.txt files/archivo13.txt files/archivo14.txt files/archivo15.txt files/archivo16.txt files/archivo17.txt files/archivo18.txt files/archivo19.txt files/archivo20.txt files/archivo21.txt files/archivo22.txt files/archivo23.txt files/archivo24.txt files/archivo25.txt files/archivo26.txt files/archivo27.txt files/archivo28.txt files/archivo29.txt files/archivo30.txt files/archivo31.txt files/archivo32.txt files/archivo33.txt files/archivo34.txt files/archivo35.txt files/archivo36.txt files/archivo37.txt files/archivo38.txt files/archivo39.txt files/archivo40.txt files/archivo41.txt files/archivo42.txt files/archivo43.txt files/archivo44.txt files/archivo45.txt files/archivo46.txt files/archivo47.txt files/archivo48.txt files/archivo49.txt files/archivo50.txt files/archivo51.txt files/archivo52.txt files/archivo53.txt files/archivo54.txt files/archivo55.txt files/archivo56.txt files/archivo57.txt files/archivo58.txt files/archivo59.txt files/archivo60.txt files/archivo61.txt files/archivo62.txt files/archivo63.txt files/archivo64.txt files/archivo65.txt files/archivo66.txt files/archivo67.txt files/archivo68.txt files/archivo69.txt files/archivo70.txt files/archivo71.txt files/archivo72.txt files/archivo73.txt files/archivo74.txt files/archivo75.txt files/archivo76.txt files/archivo77.txt files/archivo78.txt files/archivo79.txt files/archivo80.txt files/archivo81.txt files/archivo82.txt files/archivo83.txt files/archivo84.txt files/archivo85.txt files/archivo86.txt files/archivo87.txt files/archivo88.txt files/archivo89.txt files/archivo90.txt files/archivo91.txt files/archivo92.txt files/archivo93.txt files/archivo94.txt files/archivo95.txt files/archivo96.txt files/archivo97.txt files/archivo98.txt files/archivo99.txt files/archivo100.txt | ./view

run_reduced:
	./main files/archivo1.txt files/archivo2.txt files/archivo3.txt files/archivo4.txt files/archivo5.txt files/archivo6.txt files/archivo7.txt files/archivo8.txt files/archivo9.txt files/archivo10.txt files/archivo11.txt files/archivo12.txt files/archivo13.txt files/archivo14.txt files/archivo15.txt files/archivo16.txt files/archivo17.txt files/archivo18.txt files/archivo19.txt files/archivo20.txt files/archivo21.txt files/archivo22.txt files/archivo23.txt files/archivo24.txt files/archivo25.txt

run_reduced_with_view:
	./main files/archivo1.txt files/archivo2.txt files/archivo3.txt files/archivo4.txt files/archivo5.txt files/archivo6.txt files/archivo7.txt files/archivo8.txt files/archivo9.txt files/archivo10.txt files/archivo11.txt files/archivo12.txt files/archivo13.txt files/archivo14.txt files/archivo15.txt files/archivo16.txt files/archivo17.txt files/archivo18.txt files/archivo19.txt files/archivo20.txt files/archivo21.txt files/archivo22.txt files/archivo23.txt files/archivo24.txt files/archivo25.txt | ./view