# Cross-Platform-Program-Performance-Prediction-Using-Neural-Networks

1. ACM-ICPC: This folder contains the program set we used for generating hardware counter features using PAPI toolset. It  needs PAPI library to compile and run. Please follow instruction on [this page](http://icl.cs.utk.edu/papi/software/index.html) to get the PAPI library. Under each year there is a Makefile. And the input sets for each program are in  [this](https://drive.google.com/open?id=19Clnp5uDyAYnxy3qFIi_-qz770rRR58h) google drive link. For each of the program it takes in two command line parameters. The first is input file and the second is output file. Each output file will contain the generated 37 execution features.

2. gem5-run.sh
shell script for simulating binary files using gem5 simulator, and parsing gem5 simulation output to collect the reference ticks
to compile: no compiling
to run: ./gem5-run.sh

3. arm-compile.sh
shell script for compiling C programs into binary file in ARM architecture
to compile: no compiling
to run: ./arm-compile.sh

4. data_processing.ipynb
python code in jupyter notebook format for data processing, merging features from papi and labels from gem5 to generate the final training and testing dataset in csv format (see the notebook for detailed descriptions for each stage)
to compile: no compiling
to run: using jupyter notebook (environment python3.6)

5. DNN.ipynb
python code in jupyter notebook format for Deep Neural Networks, including data processing, training, testing, results visualization and comparison with other ML algorithms (see the notebook for detailed descriptions for each part)
to compile: no compiling
to run: using jupyter notebook (environment python3.6)



