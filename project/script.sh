if( rm -rf build)
then 
	echo "build deleted"
fi
mkdir build 
cd build 
cmake .. 
make all
cd .. 
./build/bin/CG_UFPel
