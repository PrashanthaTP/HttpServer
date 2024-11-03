#!/bin/bash


PORT=8080

curl -H 'Content-Type: text/json' \
    -d '{title: "foo", body: "bar" }' \
    -X POST \
    localhost:8080/
    

