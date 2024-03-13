# STM32H745 Inter Processor/Core Communication Measurement Repository

## Overview

The goal of the project to measure the overhead of the IPC communication on a multicore microcontroller running FreeRTOS. The project uses FreeRTOS on both microcontroller cores, and utilizes the Message Buffer for IPC.

The application on the microcontroller processes the control commands recieved through a virtual serial port. The serial commands can configure the parameters of the measurement and then start the measurement. During the measurement, the result are sent back through the serial port.

The parameters are configured and the results are saved by a python script on the side of the controlling PC. These results can also be evaluated and visualized here.

## Usage

todo: requirements, how to start cubeide

The Python scripts can be executed after installing the packages in `requirements.txt`.

## Project structure

* Measurements can be recorded using the `measurements.py` file after downloading the microcontroller code.
* The results can be observed by configuring and running the `visu.py`, `histogram.py` and `visu3d.py` files.
* Fitting of the linear model was performed using `model_regression.py`.
* The final figures were rendered by running `final_visu.py`.

## Results

The state of the code at the time of the TDK submission is tagged.
