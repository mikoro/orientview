* All the libraries other than Qt should be compiled as shared and installed to the system. Use the windows build instructions as a guide.
* It seems that it is easier to make things work by compiling Qt statically. Download the Qt source package and:

./configure -static -release -nomake examples -nomake tests -skip qtwebkit -skip qtwebkit-examples -opensource -confirm-license
make -j4
sudo make install

* Run the generate_bundle.sh from the root directory to compile and configure the bundle correctly.