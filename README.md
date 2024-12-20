# Http Server in C++

> Network Programming


## Build

```bash
./build.sh
```

## Run

To run Server
```bash
./build/HttpServer
```

To connect to server
```
curl http://localhost:8080
```

## Test

Create python virtual environment and activate

> Python 3.12.8 was used at the time of this repo development

```bash
python -m venv .
source bin/activate
```

Install required modules

```bash
pip install -r dev_requirements.txt
```

Run tests 

```bash
cd tests;./test.sh
```

