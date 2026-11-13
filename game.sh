#!/bin/bash
# Keep the script entry point and Makefile builds in sync.
cd -- "$(dirname -- "${BASH_SOURCE[0]}")" || exit 1
exec make "$@"
