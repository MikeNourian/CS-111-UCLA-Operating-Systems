# NAME: Charlotte McGinn
# EMAIL: charlotte.mcginn@engineering.ucla.edu
# ID: 104-601-417 
# NAME: Milad Nourian
# EMAIL: miladnourian@ucla.edu

# Makefile for lab3b

# Variables
src = 3b.py
dst = lab3b

# Create a symlink to lab3b
build: 3b.py
	rm -rf lab3b
	ln -s $(src) $(dst)
	chmod +x $(dst)
	echo "symlink created"

# Deletes all makefile-created files
clean:
	rm -f lab3b *.tar.gz *~

# Build the distribution tarball
dist: 
	tar -cvzf lab3b-104601417.tar.gz 3b.py Makefile README