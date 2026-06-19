#!/bin/bash

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Get the script name without extension for default directory naming
SCRIPT_NAME=$(basename "$0" .sh)

export DISPLAY=:99
export LD_PRELOAD=$DESKTOP_PREFIX/lib/libdali-override.so
export DALI_DPI_HORIZONTAL=96
export DALI_DPI_VERTICAL=96
export DALI_WINDOW_WIDTH=480
export DALI_WINDOW_HEIGHT=800

# Function to display help
show_help() {
    cat << EOF
Usage: $(basename "$0") [OPTIONS] [app] [width] [height]

Capture screenshots of DALI demo applications.

Options:
  -h, --help           Show this help message and exit
  -l, --list FILE      Process applications from specified list file
  -d, --directory DIR  Directory to store output PNG files
                       (default: /tmp/{script-name}-YYYYMMDDHHMMSS.NNNNNNNNN)
  -n, --no-logs        Disable application logging (output redirected to /dev/null)

Arguments:
  app             Application name to run (searched in PATH)
  width           Optional window width (default: 480)
  height          Optional window height (default: 800)

Examples:
  $(basename "$0") --help                    Show this help message
  $(basename "$0") -l dali-demo.list         Process apps from list file
  $(basename "$0") my-app                    Run single app with default size
  $(basename "$0") my-app 1920 1080          Run single app with custom size
  $(basename "$0") -d /output my-app         Save screenshot to /output/
EOF
}

# Function to run an application and capture screenshot
run_and_capture() {
    local app=$1
    local width=$2
    local height=$3

    [ "$width" = "" ] && width=$DALI_WINDOW_WIDTH
    [ "$height" = "" ] && height=$DALI_WINDOW_HEIGHT

    # Print app info to stdout
    echo "Running: $app (${width}x${height})"

    # Determine output destination for app logs
    local output_dest
    if [ "$NO_LOGS" = true ]; then
        output_dest="/dev/null"
    else
        output_dest="$OUTPUT_DIR/logs/${app}.log"
    fi

    # Start Xvfb with output redirected
    Xvfb :99 -screen 0 ${width}x${height}x24 >> "$output_dest" 2>&1 &
    local xvfb_pid=$!

    # Run the application with output redirected
    $app >> "$output_dest" 2>&1 &
    local app_pid=$!

    # Wait for application to start
    sleep 2

    # Capture screenshot to output directory (redirect output to log)
    import -display :99 -window root "$OUTPUT_DIR/${app}.png" >> "$output_dest" 2>&1

    # Kill the application
    kill $app_pid
    kill $xvfb_pid
}

# Main script
LIST_FILE=""
APP_NAME=""
APP_WIDTH=""
APP_HEIGHT=""
OUTPUT_DIR=""
NO_LOGS=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -l|--list)
            if [ -z "$2" ]; then
                echo "Error: --list requires a file argument"
                show_help
                exit 1
            fi
            LIST_FILE="$2"
            shift 2
            ;;
        -d|--directory)
            if [ -z "$2" ]; then
                echo "Error: --directory requires a directory argument"
                show_help
                exit 1
            fi
            OUTPUT_DIR="$2"
            shift 2
            ;;
        -n|--no-logs)
            NO_LOGS=true
            shift
            ;;
        *)
            if [ -z "$APP_NAME" ]; then
                APP_NAME="$1"
            elif [ -z "$APP_WIDTH" ]; then
                APP_WIDTH="$1"
            elif [ -z "$APP_HEIGHT" ]; then
                APP_HEIGHT="$1"
            else
                echo "Error: Too many arguments"
                show_help
                exit 1
            fi
            shift
            ;;
    esac
done

# Set default output directory if not provided
if [ -z "$OUTPUT_DIR" ]; then
    OUTPUT_DIR="/tmp/${SCRIPT_NAME}-$(date +%Y%m%d%H%M%S.%N)"
fi

# Create output directory (always needed for screenshots)
mkdir -p "$OUTPUT_DIR"

# Create logs subdirectory (unless --no-logs is set)
if [ "$NO_LOGS" = false ]; then
    mkdir -p "$OUTPUT_DIR/logs"
fi

# If --list is specified, process the specified list file
if [ -n "$LIST_FILE" ]; then
    # Use provided list file path
    if [[ "$LIST_FILE" = /* ]]; then
        input_file="$LIST_FILE"
    else
        input_file="$SCRIPT_DIR/$LIST_FILE"
    fi

    # Check if file exists
    if [ ! -f "$input_file" ]; then
        echo "Error: File $input_file not found!"
        exit 1
    fi

    # Read each line from the file
    while IFS=',' read -r app width height; do
        # Remove any whitespace from variables
        app=$(echo "$app" | xargs)
        width=$(echo "$width" | xargs)
        height=$(echo "$height" | xargs)
        
        [ "$app" = "" ] && continue

        # Run in background to process multiple apps in parallel
        run_and_capture "$app" "$width" "$height"

        # Small delay between starting processes
        sleep 1
    done < "$input_file"

    # Wait for all background processes to complete
    wait

    echo "All applications processed and screenshots captured."
    echo "Output directory: $OUTPUT_DIR"
# If an app name is provided, run it directly
elif [ -n "$APP_NAME" ]; then
    # Check if the app exists in PATH
    if ! which "$APP_NAME" > /dev/null 2>&1; then
        echo "Error: Application '$APP_NAME' not found in PATH"
        exit 1
    fi

    run_and_capture "$APP_NAME" "$APP_WIDTH" "$APP_HEIGHT"
    echo "Output directory: $OUTPUT_DIR"
else
    echo "Error: No action specified"
    show_help
    exit 1
fi
