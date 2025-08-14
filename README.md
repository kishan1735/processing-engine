## Steps to Setup Project on Ubuntu/Debian Based Systems

1. Make sure g++ (17+), make is installed
2. Run `sudo apt-get update`
3. Run `sudo apt-get install libzmq3-dev libbsd-dev` (This is how you install zeromq in ubuntu based distros)
4. Run `chmod +x start_driver.sh`
5. Run `chmod +x start_engine.sh`
6. Run make
7. Now you should be able to run the test runner

## Optimizations done

1. Used buffer loading of files instead of reading line by line via a stringstream.
2. Fast stoi
3. Optimization flags like `-O3`
4. Used effecient datastructures with O(1) access times (unordered_map instead of map etc...)
5. Additional optional optimization flags like `-fprofile-generate` and `-fprofile-use`(Instructions on how to use it is given below)
6. **Additional Notes**: The first time you run it usually takes more time than subsequent iterations. A difference can be observed especially when you use fprofile optimzation

## Changes that can be made for further optimization

1. If you notice the makefile u can see that we have use `-fprofile-generate` flag
2. Run `make` and Run once with the test_runner
3. After that run `make clean`
4. Comment out the `-fprofile-generate` flag and uncomment the `-fprofile-use` flag
5. Run make and run the test_runner. You should be able to see to improved performance

#### Additional Notes

1. If something doesn't work just manually delete the build file (can be due to funnies in the .gcda files)
2. Please edit the file path if its not sample_dataset
