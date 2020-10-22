shopt -s extglob
cd bin
rm -v !("main.o")
cd ..
echo "Starting build"
#export CPATH="E:\Program Files (x86)\Arduino\hardware\arduino\avr\cores\arduino;E:\Program Files (x86)\Arduino\hardware\tools\avr\avr\include;E:\Program Files (x86)\Arduino\hardware\arduino\avr\variants\standard"
g++ -c *.cpp
mv *.o bin
g++ -c ../Elevator.cpp -o bin/Elevator.o -D TEST_ENV
g++ -o bin/test.exe bin/*.o
echo "Finished building"

bin/test.exe

echo "Finished running tests"
read  -n 1