#!/bin/bash
#script d'installation et de compilation du projet de dequantification
#installe automatiquement TOUTES les dependances :
# - sur Linux : gcc, make, SDL2 via apt/dnf/pacman
# - sur Mac : Homebrew (si absent), puis gcc, make, SDL2 via brew

set -e

echo "=== Verification et installation des dependances ==="

OS="$(uname -s)"

if [ "$OS" = "Darwin" ]; then
    #installation automatique de Homebrew si absent
    if ! command -v brew >/dev/null 2>&1; then
        echo "Homebrew non trouve, installation automatique..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        #ajout de Homebrew au PATH pour la session courante
        if [ -d /opt/homebrew/bin ]; then
            eval "$(/opt/homebrew/bin/brew shellenv)"
        elif [ -d /usr/local/bin ] && [ -f /usr/local/bin/brew ]; then
            eval "$(/usr/local/bin/brew shellenv)"
        fi
    else
        echo "Homebrew deja installe"
    fi
    PKG_MANAGER="brew"
    INSTALL_CMD="brew install"
    UPDATE_CMD="brew update"
    SDL_PKG="sdl2"
elif command -v apt-get >/dev/null 2>&1; then
    PKG_MANAGER="apt-get"
    INSTALL_CMD="sudo apt-get install -y"
    UPDATE_CMD="sudo apt-get update"
    SDL_PKG="libsdl2-dev"
elif command -v dnf >/dev/null 2>&1; then
    PKG_MANAGER="dnf"
    INSTALL_CMD="sudo dnf install -y"
    UPDATE_CMD="sudo dnf check-update || true"
    SDL_PKG="SDL2-devel"
elif command -v pacman >/dev/null 2>&1; then
    PKG_MANAGER="pacman"
    INSTALL_CMD="sudo pacman -S --noconfirm"
    UPDATE_CMD="sudo pacman -Sy"
    SDL_PKG="sdl2"
else
    echo "Aucun gestionnaire de paquets reconnu."
    exit 1
fi

echo "Systeme detecte : $OS / $PKG_MANAGER"

#verification de gcc
if ! command -v gcc >/dev/null 2>&1; then
    echo "gcc non trouve, installation en cours..."
    if [ "$OS" = "Darwin" ]; then
        #sur Mac, gcc vient des Command Line Tools
        xcode-select --install 2>/dev/null || true
        #attendre que l'installation se termine si une fenetre s'ouvre
        until command -v gcc >/dev/null 2>&1; do
            echo "En attente de l'installation des Command Line Tools..."
            sleep 5
        done
    else
        $UPDATE_CMD
        $INSTALL_CMD gcc
    fi
else
    echo "gcc deja installe ($(gcc --version | head -n1))"
fi

#verification de make
if ! command -v make >/dev/null 2>&1; then
    echo "make non trouve, installation en cours..."
    if [ "$OS" = "Darwin" ]; then
        xcode-select --install 2>/dev/null || true
    else
        $INSTALL_CMD make
    fi
else
    echo "make deja installe"
fi

#verification de SDL2
SDL2_INSTALLED=0
if command -v sdl2-config >/dev/null 2>&1; then
    SDL2_INSTALLED=1
fi
for path in /usr/include/SDL2/SDL.h /usr/local/include/SDL2/SDL.h /opt/homebrew/include/SDL2/SDL.h; do
    if [ -f "$path" ]; then
        SDL2_INSTALLED=1
        break
    fi
done

if [ "$SDL2_INSTALLED" -eq 0 ]; then
    echo "SDL2 non trouvee, installation en cours..."
    $INSTALL_CMD $SDL_PKG
else
    echo "SDL2 deja installee"
fi

echo ""
echo "=== Compilation du projet ==="
make clean
make

echo ""
echo "=== Compilation terminee ==="
echo "Lance le programme avec : ./prog <fichier.dat>"
echo "Lance les tests avec    : ./prog_test"
