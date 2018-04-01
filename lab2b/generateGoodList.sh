#After this point, we just test the synchronization

./lab2_list --thread=1 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --thread=2 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --thread=4 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --thread=8 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --thread=12 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --thread=16 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --thread=24 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --thread=2 --iterations=900 --sync=m >> lab2_list.csv
./lab2_list --thread=4 --iterations=900 --sync=m >> lab2_list.csv
./lab2_list --thread=8 --iterations=900 --sync=m >> lab2_list.csv
./lab2_list --thread=12 --iterations=900 --sync=m >> lab2_list.csv
./lab2_list --thread=16 --iterations=900 --sync=m >> lab2_list.csv
./lab2_list --thread=24 --iterations=900 --sync=m >> lab2_list.csv

#spin
./lab2_list --thread=1 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --thread=2 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --thread=4 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --thread=8 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --thread=12 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --thread=16 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --thread=24 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --thread=2 --iterations=900 --sync=s >> lab2_list.csv
./lab2_list --thread=4 --iterations=900 --sync=s >> lab2_list.csv
./lab2_list --thread=8 --iterations=900 --sync=s >> lab2_list.csv
./lab2_list --thread=12 --iterations=900 --sync=s >> lab2_list.csv
./lab2_list --thread=16 --iterations=900 --sync=s >> lab2_list.csv
./lab2_list --thread=24 --iterations=900 --sync=s >> lab2_list.csv


#Run your program with --yield=id, 4 lists, 1,4,8,12,16 threads, and 1, 2, 4, 8, 16 iterations (and no synchronization) to see how many iterations it takes to reliably fail (and make sure your Makefile expects some of these tests to fail).

./lab2_list --thread=1 --iterations=1 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=1 --iterations=2 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=1 --iterations=4 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=1 --iterations=8 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=1 --iterations=16  --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=1 --iterations=32 --lists=4 --yield=id >> lab2_list.csv 

./lab2_list --thread=2 --iterations=1 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=2 --iterations=2 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=2 --iterations=4 --lists=4  --yield=id >> lab2_list.csv 
./lab2_list --thread=2 --iterations=8 --lists=4  --yield=id >> lab2_list.csv 
./lab2_list --thread=2 --iterations=16 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=2 --iterations=32 --lists=4 --yield=id >> lab2_list.csv 

./lab2_list --thread=4 --iterations=1 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=4 --iterations=2 --lists=4 --yield=id >> lab2_list.csv
./lab2_list --thread=4 --iterations=4 --lists=4 --yield=id >> lab2_list.csv
./lab2_list --thread=4 --iterations=8 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=4 --iterations=16 --lists=4 --yield=id >> lab2_list.csv
./lab2_list --thread=4 --iterations=32 --lists=4 --yield=id >> lab2_list.csv 

./lab2_list --thread=8 --iterations=1 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=8 --iterations=2 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=8 --iterations=4 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=8 --iterations=8 --lists=4 --yield=id >> lab2_list.csv
./lab2_list --thread=8 --iterations=16 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=8 --iterations=32 --lists=4 --yield=id >> lab2_list.csv

./lab2_list --thread=12 --iterations=1 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=12 --iterations=2 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=12 --iterations=4 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=12 --iterations=8 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=12 --iterations=16 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=12 --iterations=32 --lists=4 --yield=id >> lab2_list.csv 

./lab2_list --thread=16 --iterations=1 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=16 --iterations=2 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=16 --iterations=4 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=16 --iterations=8 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=16 --iterations=16 --lists=4 --yield=id >> lab2_list.csv 
./lab2_list --thread=16 --iterations=32 --lists=4 --yield=id >> lab2_list.csv 

#Run your program with --yield=id, 4 lists, 1,4,8,12,16 threads, and 10, 20, 40, 80 iterations, --sync=s and --sync=m to confirm that updates are now properly protected (i.e., all runs succeeded).

./lab2_list --thread=1 --iterations=10 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=1 --iterations=20 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=1 --iterations=40 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=1 --iterations=80 --lists=4 --sync=m --yield=id >> lab2_list.csv 

./lab2_list --thread=2 --iterations=10 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=2 --iterations=20 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=2 --iterations=40 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=2 --iterations=80 --lists=4 --sync=m --yield=id >> lab2_list.csv 

./lab2_list --thread=4 --iterations=10 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=4 --iterations=20 --lists=4 --sync=m --yield=id >> lab2_list.csv
./lab2_list --thread=4 --iterations=40 --lists=4 --sync=m --yield=id >> lab2_list.csv
./lab2_list --thread=4 --iterations=80 --lists=4 --sync=m --yield=id >> lab2_list.csv 

