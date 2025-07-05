#!/bin/bash

REPO_DIR="Asciiroids"

brew install openal-soft

cd ..

./"${REPO_DIR}"/fetch-dependencies.py "${REPO_DIR}"/dependencies

cd "${REPO_DIR}/${REPO_DIR}"

./build.sh

while true; do
    # Ask the user
    read -p "Do you want to run the program? (yes/no): " response

    # Process the response
    case "$response" in
        yes|y|Y|YES|Yes)
            echo "Running the program..."
            ./run.sh
            break
            ;;
        no|n|N|NO|No)
            echo "Alright. Have a nice day!"
            break
            ;;
        *)
            echo "Invalid response. Please answer yes or no."
            ;;
    esac
done

