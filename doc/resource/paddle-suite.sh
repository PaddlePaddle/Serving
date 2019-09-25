#!/bin/bash
kubectl apply -f https://raw.githubusercontent.com/volcano-sh/volcano/master/installer/volcano-development.yaml
OUTPUT_NODE=$(kubectl get no | awk '{print $1}' | sed -n '2p')
kubectl label nodes $OUTPUT_NODE nodeType=model --overwrite
kubectl apply -f paddle-suite.yaml