./lab2_list --thread=8 --iterations=10 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=8 --iterations=20 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=8 --iterations=40 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=8 --iterations=80 --lists=4 --sync=m --yield=id >> lab2_list.csv

./lab2_list --thread=12 --iterations=10 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=12 --iterations=20 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=12 --iterations=40 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=12 --iterations=80 --lists=4 --sync=m --yield=id >> lab2_list.csv 

./lab2_list --thread=16 --iterations=10 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=16 --iterations=20 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=16 --iterations=40 --lists=4 --sync=m --yield=id >> lab2_list.csv 
./lab2_list --thread=16 --iterations=80 --lists=4 --sync=m --yield=id >> lab2_list.csv 

#from here
./lab2_list --thread=1 --iterations=10 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=1 --iterations=20 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=1 --iterations=40 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=1 --iterations=80 --lists=4 --sync=s --yield=id >> lab2_list.csv 

./lab2_list --thread=2 --iterations=10 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=2 --iterations=20 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=2 --iterations=40 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=2 --iterations=80 --lists=4 --sync=s --yield=id >> lab2_list.csv 

./lab2_list --thread=4 --iterations=10 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=4 --iterations=20 --lists=4 --sync=s --yield=id >> lab2_list.csv
./lab2_list --thread=4 --iterations=40 --lists=4 --sync=s --yield=id >> lab2_list.csv
./lab2_list --thread=4 --iterations=80 --lists=4 --sync=s --yield=id >> lab2_list.csv 

./lab2_list --thread=8 --iterations=10 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=8 --iterations=20 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=8 --iterations=40 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=8 --iterations=80 --lists=4 --sync=s --yield=id >> lab2_list.csv

./lab2_list --thread=12 --iterations=10 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=12 --iterations=20 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=12 --iterations=40 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=12 --iterations=80 --lists=4 --sync=s --yield=id >> lab2_list.csv 

./lab2_list --thread=16 --iterations=10 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=16 --iterations=20 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=16 --iterations=40 --lists=4 --sync=s --yield=id >> lab2_list.csv 
./lab2_list --thread=16 --iterations=80 --lists=4 --sync=s --yield=id >> lab2_list.csv 


#Rerun both synchronized versions, without yields, for 1000 iterations, 1,2,4\

#spin                                                                         
#./lab2_list --threads=1 --iterations=1000 --sync=s --list=1 >> lab2_list.csv
#./lab2_list --threads=2 --iterations=1000 --sync=s --list=1 >> lab2_list.csv
#./lab2_list --threads=4 --iterations=1000 --sync=s --list=1 >> lab2_list.csv
#./lab2_list --threads=8 --iterations=1000 --sync=s --list=1 >> lab2_list.csv
#./lab2_list --threads=12 --iterations=1000 --sync=s --list=1 >> lab2_list.csv

./lab2_list --threads=1 --iterations=1000 --sync=s --list=4 >> lab2_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=s --list=4 >> lab2_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=s --list=4 >> lab2_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=s --list=4 >> lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s --list=4 >> lab2_list.csv

./lab2_list --threads=1 --iterations=1000 --sync=s --list=8 >> lab2_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=s --list=8 >> lab2_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=s --list=8 >> lab2_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=s --list=8 >> lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s --list=8 >> lab2_list.csv

./lab2_list --threads=1 --iterations=1000 --sync=s --list=16 >> lab2_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=s --list=16 >> lab2_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=s --list=16 >> lab2_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=s --list=16 >> lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s --list=16 >> lab2_list.csv

#mutex                                                                        

#./lab2_list --threads=1 --iterations=1000 --sync=m --list=1 >> lab2_list.csv
#./lab2_list --threads=2 --iterations=1000 --sync=m --list=1 >> lab2_list.csv
#./lab2_list --threads=4 --iterations=1000 --sync=m --list=1 >> lab2_list.csv
#./lab2_list --threads=8 --iterations=1000 --sync=m --list=1 >> lab2_list.csv
#./lab2_list --threads=12 --iterations=1000 --sync=m --list=1 >> lab2_list.csv

./lab2_list --threads=1 --iterations=1000 --sync=m --list=4 >> lab2_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=m --list=4 >> lab2_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=m --list=4 >> lab2_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=m --list=4 >> lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=m --list=4 >> lab2_list.csv

./lab2_list --threads=1 --iterations=1000 --sync=m --list=8 >> lab2_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=m --list=8 >> lab2_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=m --list=8 >> lab2_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=m --list=8 >> lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=m --list=8 >> lab2_list.csv

./lab2_list --threads=1 --iterations=1000 --sync=m --list=16 >> lab2_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=m --list=16 >> lab2_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=m --list=16 >> lab2_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=m --list=16 >> lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s --list=16 >> lab2_list.csv

mv lab2_list.csv lab2b_list.csv

