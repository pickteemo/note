#cpptools linux系统下的依赖包有：
#1)C/C++ language components (Linux / x86_64)
#2)ClangFormat (Linux / x86_64)
#3)Mono Framework Assemblies
#4)Mono Runtime (Linux / x86_64)

sudo apt-get update
sudo dpkg -i ./code_1.52.1-1608136922_amd64.deb
sudo apt-get -y install --fix-broken
sudo dpkg -i ./code_1.52.1-1608136922_amd64.deb
code --install-extension ms-vscode.cpptools
