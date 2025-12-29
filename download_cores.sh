#!/bin/bash

# libretro_raylib - Core Downloader Script
# Downloads libretro cores for macOS from the buildbot

# Don't exit on error - we handle errors manually
set +e

BUILDBOT_BASE="https://buildbot.libretro.com"

# Detect architecture
ARCH=$(uname -m)
if [ "$ARCH" = "arm64" ]; then
    ARCH_DIR="arm64"
    echo "Detected Apple Silicon (arm64)"
else
    ARCH_DIR="x86_64"
    echo "Detected Intel Mac (x86_64)"
fi

BUILDBOT_LATEST="${BUILDBOT_BASE}/nightly/apple/osx/${ARCH_DIR}/latest"
CORES_DIR="./cores"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Create cores directory if it doesn't exist
mkdir -p "$CORES_DIR"

echo -e "${BLUE}=== Libretro Core Downloader for macOS ===${NC}\n"

# Function to fetch available cores
fetch_cores_list() {
    echo -e "${YELLOW}Fetching list of available cores...${NC}"
    
    # Fetch cores directly from /latest/ directory (they're in ZIP files)
    # Extract core names from href attributes - href contains full path, extract just filename
    CORE_LIST=$(curl -s "${BUILDBOT_LATEST}/" 2>/dev/null | grep -oE 'href="[^"]*_libretro\.dylib\.zip' | sed 's/href="//' | sed 's|.*/||' | sed 's/_libretro\.dylib\.zip//' | sort -u)
    
    if [ -z "$CORE_LIST" ]; then
        echo -e "${RED}Error: Could not fetch core list from buildbot${NC}"
        echo -e "${YELLOW}You may need to download cores manually from:${NC}"
        echo -e "${BLUE}${BUILDBOT_LATEST}/${NC}"
        return 1
    fi
    
    echo -e "${GREEN}Found cores in latest build${NC}\n"
    echo "$CORE_LIST"
}

# Function to display cores menu
show_cores_menu() {
    local cores=("$@")
    local count=1
    
    echo -e "${BLUE}Available cores:${NC}\n"
    
    for core in "${cores[@]}"; do
        # Check if already downloaded
        if [ -f "${CORES_DIR}/${core}_libretro.dylib" ]; then
            echo -e "  ${GREEN}[$count]${NC} $core ${GREEN}(downloaded)${NC}"
        else
            echo -e "  ${BLUE}[$count]${NC} $core"
        fi
        ((count++))
    done
    
    echo -e "\n  ${YELLOW}[a]${NC} Download all cores"
    echo -e "  ${YELLOW}[q]${NC} Quit"
    echo ""
}

# Function to download a core
download_core() {
    local core_name=$1
    
    # Strip any path components from core_name (in case it somehow includes them)
    core_name=$(basename "$core_name")
    core_name="${core_name%_libretro.dylib}"
    core_name="${core_name%_libretro.dylib.zip}"
    
    local zip_file="${core_name}_libretro.dylib.zip"
    local core_file="${core_name}_libretro.dylib"
    local zip_path="${CORES_DIR}/${zip_file}"
    local output_path="${CORES_DIR}/${core_file}"
    local download_url="${BUILDBOT_LATEST}/${zip_file}"
    
    echo -e "${YELLOW}Downloading ${core_name}...${NC}"
    
    # Download the ZIP file
    if curl -f -L -o "$zip_path" "$download_url" 2>/dev/null; then
        # Extract the dylib from the zip
        if unzip -q -o "$zip_path" -d "$CORES_DIR" 2>/dev/null; then
            # Check if extraction was successful
            if [ -f "$output_path" ]; then
                # Verify it's actually a dylib file
                if file "$output_path" 2>/dev/null | grep -q "Mach-O"; then
                    echo -e "${GREEN}✓ Successfully downloaded ${core_name}${NC}"
                    chmod +x "$output_path"
                    # Clean up zip file
                    rm -f "$zip_path"
                    return 0
                else
                    echo -e "${RED}✗ Extracted file is not a valid dylib${NC}"
                    rm -f "$output_path" "$zip_path"
                    return 1
                fi
            else
                # Try to find the dylib in the extracted contents
                extracted_file=$(find "$CORES_DIR" -name "${core_file}" -type f | head -1)
                if [ -n "$extracted_file" ] && [ "$extracted_file" != "$output_path" ]; then
                    mv "$extracted_file" "$output_path"
                    if file "$output_path" 2>/dev/null | grep -q "Mach-O"; then
                        echo -e "${GREEN}✓ Successfully downloaded ${core_name}${NC}"
                        chmod +x "$output_path"
                        rm -f "$zip_path"
                        return 0
                    fi
                fi
                echo -e "${RED}✗ Could not find extracted dylib file${NC}"
                rm -f "$zip_path"
                return 1
            fi
        else
            echo -e "${RED}✗ Failed to extract ZIP file${NC}"
            rm -f "$zip_path"
            return 1
        fi
    else
        echo -e "${RED}✗ Failed to download ${core_name}${NC}"
        echo -e "${YELLOW}URL: ${download_url}${NC}"
        return 1
    fi
}

