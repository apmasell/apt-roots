# apt-roots
This finds all the packages that were, in principle, installed by the
administrator of the system. Some packages are marked as manually installed,
even when they are not both in error and to prevent uninstallation of important
programs.

`apt-roots` finds packages that are installed upon which no other installed
package depends, whether marked manual or not.

Installation from PPA is available:

    sudo apt-add-repository ppa:apmasell/ppa
    sudo apt-get update
    sudo apt-get install apt-roots

Or it can be compile from source:

    sudo apt-get install libapt-pkg-dev
    make
    sudo make install
