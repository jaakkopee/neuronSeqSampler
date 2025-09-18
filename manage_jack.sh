#!/bin/bash
# Script to manage JACK for recording

case "$1" in
    "stop")
        echo "Stopping JACK..."
        killall jackd
        killall qjackctl
        echo "JACK stopped. You can now record audio."
        ;;
    "start")
        echo "Starting JACK..."
        qjackctl &
        echo "JACK started."
        ;;
    *)
        echo "Usage: $0 {stop|start}"
        echo "  stop  - Stop JACK to enable recording"
        echo "  start - Start JACK for audio production"
        exit 1
        ;;
esac