# Fallback list of common cores (if buildbot fetch fails)
get_fallback_cores() {
    cat <<EOF
2048
atari800
beetle-psx
beetle-psx-hw
bluemsx
bsnes
bsnes-mercury-accuracy
bsnes-mercury-balanced
bsnes-mercury-performance
cap32
citra
desmume
dolphin
dosbox
dosbox-pure
easyrpg
fbalpha2012
fbalpha2012-cps1
fbalpha2012-cps2
fbalpha2012-cps3
fbalpha2012-neogeo
fbneo
fceumm
flycast
freeintv
fuse
gambatte
genesis-plus-gx
gpsp
gw
handy
hatari
lutro
mame2000
mame2003
mame2003-plus
mame2010
mame2015
mame2016
mednafen-ngp
mednafen-pce-fast
mednafen-psx
mednafen-psx-hw
mednafen-saturn
mednafen-supergrafx
mednafen-vb
mednafen-wswan
melonds
mgba
mupen64plus-next
nestopia
np2kai
o2em
opera
pcsx2
pcsx-rearmed
picodrive
pokemini
ppsspp
prboom
prosystem
quicknes
sameboy
scummvm
snes9x
snes9x2002
snes9x2005
snes9x2010
stella
stella2014
swanstation
tgbdual
theodore
tic80
tyrquake
uzem
vba-next
vbam
vecx
vice-x128
vice-x64
vice-x64sc
vice-xscpu64
vice-xvic
virtualjaguar
yabause
yabause-qt
EOF
}

# Main execution
main() {
    # Fetch cores list
    CORE_LIST_OUTPUT=$(fetch_cores_list)
    
    if [ $? -ne 0 ] || [ -z "$CORE_LIST_OUTPUT" ]; then
        echo -e "${YELLOW}Buildbot fetch failed, using fallback core list...${NC}\n"
        mapfile -t cores < <(get_fallback_cores)
    else
        mapfile -t cores < <(echo "$CORE_LIST_OUTPUT")
    fi
    
    if [ ${#cores[@]} -eq 0 ]; then
        echo -e "${RED}No cores found. Please check your internet connection.${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}Found ${#cores[@]} cores${NC}"
    if [ -z "$CORE_LIST_OUTPUT" ]; then
        echo -e "${YELLOW}(Using fallback list - buildbot may be unavailable)${NC}"
    fi
    echo ""
    
    # Show menu loop
    while true; do
        show_cores_menu "${cores[@]}"
        
        read -p "Select core(s) to download (number, comma-separated numbers, 'a' for all, 'q' to quit): " selection
        
        case "$selection" in
            [qQ])
                echo -e "${BLUE}Goodbye!${NC}"
                exit 0
                ;;
            [aA])
                echo -e "${YELLOW}Downloading all cores...${NC}\n"
                downloaded=0
                failed=0
                for core in "${cores[@]}"; do
                    if download_core "$core"; then
                        ((downloaded++))
                    else
                        ((failed++))
                    fi
                    echo ""
                done
                echo -e "${GREEN}Downloaded: ${downloaded}${NC}"
                if [ $failed -gt 0 ]; then
                    echo -e "${RED}Failed: ${failed}${NC}"
                fi
                ;;
            *)
                # Parse comma-separated numbers
                IFS=',' read -ra selections <<< "$selection"
                for sel in "${selections[@]}"; do
                    sel=$(echo "$sel" | xargs) # trim whitespace
                    if [[ "$sel" =~ ^[0-9]+$ ]] && [ "$sel" -ge 1 ] && [ "$sel" -le ${#cores[@]} ]; then
                        idx=$((sel - 1))
                        core="${cores[$idx]}"
                        download_core "$core"
                        echo ""
                    else
                        echo -e "${RED}Invalid selection: $sel${NC}"
                    fi
                done
                ;;
        esac
        
        echo ""
        read -p "Press Enter to continue..."
        echo ""
    done
}

# Run main function
main

