# install_dependencies.ps1
# Run as Administrator.
# Install Chocolatey (if not already installed)
# If you want to remove it: Remove-Item C:\ProgramData\chocolatey -Recurse -Force

Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
Set-ExecutionPolicy Bypass -Scope Process -Force
iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

# Install Python 3.13
choco install python -y

# Install CMake
choco install cmake -y

# Install MinGW (includes g++)
choco install mingw -y
