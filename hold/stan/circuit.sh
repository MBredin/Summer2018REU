#!/bin/bash
echo "circuit Preset Simulation Tests"
echo "   4 Thread Simulations"
echo "      Trial 1"
./circuit_x86.x 4 1 0 0 &> circuit_4.txt
for X in {2..10..1}
do
	echo "      Trial $X"
	./circuit_x86.x 4 1 0 0 &>> circuit_4.txt
done


echo "   8 Thread Simulations"
echo "      Trial 1"
./circuit_x86.x 8 1 0 0 &> circuit_8.txt
for X in {2..10..1}
do
	echo "      Trial $X"
	./circuit_x86.x 8 1 0 0 &>> circuit_8.txt
done


echo "   16 Thread Simulations"
echo "      Trial 1"
./circuit_x86.x 16 1 0 0 &> circuit_16.txt
for X in {2..10..1}
do
	echo "      Trial $X"
	./circuit_x86.x 16 1 0 0 &>> circuit_16.txt
done


echo "   32 Thread Simulations"
echo "      Trial 1"
./circuit_x86.x 32 1 0 0 &> circuit_32.txt
for X in {2..10..1}
do
	echo "      Trial $X"
	./circuit_x86.x 32 1 0 0 &>> circuit_32.txt
done

