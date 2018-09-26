# How to add a new documentation project

1. Create new directory to contain all relevant files
2. Edit the links at the bottom of header.html in every documentation directory to include:
    - A link to every index.html
    - The entry corresponding to the current directory should have the `current` class
3. Edit Makefile to call doxygen in new directory
