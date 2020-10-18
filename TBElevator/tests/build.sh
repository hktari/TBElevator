rm bin/*
g++ -c *.cpp
mv *.o bin
g++ -c ../TBElevator.cpp -o bin/TBElevator.o
g++ -o bin/test.exe bin/*.o
echo "Finished building"
echo "Press to run"
read  -n 1
bin/test.exe


echo "Finished running tests"
read  -n 1