shopt -s extglob
cd bin
rm -v !("main.o")
cd ..
echo "Starting build"
g++ -c *.cpp
mv *.o bin
g++ -c ../TBElevator.cpp -o bin/TBElevator.o
g++ -o bin/test.exe bin/*.o
echo "Finished building"

bin/test.exe

echo "Finished running tests"
read  -n 1